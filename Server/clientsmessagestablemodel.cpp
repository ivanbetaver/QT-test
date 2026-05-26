#include "clientsmessagestablemodel.h"
#include <QJsonDocument>
#include <QJsonObject>

ClientsMessagesTableModel::ClientsMessagesTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
    columns_ = {"ID клиента", "Тип данных", "Содержимое", "Время получения"};
}


void ClientsMessagesTableModel::addMessageToTable(const QString& clientId, const QString& type, const QString& message, const QDateTime& time) {
    int row = messages_.size();
    beginInsertRows(QModelIndex(), row, row);
    messages_.push_back(Message{clientId, type, formatMessageByType(type, message), time});
    endInsertRows();
}

QVariant ClientsMessagesTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section >= 0 && section < columns_.size())
                return columns_[section];
        }
    }

    return QVariant();
}

int ClientsMessagesTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return messages_.size();
}

int ClientsMessagesTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return columns_.size();
}

QVariant ClientsMessagesTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= messages_.size() ||
        col < 0 || col >= columns_.size()) {
        return QVariant();
    }

    const Message& message = messages_[row];

   if (role == Qt::DisplayRole) {
        switch (col) {
        case 0: return message.clientId;
        case 1: return message.type;
        case 2: return message.message;
        case 3: return message.time;
        default: return QVariant();
        }
    }

    return QVariant();
}

QString ClientsMessagesTableModel::formatMessageByType(const QString& type, const QString& message)
{
    // Парсинг JSON для отображения в логах
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (type == "NetworkMetrics") {
            return QString("Пропускная способность: %1 Mbps, Задержка: %2 ms, Потери: %3%")
                .arg(obj["bandwidth"].toDouble())
                .arg(obj["latency"].toDouble())
                .arg(obj["packet_loss"].toDouble());
        } else if (type == "DeviceStatus") {
            return QString("Время работы: %1 сек, CPU: %2%, Память: %3%")
                .arg(obj["uptime"].toInt())
                .arg(obj["cpu_usage"].toInt())
                .arg(obj["memory_usage"].toInt());
        } else if (type == "Log") {
            return QString("[%1] %2").arg(obj["severity"].toString()).arg(obj["message"].toString());
        }
    }
    return message;
}

