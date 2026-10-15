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

    QString targetDateIso = targetDate.toString("yyyy-MM-dd");
    QString monthName = QLocale::system().monthName(targetDate.month(), QLocale::LongFormat).toLower();
    QString targetTextKey = QString("%1 %2").arg(targetDate.day()).arg(monthName);

    for (auto it = eventsMap.begin(); it != eventsMap.end(); ++it) {
        QString key = it.key();
        const EventData& ev = it.value();

        if (key == targetDateIso || key == targetTextKey) {
            return ev.content;
        }

        // 2. Récurrence Annuelle : MÊME JOUR et MÊME MOIS (tous les ans)
        if (ev.isAnnual) {
            QDate originalDate = QDate::fromString(key, "yyyy-MM-dd");
            if (originalDate.isValid()) {
                if (originalDate.day() == targetDate.day() && originalDate.month() == targetDate.month()) {
                    return ev.content;
                }
            }
            else {
                QStringList parts = key.split(' ');
                if (parts.size() >= 2) {
                    int origDay = parts[0].toInt();
                    QString origMonth = parts[1];
                    if (origDay == targetDate.day() && origMonth == monthName) {
                        return ev.content;
                    }
                }
            }
        }
    }
    return QString();
}