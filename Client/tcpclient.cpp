#include "tcpclient.h"
#include "eventsgenerator.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>
#include <QUuid>

TcpClient::TcpClient(const QString& serverHost, const quint16& serverPort, QObject *parent)
    : QObject(parent),
    m_serverHost(serverHost),
    m_serverPort(serverPort),
    m_socket(new QTcpSocket(this)),
    m_reconnectTimer(new QTimer(this)),
    m_dataTimer(new QTimer(this)),
    m_isRunning(false),
    m_isConnected(false)
{
    // идентификатор клента
    id_ = QUuid::createUuid().toString();

    // Настройка таймера переподключения
    m_reconnectTimer->setInterval(5000);

    // Настройка таймера отправки данных (случайная задержка)
    m_dataTimer->setSingleShot(true);

    // Подключение сигналов сокета
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpClient::onError);

    // Подключение таймеров
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpClient::onReconnectTimer);
    connect(m_dataTimer, &QTimer::timeout, this, &TcpClient::onDataTimer);
}

TcpClient::~TcpClient()
{
    m_socket->disconnectFromHost();
}

void TcpClient::start()
{
    m_reconnectTimer->start();
}

void TcpClient::connectToServer()
{
    m_socket->connectToHost(m_serverHost, m_serverPort);
    qDebug().noquote() << QString("[Клиент] Попытка подключения к %1:%2").arg(m_serverHost).arg(m_serverPort);
}

void TcpClient::onConnected()
{
    m_isConnected = true;
    m_reconnectTimer->stop();
    m_dataTimer->start();
    qDebug().noquote() << "[Клиент] Подключен к серверу";

    // Отправка подтверждения подключения
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << id_;
    m_socket->write(data);
    m_socket->flush();
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

            qDebug().noquote() << QString("[Клиент] Получено подтверждение: %1").arg(message);

            if (status == "connected") {
                qDebug().noquote() << "[Клиент] Ожидание команды старта...";
            }
        } else if (type == "command") {
            // Команда от сервера
            QString command = obj["command"].toString();
            qDebug().noquote() << QString("[Клиент] Получена команда: %1").arg(command);
            processCommand(obj);
        }
    }
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    qDebug().noquote() << QString("[Клиент] Ошибка сокета: %1").arg(m_socket->errorString());
}

void TcpClient::onReconnectTimer()
{
    qDebug().noquote() << "[Клиент] Попытка подключения...";
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

void TcpClient::processCommand(const QJsonObject& command)
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
    QString jsonData = EventsGenerator::generateRandom();

    if (!jsonData.isEmpty()) {
        m_socket->write(jsonData.toUtf8() + "\n");
        m_socket->flush();
        qDebug().noquote() << "[Клиент] Отправлены данные";
    }
}

void TcpClient::sendMessage(const QJsonObject& message)
{
    if (m_isConnected) {
        QJsonDocument doc(message);
        m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
        m_socket->flush();
    }
}
