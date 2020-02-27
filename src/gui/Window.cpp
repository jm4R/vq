#include "gui/Window.hpp"

#include "gui/ProjectWidget.hpp"

#include <QSplitter>

#ifndef APP_VERSION
#define APP_VERSION "Dev"
#endif

Window::Window()
{
    Q_INIT_RESOURCE(res);
    setWindowIcon(QIcon{":/icon.ico"});
    setWindowTitle(QString{"vq %1"}.arg(APP_VERSION));
    resize(1000, 600);
    auto splitter = new QSplitter{this};
    splitter->setContentsMargins(10, 10, 10, 10);
    setCentralWidget(splitter);

    [[maybe_unused]] auto tw = new ProjectWidget{splitter};
}

Window::~Window()
{
    Q_CLEANUP_RESOURCE(res);
}