#include "clientstablemodel.h"
#include <QTableView>

ClientsTableModel::ClientsTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
    columns_ = {"ID клиента", "IP-адрес", "Статус"};
}

void ClientsTableModel::addClientToTable(const QString& clientId, const QString &ip, const QString &status) {
    int row = clients_.size();
    beginInsertRows(QModelIndex(), row, row);
    clients_.push_back(Client{clientId, ip, status});
    index_[clientId] = clients_.size() - 1;
    endInsertRows();
}

void ClientsTableModel::updateClientStatus(const QString &clientId, const QString &status) {
    int row = index_[clientId];

    if (row < 0 || row >= clients_.size())
        return;

    clients_[row].status = status;

    const QModelIndex& index = this->index(row, 2);  // Колонка статуса
    emit dataChanged(index, index);
}

void ClientsTableModel::removeClientFromTable(const QString& clientId) {
    int row = index_[clientId];

    if (row < 0 || row >= clients_.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    clients_.remove(row);
    endRemoveRows();
}

QVariant ClientsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section >= 0 && section < columns_.size())
                return columns_[section];
        }
    }

    return QVariant();
}

int ClientsTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return clients_.size();
}

int ClientsTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return columns_.size();
}

QVariant ClientsTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= clients_.size() ||
        col < 0 || col >= columns_.size()) {
        return QVariant();
    }

    const Client& client = clients_[row];

    if (role == Qt::DisplayRole) {
        switch (col) {
        case 0: return client.id;
        case 1: return client.ip;
        case 2: return client.status;
        default: return QVariant();
        }
    }

    return QVariant();
}
