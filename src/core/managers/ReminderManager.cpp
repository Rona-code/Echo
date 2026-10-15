#include "ReminderManager.h"
#include "EventManager.h"
#include <QSettings>
#include <QTime>

ReminderManager::ReminderManager(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ReminderManager::checkReminders);
    m_timer->start(10000); // Vérification toute les 10 secondes
}

void ReminderManager::checkReminders()
{
    QSettings settings;
    QString rappelTimeStr = settings.value("explorer/rappelTime", "08:00").toString();

    QTime targetTime = QTime::fromString(rappelTimeStr, "H:mm");
    if (!targetTime.isValid()) targetTime = QTime(8, 0);

    QTime currentTime = QTime::currentTime();
    QDate currentDate = QDate::currentDate();

    // Vérifie l'heure et s'assure de ne notifier qu'une seule fois par jour
    if (currentTime.hour() == targetTime.hour() &&
        currentTime.minute() == targetTime.minute() &&
        m_lastNotifiedDate != currentDate) {

        QString eventText = EventManager::getEventTextForDate(currentDate);
        if (!eventText.isEmpty()) {
            m_lastNotifiedDate = currentDate;
            emit reminderTriggered(eventText);
        }
    }
}