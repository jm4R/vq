#pragma once

#include "gui/Window.hpp"

#include <QtWidgets/QApplication>

class Gui
{
public:
    explicit Gui(int argc, char* argv[]);
    int run();

private:
    QApplication application_;
    Window window_;
};
