#include <QApplication>
#include <QDesktopWidget>

#include "GLWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window window;
    window.resize(512, 512);

    int desktopArea = QApplication::desktop()->width() * QApplication::desktop()->height();
    int widgetArea = window.width() * window.height();

    if (((float)widgetArea / (float)desktopArea) < 0.75f)
        window.show();
    else
        window.showMaximized();

    return app.exec();
}
