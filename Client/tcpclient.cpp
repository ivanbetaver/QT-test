#include "TcpClient.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QThread>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_dataTimer(new QTimer(this))
    , m_isRunning(false)
    , m_isConnected(false)
    , m_clientId(-1)
{
    // Настройка таймера переподключения
    m_reconnectTimer->setInterval(5000);
    m_reconnectTimer->setSingleShot(true);

    // Настройка таймера отправки данных (случайная задержка)
    m_dataTimer->setSingleShot(true);

    // Подключение сигналов сокета
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &TcpClient::onError);

    // Подключение таймеров
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpClient::onReconnectTimer);
    connect(m_dataTimer, &QTimer::timeout, this, &TcpClient::onDataTimer);
}

TcpClient::~TcpClient()
{
    m_socket->disconnectFromHost();
}

void TcpClient::connectToServer(const QString& host, quint16 port)
{
    m_serverHost = host;
    m_serverPort = port;
    m_socket->connectToHost(host, port);
    qDebug().noquote() << QString("[Клиент] Попытка подключения к %1:%2").arg(host).arg(port);
}

void TcpClient::onConnected()
{
    m_isConnected = true;
    m_reconnectTimer->stop();
    qDebug().noquote() << "[Клиент] Подключен к серверу";
    qDebug().noquote() << "[Клиент] Ожидание подтверждения подключения...";
}

void TcpClient::onDisconnected()
{
    m_isConnected = false;
    m_isRunning = false;
    m_dataTimer->stop();
    qDebug().noquote() << "[Клиент] Отключен от сервера";

    // Попытка переподключения
    if (!m_reconnectTimer->isActive()) {
        qDebug().noquote() << "[Клиент] Попытка переподключения через 5 секунд...";
        m_reconnectTimer->start();
    }
}

void TcpClient::onReadyRead()
{
    m_readBuffer.append(m_socket->readAll());

    // Обработка полных сообщений
    while (m_readBuffer.contains('\n')) {
        int index = m_readBuffer.indexOf('\n');
        QByteArray message = m_readBuffer.left(index);
        m_readBuffer.remove(0, index + 1);

        // Парсинг JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug().noquote() << QString("[Клиент] Ошибка парсинга JSON: %1").arg(parseError.errorString());
            continue;
        }

        if (!doc.isObject()) {
            qDebug().noquote() << "[Клиент] Получен некорректный JSON";
            continue;
        }

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        if (type == "connection_ack") {
            // Подтверждение подключения от сервера
            QString status = obj["status"].toString();
            QString message = obj["message"].toString();
            m_clientId = obj["client_id"].toInt();

            qDebug().noquote() << QString("[Клиент] Получено подтверждение: %1").arg(message);
            qDebug().noquote() << QString("[Клиент] ID клиента: %1").arg(m_clientId);

            if (status == "connected") {
                qDebug().noquote() << "[Клиент] Ожидание команды старта...";
            }
        } else if (type == "command") {
            // Команда от сервера
            QString command = obj["command"].toString();
            qDebug().noquote() << QString("[Клиент] Получена команда: %1").arg(command);
            onCommandReceived(obj);
        }
    }
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    qDebug().noquote() << QString("[Клиент] Ошибка сокета: %1").arg(m_socket->errorString());
}

void TcpClient::onReconnectTimer()
{
    qDebug().noquote() << "[Клиент] Попытка переподключения...";
    m_socket->connectToHost(m_serverHost, m_serverPort);
}

void TcpClient::onDataTimer()
{
    if (m_isRunning && m_isConnected) {
        generateAndSendData();

        // Случайная задержка от 0.01 до 0.1 секунд
        int delay = QRandomGenerator::global()->bounded(10, 100);
        m_dataTimer->start(delay);
    }
}

void TcpClient::onCommandReceived(const QJsonObject& command)
{
    QString cmd = command["command"].toString();

    if (cmd == "start") {
        if (!m_isRunning) {
            m_isRunning = true;
            qDebug().noquote() << "[Клиент] Начало отправки данных";
            onDataTimer(); // Начинаем отправку
        } else {
            qDebug().noquote() << "[Клиент] Уже в режиме отправки данных";
        }
    } else if (cmd == "stop") {
        if (m_isRunning) {
            m_isRunning = false;
            m_dataTimer->stop();
            qDebug().noquote() << "[Клиент] Остановка отправки данных";
        } else {
            qDebug().noquote() << "[Клиент] Уже в режиме ожидания";
        }
    } else {
        qDebug().noquote() << QString("[Клиент] Неизвестная команда: %1").arg(cmd);
    }
}

void TcpClient::generateAndSendData()
{
    // Случайный выбор типа данных (NetworkMetrics, DeviceStatus, Log)
    int type = QRandomGenerator::global()->bounded(3);

    QString jsonData;
    switch (type) {
    case 0:
        jsonData = generateNetworkMetrics();
        break;
    case 1:
        jsonData = generateDeviceStatus();
        break;
    case 2:
        jsonData = generateLog();
        break;
    }

    if (!jsonData.isEmpty()) {
        m_socket->write(jsonData.toUtf8() + "\n");
        m_socket->flush();
        qDebug().noquote() << "[Клиент] Отправлены данные";
    }
}

QString TcpClient::generateNetworkMetrics()
{
    QJsonObject metrics;
    metrics["type"] = "NetworkMetrics";
    metrics["bandwidth"] = QRandomGenerator::global()->bounded(10, 1000) / 10.0; // 1.0 - 100.0 Mbps
    metrics["latency"] = QRandomGenerator::global()->bounded(5, 150); // 5-150 ms
    metrics["packet_loss"] = QRandomGenerator::global()->bounded(0, 100) / 1000.0; // 0-0.1%

    QJsonDocument doc(metrics);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString TcpClient::generateDeviceStatus()
{
    QJsonObject status;
    status["type"] = "DeviceStatus";
    status["uptime"] = QRandomGenerator::global()->bounded(0, 86400); // 0-24 часов в секундах
    status["cpu_usage"] = QRandomGenerator::global()->bounded(0, 100);
    status["memory_usage"] = QRandomGenerator::global()->bounded(0, 100);

    QJsonDocument doc(status);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString TcpClient::generateLog()
{
    QStringList severities = {"INFO", "WARNING", "ERROR", "DEBUG"};
    QStringList messages = {
        "Интерфейс eth0 перезапущен",
        "Обнаружена высокая загрузка сети",
        "Пакет потерян при передаче",
        "Устройство успешно синхронизировано",
        "Критическая ошибка в работе модуля связи",
        "Обновление конфигурации применено",
        "Таймаут соединения с удаленным узлом",
        "Буфер обмена данных очищен",
        "Запущена диагностика сети",
        "Обнаружено подозрительное сетевое подключение"
    };

    // Генерация сообщения разной длины (короткое, среднее, длинное)
    QString message;
    int lengthType = QRandomGenerator::global()->bounded(3);

    if (lengthType == 0) {
        // Короткое сообщение (до 50 символов)
        message = messages[QRandomGenerator::global()->bounded(messages.size())].left(50);
    } else if (lengthType == 1) {
        // Среднее сообщение (50-200 символов)
        message = generateRandomString(50, 200);
    } else {
        // Длинное сообщение (200+ символов)
        message = generateRandomString(200, 500);
    }

    QJsonObject log;
    log["type"] = "Log";
    log["severity"] = severities[QRandomGenerator::global()->bounded(severities.size())];
    log["message"] = message;

    QJsonDocument doc(log);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString TcpClient::generateRandomString(int minLen, int maxLen)
{
    int length = QRandomGenerator::global()->bounded(minLen, maxLen + 1);
    const QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*() ";

    QString result;
    for (int i = 0; i < length; ++i) {
        result += characters[QRandomGenerator::global()->bounded(characters.length())];
    }
    return result;
}

void TcpClient::sendJsonMessage(const QJsonObject& message)
{
    if (m_isConnected) {
        QJsonDocument doc(message);
        m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
        m_socket->flush();
    }
}