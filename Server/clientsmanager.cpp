#include "clientsmanager.h"
#include "tcpserver.h"
#include "clientstablemodel.h"
#include "clientsmessagestablemodel.h"
#include <QJsonDocument>
#include <QJsonParseError>

ClientsManager::ClientsManager(QObject *parent)
    : QObject{parent},
    tcpServer_(new TcpServer(this)),
    clientsTableModel_(new ClientsTableModel(this)),
    clientsMessagesTableModel_(new ClientsMessagesTableModel(this)),
    clientsRunning_(true),
    settings_("config.ini", QSettings::IniFormat) {

    // Создание и запуск TCP сервера в отдельном потоке
    tcpServer_->start();

    // Подключение сигналов от TCP сервера
    connect(tcpServer_, &TcpServer::clientConnected, this, &ClientsManager::onClientConnected);
    connect(tcpServer_, &TcpServer::clientDisconnected, this, &ClientsManager::onClientDisconnected);
    connect(tcpServer_, &TcpServer::messageReceived, this, &ClientsManager::onMessageReceived);
    connect(tcpServer_, &TcpServer::logMessage, this, [=] (const QString& message, const QString& severity) {
        emit logServerMessage(message, severity);
    });
}

QAbstractTableModel *ClientsManager::getClientsTableModel() {
    return clientsTableModel_;
}

QAbstractTableModel *ClientsManager::getClientsMessagesTableModel() {
    return clientsMessagesTableModel_;
}

void ClientsManager::startMetrics() {
    qDebug().noquote() << "Включение метрик клиентов";
    clientsRunning_ = true;
    emit logServerMessage("Включение метрик клиентов", "SUCCESS");
    tcpServer_->broadcastCommand("start");
    emit metricsIsActiveChanged();
}

void ClientsManager::stopMetrics() {
    qDebug().noquote() << "Отключение метрик клиентов";
    clientsRunning_ = false;
    emit logServerMessage("Отключение метрик клиентов", "WARNING");
    tcpServer_->broadcastCommand("stop");
    emit metricsIsActiveChanged();
}

const bool &ClientsManager::metricsIsActive() {
    return clientsRunning_;
}

void ClientsManager::onClientConnected(QString clientId, const QString& ip, int port) {
    qDebug().noquote() << "Клиент подключен к серверу";
    clientsTableModel_->addClientToTable(clientId, ip, "Подключен");
    emit clientAdded();

    // Отправка команды старт клиенту
    if (clientsRunning_) {
        tcpServer_->sendCommandToClient(clientId, "start");
    }
}

void ClientsManager::onClientDisconnected(const QString& clientId) {
    qDebug().noquote() << QString("Клиент %1: Потеря соединения").arg(clientId);
    clientsTableModel_->updateClientStatus(clientId, "Отключен");
}

void ClientsManager::onMessageReceived(const QString& clientId, const QString& message)
{
    qDebug().noquote() << QString("Новое сообщение клиент %1: [%2]").arg(clientId).arg(message);

    // Парсинг JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit logServerMessage(QString("Ошибка парсинга JSON от клиента %1: %2").arg(clientId).arg(parseError.errorString()), "ERROR");
        return;
    }

    if (!doc.isObject()) {
        emit logServerMessage(QString("Некорректный JSON от клиента %1").arg(clientId), "ERROR");
        return;
    }

    QJsonObject obj = doc.object();

    QString type = obj["type"].toString();
    if (type == "NetworkMetrics" || type == "DeviceStatus" || type == "Log") {
        clientsMessagesTableModel_->addMessageToTable(clientId, type, message, QDateTime::currentDateTime());
        emit  messageAdded();
    } else {
        emit logServerMessage(QString("Неизвестный тип данных от клиента %1: %2").arg(clientId).arg(type), "WARNING");
    }

    // Проверка критических значений
    if (type == "DeviceStatus") {
        int cpu = obj["cpu_usage"].toInt();
        int memory = obj["memory_usage"].toInt();

        if (cpu > settings_.value("settings/critical_cpu").toInt()) {
            emit logServerMessage(QString("Клиент %1: Критическая загрузка CPU: %2%%")
                                .arg(clientId).arg(cpu), "WARNING");
        }
        if (memory > settings_.value("settings/critical_memory").toInt()) {
            emit logServerMessage(QString("Клиент %1: Критическая загрузка памяти: %2%%")
                                .arg(clientId).arg(memory), "WARNING");
        }
    } else if (type == "NetworkMetrics") {
        double latency = obj["latency"].toDouble();
        if (latency > settings_.value("settings/critical_latency").toDouble()) {
            emit logServerMessage(QString("Клиент %1: Высокая задержка сети: %2 ms")
                                .arg(clientId).arg(latency), "WARNING");
        }
    }
}
