#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onAccepted();

    void on_lineEdit_criticalCpu_editingFinished();

    void on_lineEdit_criticalMemory_editingFinished();

    void on_lineEdit_criticalLatency_editingFinished();

private:
    Ui::SettingsDialog *ui_;
    QSettings settings_;
};

#endif // SETTINGSDIALOG_H
