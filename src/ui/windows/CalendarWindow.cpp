#include "CalendarWindow.h"
#include "../dialogs/AddEventDialog.h"
#include "../../core/managers/EventManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QLocale>
#include <QDateTime>

CalendarWindow::CalendarWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Calendrier"));
    resize(670, 430);
    setStyleSheet("background-color: #E9ECEF;");

    QDateTime now = QDateTime::currentDateTime();
    m_currentMonth = now.date().month();
    m_currentYear = now.date().year();

    m_eventsMap = EventManager::loadEvents();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_dateTitle = new QLabel(this);
    m_dateTitle->setAlignment(Qt::AlignCenter);
    m_dateTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #212529;");
    mainLayout->addWidget(m_dateTitle);

    auto* weekHeaderLayout = new QHBoxLayout();
    QStringList daysOfWeek = { tr("Lundi"), tr("Mardi"), tr("Mercredi"), tr("Jeudi"), tr("Vendredi"), tr("Samedi"), tr("Dimanche") };
    for (const QString& dayName : daysOfWeek) {
        auto* lbl = new QLabel(dayName, this);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("font-weight: bold; font-size: 13px; color: #495057;");
        lbl->setFixedSize(88, 25);
        weekHeaderLayout->addWidget(lbl);
    }
    mainLayout->addLayout(weekHeaderLayout);

    m_dayGrid = new QGridLayout();
    m_dayGrid->setSpacing(2);
    mainLayout->addLayout(m_dayGrid);

    auto* bottomLayout = new QHBoxLayout();
    auto* btnPrevious = new QPushButton(tr("Précédent"), this);
    auto* btnNext = new QPushButton(tr("Suivant"), this);
    auto* btnBack = new QPushButton(tr("Retour"), this);

    bottomLayout->addStretch();
    bottomLayout->addWidget(btnPrevious);
    bottomLayout->addWidget(btnNext);
    bottomLayout->addWidget(btnBack);
    mainLayout->addLayout(bottomLayout);

    connect(btnPrevious, &QPushButton::clicked, this, &CalendarWindow::onPreviousClicked);
    connect(btnNext, &QPushButton::clicked, this, &CalendarWindow::onNextClicked);
    connect(btnBack, &QPushButton::clicked, this, &CalendarWindow::onBackClicked);

    displayDays();
}

void CalendarWindow::displayDays()
{
    QLayoutItem* item;
    while ((item = m_dayGrid->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    QDate firstDayOfMonth(m_currentYear, m_currentMonth, 1);
    int daysInMonth = firstDayOfMonth.daysInMonth();
    int dayOfWeek = firstDayOfMonth.dayOfWeek();

    QString monthName = QLocale::system().monthName(m_currentMonth, QLocale::LongFormat);
    monthName[0] = monthName[0].toUpper();
    m_dateTitle->setText(monthName + " " + QString::number(m_currentYear));

    int row = 0, col = dayOfWeek - 1;
    for (int i = 0; i < col; ++i) {
        auto* blank = new QWidget(this);
        blank->setFixedSize(88, 50);
        blank->setStyleSheet("background: transparent; border: none;");
        m_dayGrid->addWidget(blank, row, i);
    }

    for (int day = 1; day <= daysInMonth; ++day) {
        QString monthStr = monthName.toLower();
        QString dateKeyShort = QString("%1 %2").arg(day).arg(monthStr);

        auto* dayWidget = new DayWidget(day, dateKeyShort, this);
        connect(dayWidget, &DayWidget::rightClicked, this, &CalendarWindow::handleDayRightClicked);

        if (m_eventsMap.contains(dateKeyShort)) {
            EventData ev = m_eventsMap[dateKeyShort];
            if (ev.isAnnual || ev.year == QString::number(m_currentYear)) {
                dayWidget->setEventContent(ev.content, true);
            }
        }

        m_dayGrid->addWidget(dayWidget, row, col);
        col++;
        if (col > 6) {
            col = 0;
            row++;
        }
    }
}

void CalendarWindow::onPreviousClicked() {
    m_currentMonth--;
    if (m_currentMonth < 1) { m_currentMonth = 12; m_currentYear--; }
    displayDays();
}

void CalendarWindow::onNextClicked() {
    m_currentMonth++;
    if (m_currentMonth > 12) { m_currentMonth = 1; m_currentYear++; }
    displayDays();
}

void CalendarWindow::onBackClicked() { close(); }

void CalendarWindow::handleDayRightClicked(DayWidget* widget, const QPoint& globalPos)
{
    m_activeDayWidget = widget;
    m_activeDateKey = widget->getDateKey();
    QString monthName = QLocale::system().monthName(m_currentMonth, QLocale::LongFormat);
    m_activeFullDateStr = QString("%1 %2 %3").arg(widget->getDayNumber()).arg(monthName).arg(m_currentYear);

    QMenu contextMenu(this);
    QAction* addAction = contextMenu.addAction(tr("Ajouter rappel"));
    QAction* delAction = contextMenu.addAction(tr("Supprimer rappel"));

    QAction* selectedAction = contextMenu.exec(globalPos);
    if (selectedAction == addAction) addRappel();
    else if (selectedAction == delAction) delRappel();
}

void CalendarWindow::addRappel()
{
    AddEventDialog dialog(m_activeFullDateStr, this);
    if (dialog.exec() == QDialog::Accepted && !dialog.text().isEmpty()) {
        EventData ev;
        ev.content = dialog.text();
        QString rec = dialog.recurrenceType();
        ev.isAnnual = (rec == "annual");
        ev.year = QString::number(m_currentYear);

        m_eventsMap[m_activeDateKey] = ev;
        EventManager::saveEvents(m_eventsMap);
        displayDays();
    }
}

void CalendarWindow::delRappel()
{
    if (m_eventsMap.contains(m_activeDateKey)) {
        m_eventsMap.remove(m_activeDateKey);
        EventManager::saveEvents(m_eventsMap);
        displayDays();
    }
}