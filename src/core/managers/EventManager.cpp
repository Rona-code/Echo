#include "EventManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

QString EventManager::getAppdataPath()
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/EchoData";
    QDir().mkpath(dirPath);
    return dirPath + "/calendar_events.json";
}

QMap<QString, EventData> EventManager::loadEvents()
{
    QMap<QString, EventData> eventsMap;
    QFile file(getAppdataPath());
    if (!file.open(QIODevice::ReadOnly)) return eventsMap;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return eventsMap;

    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QJsonObject eventObj = it.value().toObject();
        EventData ev;
        ev.content = eventObj["content"].toString();
        ev.isMonthly = eventObj["isMonthly"].toBool();
        ev.isAnnual = eventObj["isAnnual"].toBool();
        ev.year = eventObj["year"].toString();
        eventsMap[it.key()] = ev;
    }
    return eventsMap;
}

void EventManager::saveEvents(const QMap<QString, EventData>& eventsMap)
{
    QJsonObject obj;
    for (auto it = eventsMap.begin(); it != eventsMap.end(); ++it) {
        QJsonObject eventObj;
        eventObj["content"] = it.value().content;
        eventObj["isMonthly"] = it.value().isMonthly;
        eventObj["isAnnual"] = it.value().isAnnual;
        eventObj["year"] = it.value().year;
        obj[it.key()] = eventObj;
    }

    QFile file(getAppdataPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
    }
}

QString EventManager::getEventTextForDate(const QDate& targetDate)
{
    QMap<QString, EventData> eventsMap = loadEvents();


    QString targetDateIso = targetDate.toString("yyyy-MM-dd"); // "2026-10-14"

    QString monthName = QLocale::system().monthName(targetDate.month(), QLocale::LongFormat).toLower();
    QString targetTextKey = QString("%1 %2").arg(targetDate.day()).arg(monthName);

    for (auto it = eventsMap.begin(); it != eventsMap.end(); ++it) {
        QString key = it.key();
        const EventData& ev = it.value();

        // On vérifie si la clé correspond soit au format ISO, soit au format textuel "14 octobre"
        if (key == targetDateIso || key == targetTextKey ||
            (ev.isAnnual && key.endsWith(targetDate.toString("MM-dd"))) ||
            (ev.isMonthly && key.startsWith(QString::number(targetDate.day()) + " "))) {
            return ev.content;
        }
    }
    return QString();
}