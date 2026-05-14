#include <QCoreApplication>
#include "TcpClient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Telecom Client");
    app.setApplicationVersion("1.0");

    TcpClient client;
    client.connectToServer("localhost", 12345);

    return app.exec();
}