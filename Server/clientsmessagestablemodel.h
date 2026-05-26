#ifndef CLIENTSMESSAGESTABLEMODEL_H
#define CLIENTSMESSAGESTABLEMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>

class ClientsMessagesTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    struct Message {
        QString clientId;
        QString type;
        QString message;
        QDateTime time;
    };

    explicit ClientsMessagesTableModel(QObject *parent = nullptr);
    void addMessageToTable(const QString& clientId, const QString& type, const QString& message, const QDateTime& time);

protected:
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QString formatMessageByType(const QString &type, const QString &message);

private:
    QStringList columns_;
    QVector<Message> messages_;
};

#endif // CLIENTSMESSAGESTABLEMODEL_H
