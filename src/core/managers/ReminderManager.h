#pragma once
#include <QObject>
#include <QTimer>
#include <QDate>

class ReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit ReminderManager(QObject* parent = nullptr);

signals:
    void reminderTriggered(const QString& eventText);

private slots:
    void checkReminders();

private:
    QTimer* m_timer{ nullptr };
    QDate m_lastNotifiedDate;
};