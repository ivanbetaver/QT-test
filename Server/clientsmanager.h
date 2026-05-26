#ifndef CLIENTSMANAGER_H
#define CLIENTSMANAGER_H

#include <QObject>
#include <QSettings>

class TcpServer;
class ClientsTableModel;
class ClientsMessagesTableModel;
class QAbstractTableModel;

class ClientsManager : public QObject {
    Q_OBJECT

public:
    explicit ClientsManager(QObject *parent = nullptr);
    QAbstractTableModel* getClientsTableModel();
    QAbstractTableModel* getClientsMessagesTableModel();
    void startMetrics();
    void stopMetrics();
    const bool& metricsIsActive();
    void updateCriticalLimits();

signals:
    void clientAdded();
    void messageAdded();
    void logServerMessage(const QString& message, const QString& severity = "INFO");
    void metricsIsActiveChanged();

private slots:
    void onClientConnected(QString clientId, const QString& ip, int port);
    void onClientDisconnected(const QString& clientId);
    void onMessageReceived(const QString& clientId, const QString& message);

private:
    TcpServer* tcpServer_;
    ClientsTableModel* clientsTableModel_;
    ClientsMessagesTableModel* clientsMessagesTableModel_;
    bool clientsRunning_;
    QSettings settings_;
};

inline void ClientsManager::updateCriticalLimits()
{
    settings_.sync();
}

#endif // CLIENTSMANAGER_H
