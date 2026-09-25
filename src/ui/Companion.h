#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMouseEvent>
#include <QPoint>

class Companion : public QWidget
{
    Q_OBJECT

public:
    explicit Companion(QWidget* parent = nullptr);

    float scale() const { return m_scale; }
    void setScale(float newScale);

    void setSprite(const QString& imagePath);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPixmap m_pixmap;
    QString m_currentImagePath;
    QPoint m_dragPosition;

    float m_scale{ 0.5f };
};