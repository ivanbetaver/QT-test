#include <QApplication>
#include "mainwindow.h"
#include "clientsmanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QT-test application");
    app.setApplicationVersion("1.0");

    ClientsManager clientsManager;

    MainWindow window(&clientsManager, nullptr);
    window.show();

    return app.exec();
}
