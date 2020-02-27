#pragma once

#include <QMainWindow>

class QStackedLayout;

class Window : public QMainWindow
{
    Q_OBJECT
public:
    Window();
    virtual ~Window();

private:
    QAction* _synchronizeDbAction{nullptr};
    QStackedLayout* _stackWidget{nullptr};
};