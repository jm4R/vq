#include "gui/Gui.hpp"

#include <QStyleFactory>

Gui::Gui(int argc, char* argv[]) : application_{argc, argv}, window_{} {}

int Gui::run()
{
    window_.show();
    auto style = QStyleFactory::create("Fusion");
    if (style)
        application_.setStyle(style);
    return application_.exec();
}
