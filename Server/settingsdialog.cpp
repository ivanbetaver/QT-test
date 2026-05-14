#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadSettings();
    setWindowTitle("Настройки сервера");
    setModal(true);
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* group = new QGroupBox("Критические значения", this);
    QFormLayout* formLayout = new QFormLayout(group);

    m_criticalCpu = new QLineEdit(this);
    m_criticalMemory = new QLineEdit(this);
    m_criticalLatency = new QLineEdit(this);

    formLayout->addRow("Критическая загрузка CPU (%):", m_criticalCpu);
    formLayout->addRow("Критическая загрузка памяти (%):", m_criticalMemory);
    formLayout->addRow("Критическая задержка (ms):", m_criticalLatency);

    mainLayout->addWidget(group);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Сохранить", this);
    m_cancelButton = new QPushButton("Отмена", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::loadSettings()
{
    QSettings settings("TelecomSystem", "Server");
    m_criticalCpu->setText(settings.value("critical_cpu", "80").toString());
    m_criticalMemory->setText(settings.value("critical_memory", "85").toString());
    m_criticalLatency->setText(settings.value("critical_latency", "100").toString());
}

void SettingsDialog::accept()
{
    QSettings settings("TelecomSystem", "Server");
    settings.setValue("critical_cpu", m_criticalCpu->text().toInt());
    settings.setValue("critical_memory", m_criticalMemory->text().toInt());
    settings.setValue("critical_latency", m_criticalLatency->text().toDouble());

    QDialog::accept();
}

QJsonObject SettingsDialog::getSettings() const
{
    QJsonObject settings;
    settings["critical_cpu"] = m_criticalCpu->text().toInt();
    settings["critical_memory"] = m_criticalMemory->text().toInt();
    settings["critical_latency"] = m_criticalLatency->text().toDouble();
    return settings;
}