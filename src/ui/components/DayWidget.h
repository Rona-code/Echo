#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class DayWidget : public QWidget {
    Q_OBJECT
public:
    explicit DayWidget(int dayNumber, const QString& dateKey, QWidget* parent = nullptr);

    void setEventContent(const QString& content, bool hasEvent);
    QString getDateKey() const { return m_dateKey; }
    int getDayNumber() const { return m_dayNumber; }

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    void rightClicked(DayWidget* widget, const QPoint& globalPos);

private:
	void mousePressEvent(QMouseEvent* event) override;

    int m_dayNumber;
    QString m_dateKey;
    QLabel* m_numberLabel;
    QLabel* m_contentLabel;
    QLabel* m_flagLabel;
};