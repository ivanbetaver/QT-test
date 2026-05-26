#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(const QString& serverHost, const quint16& serverPort, QObject *parent = nullptr);
    ~TcpClient();

    void start();
    void sendMessage(const QJsonObject& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();
    void onDataTimer();

private:
    void connectToServer();
    void generateAndSendData();
    void processCommand(const QJsonObject& command);

    QString m_serverHost;
    quint16 m_serverPort;
    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_dataTimer;
    QByteArray m_readBuffer;
    bool m_isRunning;
    bool m_isConnected;
    QString id_;
};

#endif // TCPCLIENT_H
