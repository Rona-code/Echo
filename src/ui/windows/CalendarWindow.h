#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMap>
#include "../components/DayWidget.h"
#include "../../core/managers/EventManager.h"

class CalendarWindow : public QWidget {
    Q_OBJECT
public:
    explicit CalendarWindow(QWidget* parent = nullptr);

private slots:
    void displayDays();
    void onPreviousClicked();
    void onNextClicked();
    void onBackClicked();
    void handleDayRightClicked(DayWidget* widget, const QPoint& globalPos);
    void addRappel();
    void delRappel();

private:
    int m_currentMonth;
    int m_currentYear;

    QLabel* m_dateTitle{ nullptr };
    QGridLayout* m_dayGrid{ nullptr };
    QLineEdit* m_rappelTimeInput{ nullptr };

    QMap<QString, EventData> m_eventsMap;

    DayWidget* m_activeDayWidget{ nullptr };
    QString m_activeDateKey;
    QString m_activeFullDateStr;
};