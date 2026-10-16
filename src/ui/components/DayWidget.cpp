#include "DayWidget.h"
#include <QContextMenuEvent>
#include <QMouseEvent>

DayWidget::DayWidget(int dayNumber, const QString& dateKey, QWidget* parent)
    : QWidget(parent), m_dayNumber(dayNumber), m_dateKey(dateKey)
{
    setFixedSize(88, 50);
    setStyleSheet("background-color: #F8F9FA; border: 1px solid #DEE2E6; border-radius: 4px;");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);

    auto* topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);

    m_numberLabel = new QLabel(QString::number(dayNumber), this);
    m_numberLabel->setStyleSheet("font-weight: bold; color: #333333; border: none; background: transparent;");
    topLayout->addWidget(m_numberLabel);

    topLayout->addStretch();

    // Petit point rouge pour l'indicateur d'événement
    m_flagLabel = new QLabel("●", this);
    m_flagLabel->setStyleSheet("color: #DC3545; font-size: 10px; border: none; background: transparent;");
    m_flagLabel->setVisible(false);
    topLayout->addWidget(m_flagLabel);

    layout->addLayout(topLayout);

    m_contentLabel = new QLabel("", this);
    m_contentLabel->setStyleSheet("font-size: 8px; color: #6C757D; border: none; background: transparent;");
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_contentLabel->setWordWrap(true);
    layout->addWidget(m_contentLabel);
}

void DayWidget::setEventContent(const QString& content, bool hasEvent)
{
    m_contentLabel->setText(content);
    m_flagLabel->setVisible(hasEvent);
}

void DayWidget::contextMenuEvent(QContextMenuEvent* event)
{
    emit rightClicked(this, event->globalPos());
    event->accept();
}

void DayWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
}