#include "tcpclient.h"
#include <QCoreApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("QT-test Client");
    app.setApplicationVersion("1.0");

    // Получение параметров подключения
    QSettings settings("config.ini", QSettings::IniFormat);
    QString serverHost = settings.value("server/host").toString();
    qint16 serverPort = settings.value("server/port").toInt();

    // Создание и запуск TCP клиента
    TcpClient client(serverHost, serverPort);
    client.start();

    return app.exec();
}
