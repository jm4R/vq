#include "gui/ProjectWidget.hpp"

#include "model/OutputGenerator.hpp"
#include "model/VcxprojParser.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeWidget>
#include <QVBoxLayout>

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
            _confCombo = new QComboBox{this};
            optionsWidget->layout()->addWidget(_confCombo);
            connect(_confCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                    [this](int i) { onConfigurationChanged(i); });
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

            _listWidget->setHeaderItem(
                new QTreeWidgetItem{{"Property", "Value(s)"}});
            populateList();
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
    /*auto path = QFileDialog::getOpenFileName(
        this, {}, {}, "Visual Studio project (*.vcxproj)");*/
    QString path{"/home/jaskol/test.vcxproj"};
    if (path.isNull())
        return;

    auto&& file = QFile{path};
    _project = parseVcxproj(file);
    _project.vcxprojPath = path;
    if (_project.configurations.size() > 0)
    {
        populateConfigurations();
    }
    reload();
}

void ProjectWidget::onGenerateInvoked()
{
    auto&& generator = OutputGenerator{this};
    connect(&generator, &OutputGenerator::finished,
            [this] { onGenerationFinished(); });
    connect(&generator, &OutputGenerator::failure,
            [this](auto what) { onGenerationFailure(what); });
    generator.generate(_project, _currentCfg);
}

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
    menu->addAction("Clear", [this] { onClearRequested(); },
                    QKeySequence::Delete);
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

void ProjectWidget::onConfigurationChanged(int index)
{
    constexpr static auto NONE_INDEX = -1;
    assert(_project.configurations.size() > index);
    if (index == NONE_INDEX)
        return; // TODO
    _currentCfg = _project.configurations[index];
    populateList();
}

void ProjectWidget::populateConfigurations()
{
    _confCombo->clear();
    for(const auto& c :_project.configurations)
    {
        _confCombo->addItem(c.name);
    }
}

void ProjectWidget::populateList()
{
    _listWidget->clear();
    auto addItem = [&](const char* name, auto* data) {
        ItemType t =
            std::is_same_v<decltype(data), QString*> ? kPathType : kListType;
        auto i = new QTreeWidgetItem{_listWidget, {name}, t};
        i->setData(kValueColumn, Qt::UserRole, QVariant::fromValue(data));
    };

    addItem("Source project (vcxproj)", &_project.vcxprojPath);
    addItem("Preprocessor defines", &_currentCfg.defines);
    addItem("Include directories", &_currentCfg.includePaths);
    addItem("Header files", &_currentCfg.headerPaths);
    addItem("Source files", &_currentCfg.sourcePaths);
    addItem("Compiler flags (for clang code model)", &_project.flags);

    addItem("Solution path (sln)", &_project.slnPath);
    addItem("MSBUILD path", &_project.msbuildPath);
    addItem("Post build commands", &_project.postBuildCommand);
    addItem("Executable to run/debug", &_project.executablePath);

    reload();
    _listWidget->resizeColumnToContents(0);
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
