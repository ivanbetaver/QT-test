#include "tcpserver.h"
#include <QOverload>
#include <QAbstractSocket>
#include <QSettings>
#include <QJsonDocument>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
    // Получение параметров подключения
    QSettings settings("config.ini", QSettings::IniFormat);
    m_port = settings.value("server/port").toInt();
 }

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::start()
{
    if (listen(QHostAddress::Any, m_port)) {
        emit logMessage(QString("Сервер запущен на порту %1").arg(m_port), "SUCCESS");
    } else {
        emit logMessage(QString("Ошибка запуска сервера: %1").arg(errorString()), "ERROR");
    }
}

void TcpServer::stop()
{
    for (QTcpSocket* socket : m_clients.values()) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_clients.clear();
    close();
    emit logMessage("Сервер остановлен", "WARNING");
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    // Попытка подключения
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

   // Подключение сигналов
    connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpServer::onClientError);
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    auto it = m_sockets.find(socket);
    // Подтверждение подключения
    if (it == m_sockets.end()) {
        QDataStream stream(socket);
        QString clientId;
        stream >> clientId;
        m_clients[clientId] = socket;
        m_sockets[socket] = clientId;
        emit clientConnected(clientId, socket->peerAddress().toString(), socket->peerPort());
        emit logMessage(QString("Клиент %1 подключен (IP: %2, Порт: %3)")
                            .arg(clientId).arg(socket->peerAddress().toString()).arg(socket->peerPort()), "SUCCESS");

        QJsonObject ack;
        ack["type"] = "connection_ack";
        ack["status"] = "connected";
        ack["message"] = "Подключение к серверу установлено";

        QJsonDocument doc(ack);
        socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
        socket->flush();
        return;
    }

    QString clientId = it.value();
    QByteArray buffer = socket->readAll();

    // Обработка полных сообщений (разделенных \n)
    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        QString message = QString::fromUtf8(buffer.left(index));
        emit messageReceived(clientId, message);
        buffer.remove(0, index + 1);
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    auto it = m_sockets.find(socket);
    if (it != m_sockets.end()) {
        const QString& clientId = it.value();
        emit clientDisconnected(clientId);
        emit logMessage(QString("Клиент %1 отключен").arg(clientId), "WARNING");
        m_clients.remove(clientId);
        m_sockets.remove(socket);
    }

    socket->deleteLater();
}

void TcpServer::onClientError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    const QString& clientId = m_clients.key(socket, "Неизвестный клиент");
    emit logMessage(QString("Ошибка клиента %1: %2").arg(clientId).arg(socket->errorString()), "ERROR");
}

void TcpServer::sendCommandToClient(const QString& clientId, const QString& command)
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
    for (const QString& clientId : m_clients.keys()) {
        sendCommandToClient(clientId, command);
    }
}

void TcpServer::updateSettings(const QJsonObject& settings)
{
    m_settings = settings;
}
