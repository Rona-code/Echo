#pragma once
#include <QString>
#include <QDate>
#include <QMap>

struct EventData {
    QString content;
    bool isAnnual{ false };
    bool isMonthly{ false };
    QString year;
};

class EventManager {
public:
    static QString getAppdataPath();
    static QMap<QString, EventData> loadEvents();
    static void saveEvents(const QMap<QString, EventData>& eventsMap);
    static QString getEventTextForDate(const QDate& targetDate);
};