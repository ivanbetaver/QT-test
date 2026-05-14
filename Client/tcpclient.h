#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QRandomGenerator>
#include <QJsonObject>

/**
 * @brief TCP клиент для эмуляции устройства
 */
class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void connectToServer(const QString& host, quint16 port);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();
    void onDataTimer();
    void onCommandReceived(const QJsonObject& command);

private:
    void generateAndSendData();
    QString generateNetworkMetrics();
    QString generateDeviceStatus();
    QString generateLog();
    QString generateRandomString(int minLen, int maxLen);
    void sendJsonMessage(const QJsonObject& message);

    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_dataTimer;
    QByteArray m_readBuffer;
    QString m_serverHost;
    quint16 m_serverPort;
    bool m_isRunning;
    bool m_isConnected;
    int m_clientId;
};

#endif // TCPCLIENT_H