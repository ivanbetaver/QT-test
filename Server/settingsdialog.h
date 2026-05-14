#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QJsonObject>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QJsonObject getSettings() const;

private slots:
    void accept() override;
    void loadSettings();

private:
    void setupUI();

    QLineEdit* m_criticalCpu;
    QLineEdit* m_criticalMemory;
    QLineEdit* m_criticalLatency;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
};

#endif // SETTINGSDIALOG_H