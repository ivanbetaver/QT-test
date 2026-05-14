#include "TcpServer.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QOverload>
#include <QAbstractSocket>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
    , m_nextClientId(1)
{
    // Настройки по умолчанию
    m_settings["critical_cpu"] = 80;
    m_settings["critical_memory"] = 85;
    m_settings["critical_latency"] = 100;
}

TcpServer::~TcpServer()
{
    stopServer();
}

void TcpServer::startServer(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        emit logMessage(QString("Сервер запущен на порту %1").arg(port), "SUCCESS");
    } else {
        emit logMessage(QString("Ошибка запуска сервера: %1").arg(errorString()), "ERROR");
    }
}

void TcpServer::stopServer()
{
    for (QTcpSocket* socket : m_clients.values()) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_clients.clear();
    m_readBuffers.clear();
    close();
    emit logMessage("Сервер остановлен", "WARNING");
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    int clientId = m_nextClientId++;
    m_clients[clientId] = socket;
    m_readBuffers[clientId] = QByteArray();

    // Подключение сигналов
    connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpServer::onClientError);


    // Отправка подтверждения подключения
    QJsonObject ack;
    ack["type"] = "connection_ack";
    ack["status"] = "connected";
    ack["client_id"] = clientId;
    ack["message"] = "Подключение к серверу установлено";

    QJsonDocument doc(ack);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    socket->flush();

    emit clientConnected(clientId, socket->peerAddress().toString(), socket->peerPort());
    emit logMessage(QString("Новое подключение: клиент %1").arg(clientId), "SUCCESS");
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int clientId = m_clients.key(socket, -1);
    if (clientId == -1) return;

    m_readBuffers[clientId].append(socket->readAll());

    // Обработка полных сообщений (разделенных \n)
    while (m_readBuffers[clientId].contains('\n')) {
        int index = m_readBuffers[clientId].indexOf('\n');
        QByteArray message = m_readBuffers[clientId].left(index);
        m_readBuffers[clientId].remove(0, index + 1);

        // Парсинг JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            emit logMessage(QString("Ошибка парсинга JSON от клиента %1: %2")
                                .arg(clientId).arg(parseError.errorString()), "ERROR");
            continue;
        }

        if (!doc.isObject()) {
            emit logMessage(QString("Некорректный JSON от клиента %1").arg(clientId), "ERROR");
            continue;
        }

        QJsonObject obj = doc.object();
        QString dataType = obj["type"].toString();

        if (dataType == "NetworkMetrics" || dataType == "DeviceStatus" || dataType == "Log") {
            QString content = QString::fromUtf8(message);
            emit dataReceived(clientId, dataType, content, QDateTime::currentDateTime());

            // Проверка критических значений
            if (dataType == "DeviceStatus") {
                int cpu = obj["cpu_usage"].toInt();
                int memory = obj["memory_usage"].toInt();

                if (cpu > m_settings["critical_cpu"].toInt()) {
                    emit logMessage(QString("Клиент %1: Критическая загрузка CPU: %2%%")
                                        .arg(clientId).arg(cpu), "WARNING");
                }
                if (memory > m_settings["critical_memory"].toInt()) {
                    emit logMessage(QString("Клиент %1: Критическая загрузка памяти: %2%%")
                                        .arg(clientId).arg(memory), "WARNING");
                }
            } else if (dataType == "NetworkMetrics") {
                double latency = obj["latency"].toDouble();
                if (latency > m_settings["critical_latency"].toDouble()) {
                    emit logMessage(QString("Клиент %1: Высокая задержка сети: %2 ms")
                                        .arg(clientId).arg(latency), "WARNING");
                }
            }
        } else {
            emit logMessage(QString("Неизвестный тип данных от клиента %1: %2")
                                .arg(clientId).arg(dataType), "WARNING");
        }
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int clientId = m_clients.key(socket, -1);
    if (clientId != -1) {
        m_clients.remove(clientId);
        m_readBuffers.remove(clientId);
        emit clientDisconnected(clientId);
        emit logMessage(QString("Клиент %1 отключен").arg(clientId), "WARNING");
    }

    socket->deleteLater();
}

void TcpServer::onClientError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int clientId = m_clients.key(socket, -1);
    emit logMessage(QString("Ошибка клиента %1: %2").arg(clientId).arg(socket->errorString()), "ERROR");
}

void TcpServer::sendCommandToClient(int clientId, const QString& command)
{
    if (m_clients.contains(clientId)) {
        QJsonObject cmd;
        cmd["type"] = "command";
        cmd["command"] = command;
        QJsonDocument doc(cmd);
        m_clients[clientId]->write(doc.toJson(QJsonDocument::Compact) + "\n");
        m_clients[clientId]->flush();
        emit logMessage(QString("Команда '%1' отправлена клиенту %2").arg(command).arg(clientId), "INFO");
    }
}

void TcpServer::broadcastCommand(const QString& command)
{
    for (int clientId : m_clients.keys()) {
        sendCommandToClient(clientId, command);
    }
}

void TcpServer::updateSettings(const QJsonObject& settings)
{
    m_settings = settings;
}
