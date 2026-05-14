#include <QApplication>
#include "ServerMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Telecom Server");
    app.setApplicationVersion("1.0");

    ServerMainWindow window;
    window.show();

    return app.exec();
}
