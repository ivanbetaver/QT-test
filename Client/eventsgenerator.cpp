#include "eventsgenerator.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>

EventsGenerator::EventsGenerator()
{

}

QString EventsGenerator::generateRandom()
{
    // Случайный выбор типа данных (NetworkMetrics, DeviceStatus, Log)
    int type = QRandomGenerator::global()->bounded(3);

    switch (type) {
    case 0:
        return generateNetworkMetrics();
    case 1:
        return generateDeviceStatus();
    case 2:
        return generateLog();
    }
    return QString();
}

QString EventsGenerator::generateNetworkMetrics()
{
    QJsonObject metrics;
    metrics["type"] = "NetworkMetrics";
    metrics["bandwidth"] = QRandomGenerator::global()->bounded(10, 1000) / 10.0; // 1.0 - 100.0 Mbps
    metrics["latency"] = QRandomGenerator::global()->bounded(5, 150); // 5-150 ms
    metrics["packet_loss"] = QRandomGenerator::global()->bounded(0, 100) / 1000.0; // 0-0.1%

    QJsonDocument doc(metrics);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString EventsGenerator::generateDeviceStatus()
{
    QJsonObject status;
    status["type"] = "DeviceStatus";
    status["uptime"] = QRandomGenerator::global()->bounded(0, 86400); // 0-24 часов в секундах
    status["cpu_usage"] = QRandomGenerator::global()->bounded(0, 100);
    status["memory_usage"] = QRandomGenerator::global()->bounded(0, 100);

    QJsonDocument doc(status);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString EventsGenerator::generateLog()
{
    QStringList severities = {"INFO", "WARNING", "ERROR", "DEBUG"};
    QStringList messages = {
                            "Интерфейс eth0 перезапущен",
                            "Обнаружена высокая загрузка сети",
                            "Пакет потерян при передаче",
                            "Устройство успешно синхронизировано",
                            "Критическая ошибка в работе модуля связи",
                            "Обновление конфигурации применено",
                            "Таймаут соединения с удаленным узлом",
                            "Буфер обмена данных очищен",
                            "Запущена диагностика сети",
                            "Обнаружено подозрительное сетевое подключение"
    };

    // Генерация сообщения разной длины (короткое, среднее, длинное)
    QString message;
    int lengthType = QRandomGenerator::global()->bounded(3);

    if (lengthType == 0) {
        // Короткое сообщение (до 50 символов)
        message = messages[QRandomGenerator::global()->bounded(messages.size())].left(50);
    } else if (lengthType == 1) {
        // Среднее сообщение (50-200 символов)
        message = generateRandomString(50, 200);
    } else {
        // Длинное сообщение (200+ символов)
        message = generateRandomString(200, 500);
    }

    QJsonObject log;
    log["type"] = "Log";
    log["severity"] = severities[QRandomGenerator::global()->bounded(severities.size())];
    log["message"] = message;

    QJsonDocument doc(log);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString EventsGenerator::generateRandomString(int minLen, int maxLen)
{
    int length = QRandomGenerator::global()->bounded(minLen, maxLen + 1);
    const QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*() ";

    QString result;
    for (int i = 0; i < length; ++i) {
        result += characters[QRandomGenerator::global()->bounded(characters.length())];
    }
    return result;
}
