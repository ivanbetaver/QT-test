#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>
#include <memory>

class TcpServer;

QT_BEGIN_NAMESPACE
namespace Ui { class ServerMainWindow; }
QT_END_NAMESPACE

/**
 * @brief Главное окно сервера с GUI для управления клиентами
 */
class ServerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerMainWindow(QWidget *parent = nullptr);
    ~ServerMainWindow();

public slots:
    void onClientConnected(int clientId, const QString& ip, int port);
    void onClientDisconnected(int clientId);
    void onDataReceived(int clientId, const QString& dataType,
                        const QString& content, const QDateTime& time);
    void onLogMessage(const QString& message, const QString& type = "INFO");
    void onStartStopClients();

private slots:
    void setupConnections();
    void updateClientStatus(int clientId, const QString& status);
    void openSettings();
    void clearLog();
    void updateClientCount();

private:
    void setupUI();
    void createTables();
    void addClientToTable(int clientId, const QString& ip, int port);
    void addDataToTable(int clientId, const QString& dataType,
                        const QString& content, const QDateTime& time);
    void removeClientFromTable(int clientId);
    QString parseDataTypeContent(const QString& dataType, const QString& content);

    Ui::ServerMainWindow* ui;
    std::unique_ptr<TcpServer> m_tcpServer;
    QMap<int, QString> m_clientsStatus;
    bool m_clientsRunning;
};

#endif // SERVERMAINWINDOW_H