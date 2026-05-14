#include "ServerMainWindow.h"
#include "TcpServer.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDateTime>
#include <QGroupBox>
#include <QMessageBox>
#include <QJsonDocument>


class Ui::ServerMainWindow {
public:
    QTableWidget* clientsTable;
    QTableWidget* dataTable;
    QTextEdit* logEdit;
    QPushButton* startStopButton;
    QPushButton* settingsButton;
    QPushButton* clearLogButton;
    QLabel* statusLabel;
    QLabel* clientCountLabel;
};

ServerMainWindow::ServerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerMainWindow)
    , m_clientsRunning(true)
{
    setupUI();
    createTables();
    setupConnections();

    // Создание и запуск TCP сервера в отдельном потоке
    m_tcpServer = std::make_unique<TcpServer>(this);
    m_tcpServer->startServer(12345);

    onLogMessage("Сервер запущен на порту 12345", "SUCCESS");
    updateClientCount();
}

ServerMainWindow::~ServerMainWindow()
{
    if (m_tcpServer) {
        m_tcpServer->stopServer();
    }
}

void ServerMainWindow::setupUI()
{
    ui->startStopButton = new QPushButton("Остановить клиентов", this);
    ui->settingsButton = new QPushButton("Настройки", this);
    ui->clearLogButton = new QPushButton("Очистить лог", this);
    ui->statusLabel = new QLabel("Статус: Работает", this);
    ui->clientCountLabel = new QLabel("Клиентов: 0", this);

    // Центральный виджет
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Верхняя панель с кнопками
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addWidget(ui->startStopButton);
    topLayout->addWidget(ui->settingsButton);
    topLayout->addWidget(ui->clearLogButton);
    topLayout->addStretch();
    topLayout->addWidget(ui->statusLabel);
    topLayout->addWidget(ui->clientCountLabel);
    mainLayout->addLayout(topLayout);

    // Группа для таблицы клиентов
    QGroupBox* clientsGroup = new QGroupBox("Подключенные клиенты", this);
    QVBoxLayout* clientsLayout = new QVBoxLayout(clientsGroup);
    ui->clientsTable = new QTableWidget(this);
    clientsLayout->addWidget(ui->clientsTable);
    mainLayout->addWidget(clientsGroup, 1);

    // Группа для таблицы данных
    QGroupBox* dataGroup = new QGroupBox("Полученные данные", this);
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);
    ui->dataTable = new QTableWidget(this);
    dataLayout->addWidget(ui->dataTable);
    mainLayout->addWidget(dataGroup, 2);

    // Группа для лога
    QGroupBox* logGroup = new QGroupBox("Лог событий", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    ui->logEdit = new QTextEdit(this);
    ui->logEdit->setReadOnly(true);
    logLayout->addWidget(ui->logEdit);
    mainLayout->addWidget(logGroup, 1);

    setCentralWidget(centralWidget);
    resize(1200, 800);
    setWindowTitle("Telecom Server - Система управления устройствами");
}

void ServerMainWindow::createTables()
{
    // Таблица клиентов
    ui->clientsTable->setColumnCount(3);
    ui->clientsTable->setHorizontalHeaderLabels({"ID клиента", "IP-адрес", "Статус"});
    ui->clientsTable->horizontalHeader()->setStretchLastSection(true);
    ui->clientsTable->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->clientsTable->setSelectionBehavior(QTableWidget::SelectRows);

    // Таблица данных
    ui->dataTable->setColumnCount(4);
    ui->dataTable->setHorizontalHeaderLabels({"ID клиента", "Тип данных", "Содержимое", "Время получения"});
    ui->dataTable->horizontalHeader()->setStretchLastSection(true);
    ui->dataTable->setEditTriggers(QTableWidget::NoEditTriggers);
}

void ServerMainWindow::setupConnections()
{
    connect(ui->startStopButton, &QPushButton::clicked, this, &ServerMainWindow::onStartStopClients);
    connect(ui->settingsButton, &QPushButton::clicked, this, &ServerMainWindow::openSettings);
    connect(ui->clearLogButton, &QPushButton::clicked, this, &ServerMainWindow::clearLog);

    // Подключение сигналов от TCP сервера
    connect(m_tcpServer.get(), &TcpServer::clientConnected,
            this, &ServerMainWindow::onClientConnected);
    connect(m_tcpServer.get(), &TcpServer::clientDisconnected,
            this, &ServerMainWindow::onClientDisconnected);
    connect(m_tcpServer.get(), &TcpServer::dataReceived,
            this, &ServerMainWindow::onDataReceived);
    connect(m_tcpServer.get(), &TcpServer::logMessage,
            this, &ServerMainWindow::onLogMessage);
}

void ServerMainWindow::onClientConnected(int clientId, const QString& ip, int port)
{
    addClientToTable(clientId, ip, "Подключен");
    onLogMessage(QString("Клиент %1 подключен (IP: %2, Порт: %3)").arg(clientId).arg(ip).arg(port), "SUCCESS");
    updateClientCount();

    // Отправка команды старта клиенту
    if (m_clientsRunning) {
        m_tcpServer->sendCommandToClient(clientId, "start");
    }
}

void ServerMainWindow::onClientDisconnected(int clientId)
{
    removeClientFromTable(clientId);
    onLogMessage(QString("Клиент %1 отключен").arg(clientId), "WARNING");
    updateClientCount();
}

void ServerMainWindow::onDataReceived(int clientId, const QString& dataType,
                                      const QString& content, const QDateTime& time)
{
    QString parsedContent = parseDataTypeContent(dataType, content);
    addDataToTable(clientId, dataType, parsedContent, time);
    onLogMessage(QString("Получены данные от клиента %1: Тип=%2").arg(clientId).arg(dataType), "INFO");
}

void ServerMainWindow::onLogMessage(const QString& message, const QString& type)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formattedMessage;

    if (type == "ERROR") {
        formattedMessage = QString("[%1] [ОШИБКА] %2").arg(timestamp).arg(message);
    } else if (type == "WARNING") {
        formattedMessage = QString("[%1] [ПРЕДУПРЕЖДЕНИЕ] %2").arg(timestamp).arg(message);
    } else if (type == "SUCCESS") {
        formattedMessage = QString("[%1] [УСПЕХ] %2").arg(timestamp).arg(message);
    } else {
        formattedMessage = QString("[%1] [ИНФО] %2").arg(timestamp).arg(message);
    }

    ui->logEdit->append(formattedMessage);
}

void ServerMainWindow::onStartStopClients()
{
    m_clientsRunning = !m_clientsRunning;

    if (m_clientsRunning) {
        ui->startStopButton->setText("Остановить клиентов");
        ui->statusLabel->setText("Статус: Клиенты активны");
        onLogMessage("Запуск всех клиентов", "SUCCESS");
        m_tcpServer->broadcastCommand("start");
    } else {
        ui->startStopButton->setText("Запустить клиентов");
        ui->statusLabel->setText("Статус: Клиенты остановлены");
        onLogMessage("Остановка всех клиентов", "WARNING");
        m_tcpServer->broadcastCommand("stop");
    }
}

void ServerMainWindow::openSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto settings = dialog.getSettings();
        m_tcpServer->updateSettings(settings);
        onLogMessage("Настройки сервера обновлены", "INFO");
    }
}

void ServerMainWindow::clearLog()
{
    ui->logEdit->clear();
    onLogMessage("Лог очищен", "INFO");
}

void ServerMainWindow::updateClientCount()
{
    int count = ui->clientsTable->rowCount();
    ui->clientCountLabel->setText(QString("Клиентов: %1").arg(count));
}

void ServerMainWindow::addClientToTable(int clientId, const QString& ip, const QString& status)
{
    int row = ui->clientsTable->rowCount();
    ui->clientsTable->insertRow(row);

    ui->clientsTable->setItem(row, 0, new QTableWidgetItem(QString::number(clientId)));
    ui->clientsTable->setItem(row, 1, new QTableWidgetItem(ip));
    ui->clientsTable->setItem(row, 2, new QTableWidgetItem(status));

    m_clientsStatus[clientId] = status;
}

void ServerMainWindow::removeClientFromTable(int clientId)
{
    for (int i = 0; i < ui->clientsTable->rowCount(); ++i) {
        if (ui->clientsTable->item(i, 0)->text().toInt() == clientId) {
            ui->clientsTable->removeRow(i);
            break;
        }
    }
    m_clientsStatus.remove(clientId);
}

void ServerMainWindow::addDataToTable(int clientId, const QString& dataType,
                                      const QString& content, const QDateTime& time)
{
    int row = ui->dataTable->rowCount();
    ui->dataTable->insertRow(row);

    ui->dataTable->setItem(row, 0, new QTableWidgetItem(QString::number(clientId)));
    ui->dataTable->setItem(row, 1, new QTableWidgetItem(dataType));
    ui->dataTable->setItem(row, 2, new QTableWidgetItem(content));
    ui->dataTable->setItem(row, 3, new QTableWidgetItem(time.toString("hh:mm:ss.zzz")));

    // Автопрокрутка к последней строке
    ui->dataTable->scrollToBottom();
}

QString ServerMainWindow::parseDataTypeContent(const QString& dataType, const QString& content)
{
    // Парсинг JSON для красивого отображения
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (dataType == "NetworkMetrics") {
            return QString("Пропускная способность: %1 Mbps, Задержка: %2 ms, Потери: %3%")
                .arg(obj["bandwidth"].toDouble())
                .arg(obj["latency"].toDouble())
                .arg(obj["packet_loss"].toDouble());
        } else if (dataType == "DeviceStatus") {
            return QString("Время работы: %1 сек, CPU: %2%, Память: %3%")
                .arg(obj["uptime"].toInt())
                .arg(obj["cpu_usage"].toInt())
                .arg(obj["memory_usage"].toInt());
        } else if (dataType == "Log") {
            return QString("[%1] %2").arg(obj["severity"].toString()).arg(obj["message"].toString());
        }
    }
    return content;
}

void ServerMainWindow::updateClientStatus(int clientId, const QString& status)
{
    for (int i = 0; i < ui->clientsTable->rowCount(); ++i) {
        if (ui->clientsTable->item(i, 0)->text().toInt() == clientId) {
            ui->clientsTable->item(i, 2)->setText(status);
            break;
        }
    }
}
