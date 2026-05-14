#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QThread>
#include <QJsonObject>

class ClientHandler;

/**
 * @brief TCP сервер для управления клиентами
 */
class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    void startServer(quint16 port);
    void stopServer();
    void sendCommandToClient(int clientId, const QString& command);
    void broadcastCommand(const QString& command);
    void updateSettings(const QJsonObject& settings);

signals:
    void clientConnected(int clientId, const QString& ip, int port);
    void clientDisconnected(int clientId);
    void dataReceived(int clientId, const QString& dataType,
                      const QString& content, const QDateTime& time);
    void logMessage(const QString& message, const QString& type = "INFO");

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    QMap<int, QTcpSocket*> m_clients;
    QMap<int, QByteArray> m_readBuffers;
    int m_nextClientId;
    QJsonObject m_settings;
};

#endif // TCPSERVER_H