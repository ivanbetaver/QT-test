#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::SettingsDialog),
    settings_("config.ini", QSettings::IniFormat) {
    ui_->setupUi(this);

    ui_->lineEdit_criticalCpu->setText(settings_.value("settings/critical_cpu").toString());
    ui_->lineEdit_criticalMemory->setText(settings_.value("settings/critical_memory").toString());
    ui_->lineEdit_criticalLatency->setText(settings_.value("settings/critical_latency").toString());

    connect(this, &QDialog::accepted, this, &SettingsDialog::onAccepted);
}

SettingsDialog::~SettingsDialog() {
    delete ui_;
}

void SettingsDialog::onAccepted() {
    qDebug().noquote() << QString("Сохранение значений");
    settings_.sync();
}

void SettingsDialog::on_lineEdit_criticalCpu_editingFinished() {
    qDebug().noquote() << QString("Новое значение критической загрузки CPU: [%1]").arg(ui_->lineEdit_criticalCpu->text());
    settings_.setValue("settings/critical_cpu", ui_->lineEdit_criticalCpu->text());
}


void SettingsDialog::on_lineEdit_criticalMemory_editingFinished() {
    qDebug().noquote() << QString("Новое значение критической памяти: [%1]").arg(ui_->lineEdit_criticalMemory->text());
    settings_.setValue("settings/critical_memory", ui_->lineEdit_criticalMemory->text());
}


void SettingsDialog::on_lineEdit_criticalLatency_editingFinished() {
    qDebug().noquote() << QString("Новое значение критической задержки сети CPU: [%1]").arg(ui_->lineEdit_criticalLatency->text());
    settings_.setValue("settings/critical_latency", ui_->lineEdit_criticalLatency->text());
}

