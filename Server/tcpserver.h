#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QThread>
#include <QJsonObject>

class ClientHandler;

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    void start();
    void stop();
    void sendCommandToClient(const QString& clientId, const QString& command);
    void broadcastCommand(const QString& command);
    void updateSettings(const QJsonObject& settings);

signals:
    void clientConnected(const QString& clientId, const QString& ip, int port);
    void clientDisconnected(const QString& clientId);
    void messageReceived(const QString& clientId, const QString& message);
    void logMessage(const QString& message, const QString& type = "INFO");

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    quint16 m_port;
    QMap<QString, QTcpSocket*> m_clients;
    QMap<QTcpSocket*, QString> m_sockets;
    QJsonObject m_settings;
};

#endif // TCPSERVER_H
