#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientsmanager.h"
#include "settingsdialog.h"
#include <QDateTime>

MainWindow::MainWindow(ClientsManager* clientsManager, QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    clientsManager_(clientsManager) {

    ui_->setupUi(this);

    ui_->tableView_clients->setModel(clientsManager->getClientsTableModel());
    QHeaderView* tableView_clients_header = ui_->tableView_clients->horizontalHeader();
    tableView_clients_header->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(clientsManager, &ClientsManager::clientAdded, this, [=]() {
        ui_->tableView_clients->scrollToBottom();
    });

    ui_->tableView_messages->setModel(clientsManager->getClientsMessagesTableModel());
    QHeaderView* tableView_messages_header = ui_->tableView_messages->horizontalHeader();
    tableView_messages_header->setSectionResizeMode(0, QHeaderView::Stretch);
    tableView_messages_header->setSectionResizeMode(2, QHeaderView::Stretch);
    connect(clientsManager, &ClientsManager::messageAdded, this, [=]() {
        ui_->tableView_messages->scrollToBottom();
    });

    onMetricsIsActiveChanged();
    ui_->statusBar->showMessage(clientsManager_->metricsIsActive() ? "Метрики запущены" : "Метрики остановлены");

    connect(clientsManager, &ClientsManager::logServerMessage, this, &MainWindow::onLogServerMessage);
    connect(clientsManager, &ClientsManager::metricsIsActiveChanged, this, &MainWindow::onMetricsIsActiveChanged);
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::on_activateClientsButton_clicked() {
    if (clientsManager_->metricsIsActive()) {
        clientsManager_->stopMetrics();
    } else {
        clientsManager_->startMetrics();
    }
}


void MainWindow::on_pushButton_clearLog_clicked() {
    ui_->textEdit_serverLog->clear();
}

void MainWindow::on_pushButton_settings_clicked() {
    SettingsDialog settingsDialog(this);
    if (settingsDialog.exec() == QDialog::Accepted) {
        clientsManager_->updateCriticalLimits();
    }
}

void MainWindow::onLogServerMessage(const QString &message, const QString &severity) {
    ui_->textEdit_serverLog->append(formatMessage(message, severity));
}

void MainWindow::onMetricsIsActiveChanged() {
    if (clientsManager_->metricsIsActive()) {
        ui_->activateClientsButton->setText("Остановить метрики");
        ui_->statusBar->showMessage("Метрики запущены");
    } else {
        ui_->activateClientsButton->setText("Запустить метрики");
        ui_->statusBar->showMessage("Метрики остановлены");
    }
}

QString MainWindow::formatMessage(const QString& message, const QString& severity) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formattedMessage;

    if (severity == "ERROR") {
        formattedMessage = QString("[%1] [ОШИБКА] %2").arg(timestamp).arg(message);
    } else if (severity == "WARNING") {
        formattedMessage = QString("[%1] [ПРЕДУПРЕЖДЕНИЕ] %2").arg(timestamp).arg(message);
    } else if (severity == "SUCCESS") {
        formattedMessage = QString("[%1] [УСПЕХ] %2").arg(timestamp).arg(message);
    } else {
        formattedMessage = QString("[%1] [ИНФО] %2").arg(timestamp).arg(message);
    }

    return formattedMessage;
}
