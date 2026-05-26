#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ClientsManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ClientsManager* clientsManager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_activateClientsButton_clicked();
    void on_pushButton_clearLog_clicked();
    void on_pushButton_settings_clicked();
    void onLogServerMessage(const QString &message, const QString &severity);
    void onMetricsIsActiveChanged();

private:
    QString formatMessage(const QString &message, const QString &severity);

private:
    Ui::MainWindow* ui_;
    ClientsManager* clientsManager_;
};

#endif // MAINWINDOW_H
