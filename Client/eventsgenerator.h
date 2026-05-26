#ifndef EVENTSGENERATOR_H
#define EVENTSGENERATOR_H

#include <QObject>

class EventsGenerator
{
public:
    EventsGenerator();
    static QString generateRandom();

private:
    static QString generateNetworkMetrics();
    static QString generateDeviceStatus();
    static QString generateLog();
    static QString generateRandomString(int minLen, int maxLen);
};

#endif // EVENTSGENERATOR_H
