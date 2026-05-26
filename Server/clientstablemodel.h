#ifndef CLIENTSTABLEMODEL_H
#define CLIENTSTABLEMODEL_H

#include <QAbstractTableModel>


class ClientsTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    struct Client {
        QString id;
        QString ip;
        QString status;
    };

    explicit ClientsTableModel(QObject *parent = nullptr);
    void addClientToTable(const QString& clientId, const QString& ip, const QString& status);
    void updateClientStatus(const QString& clientId, const QString& status);
    void removeClientFromTable(const QString& clientId);

protected:
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStringList columns_;
    QVector<Client> clients_;
    QMap<QString, int> index_;
};

#endif // CLIENTSTABLEMODEL_H
