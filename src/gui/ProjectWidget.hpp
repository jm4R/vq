#pragma once

#include "model/ProjectDescription.hpp"

#include <QWidget>

class QPlainTextEdit;
class OutputGenerator;
class QAction;
class QCheckBox;
class QPoint;
class QTreeWidget;
class QTreeWidgetItem;

class ProjectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectWidget(QWidget* parent = nullptr);


private:
    void onSelected();
    void onLoadInvoked();
    void onGenerateInvoked();
    void onGenerationFinished();
    void onGenerationFailure(const QString& what);
    void onItemContextMenuRequested(const QPoint& point);
    void onClearRequested();

    void setValue(QTreeWidgetItem* item, const QString& val);
    void reload(QTreeWidgetItem &item);
    void reload();

signals:
    void generateRequested(const ProjectDescription& desc);

private:

    QPlainTextEdit* _listEdit{nullptr};
    QTreeWidget* _listWidget{nullptr};
    QAction* _clearAction{nullptr};
    ProjectDescription _project{};
    ConfigurationDescription _currentCfg{};
};
