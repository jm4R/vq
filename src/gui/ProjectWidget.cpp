#include "gui/ProjectWidget.hpp"

#include "model/OutputGenerator.hpp"
#include "model/VcxprojParser.hpp"

#include <QCheckBox>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QFile>

#include <cassert>

Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(TokenizedString*)

namespace {

constexpr auto kValueColumn = 1;

enum ItemType
{
    kListType = QTreeWidgetItem::UserType + 1,
    kPathType = QTreeWidgetItem::UserType + 2,
};

} // namespace

ProjectWidget::ProjectWidget(QWidget* parent) : QWidget{parent}
{
    setLayout(new QVBoxLayout{});

    {
        auto optionsWidget = new QFrame{this};
        // optionsWidget->setFrameShape(QFrame::StyledPanel);
        optionsWidget->setLayout(new QHBoxLayout{});
        {
            auto loadButton = new QPushButton{"Load vcxproj", this};
            optionsWidget->layout()->addWidget(loadButton);
            connect(loadButton, &QPushButton::clicked,
                    [this](bool) { onLoadInvoked(); });
        }
        {
            auto generateButton = new QPushButton{"Generate", this};
            optionsWidget->layout()->addWidget(generateButton);
            connect(generateButton, &QPushButton::clicked,
                    [this](bool) { onGenerateInvoked(); });
        }
        layout()->addWidget(optionsWidget);
    }
    {
        auto splitter = new QSplitter{Qt::Vertical, this};
        {
            _listWidget = new QTreeWidget{splitter};
            _listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
            _listWidget->setIndentation(0);

            connect(_listWidget, &QTreeWidget::itemSelectionChanged,
                    [this]() { onSelected(); });
            connect(_listWidget, &QTreeWidget::customContextMenuRequested,
                    [this](auto p) { onItemContextMenuRequested(p); });

            auto addItem = [&](const char* name, auto* data) {
                ItemType t = std::is_same_v<decltype(data), QString*>
                                 ? kPathType
                                 : kListType;
                auto i = new QTreeWidgetItem{_listWidget, {name}, t};
                i->setData(kValueColumn, Qt::UserRole,
                           QVariant::fromValue(data));
            };

            _listWidget->setHeaderItem(
                new QTreeWidgetItem{{"Property", "Value(s)"}});
            addItem("Source project (vcxproj)", &_project.vcxprojPath);
            addItem("Preprocessor defines", &_project.defines);
            addItem("Include directories", &_project.includePaths);
            addItem("Header files", &_project.headerPaths);
            addItem("Source files", &_project.sourcePaths);
            addItem("Compiler flags (for clang code model)", &_project.flags);

            addItem("Solution path (sln)", &_project.slnPath);
            addItem("MSBUILD path", &_project.msbuildPath);
            addItem("Post build commands", &_project.postBuildCommand);
            addItem("Executable to run/debug", &_project.executablePath);
            _listWidget->resizeColumnToContents(0);
        }
        {
            _listEdit = new QPlainTextEdit{splitter};
        }

        splitter->setSizes({1, 1});
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);
        layout()->addWidget(splitter);
    }
    {
        auto _clearAction = new QAction{"Clear", _listWidget};
        _listWidget->addAction(_clearAction);
        connect(_clearAction, &QAction::triggered,
                [this] { onClearRequested(); });
        _clearAction->setShortcut(QKeySequence::Delete);
        _clearAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }
}

void ProjectWidget::onSelected()
{
    _listEdit->clear();

    auto items = _listWidget->selectedItems();
    if (items.count() != 1)
        return;

    auto* item = items[0];

    if (item->type() == kListType)
    {
        auto str =
            item->data(kValueColumn, Qt::UserRole).value<TokenizedString*>();
        assert(str);
        for (const auto& val : *str)
        {
            _listEdit->appendPlainText(val);
        }
    }
    else if (item->type() == kPathType)
    {
        auto str = item->data(kValueColumn, Qt::UserRole).value<QString*>();
        assert(str);
        _listEdit->appendPlainText(*str);
    }
}

void ProjectWidget::onLoadInvoked()
{
    auto path = QFileDialog::getOpenFileName(
        this, {}, {}, "Visual Studio project (*.vcxproj)");
    if (path.isNull())
        return;

    auto&& file = QFile{path};
    _project = parseVcxproj(file);
    reload();
}

void ProjectWidget::onGenerateInvoked() {}

void ProjectWidget::onGenerationFinished()
{
    QMessageBox::information(this, "Success",
                             "Qt Creator project successfully created");
}

void ProjectWidget::onGenerationFailure(const QString& what)
{
    QMessageBox::critical(this, "Failure",
                          "Failed to generate project: \n" + what);
}

void ProjectWidget::onItemContextMenuRequested(const QPoint& point)
{
    auto* item = _listWidget->itemAt(point);
    if (!item)
        return;

    auto menu = new QMenu{this};
    menu->setAttribute(Qt::WA_DeleteOnClose, true);
    menu->addAction(
        "Clear", [this] { onClearRequested(); }, QKeySequence::Delete);
    menu->popup(_listWidget->viewport()->mapToGlobal(point));
}

void ProjectWidget::onClearRequested()
{
    auto items = _listWidget->selectedItems();
    for (auto& item : items)
    {
        setValue(item, "");
    }
}

void ProjectWidget::setValue(QTreeWidgetItem* item, const QString& val)
{
    if (!item)
        return;

    item->setText(kValueColumn, val);
    if (item->type() == kListType)
    {
        auto str =
            item->data(kValueColumn, Qt::UserRole).value<TokenizedString*>();
        assert(str);
        *str = TokenizedString{val};
    }
    else if (item->type() == kPathType)
    {
        auto str = item->data(kValueColumn, Qt::UserRole).value<QString*>();
        assert(str);
        *str = val;
    }
}

void ProjectWidget::reload(QTreeWidgetItem& item)
{
    if (item.type() == kListType)
    {
        auto str =
            item.data(kValueColumn, Qt::UserRole).value<TokenizedString*>();
        auto txt = item.text(0);
        assert(str);
        item.setText(kValueColumn, *str);
    }
    else if (item.type() == kPathType)
    {
        auto str = item.data(kValueColumn, Qt::UserRole).value<QString*>();
        assert(str);
        item.setText(kValueColumn, *str);
    }
}

void ProjectWidget::reload()
{
    QTreeWidgetItemIterator it(_listWidget);
    while (*it)
    {
        auto* item = *it;
        reload(*item);
        ++it;
    }
}
