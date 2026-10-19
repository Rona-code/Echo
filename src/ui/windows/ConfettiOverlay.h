#pragma once
#include <QWidget>
#include <QTimer>
#include <QList>

struct ConfettiParticle {
    QPointF pos;
    QColor color;
    double speed;
    double rotation;
    double rotSpeed;
    double size;
};

class ConfettiOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ConfettiOverlay(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QTimer* m_timer;
    QList<ConfettiParticle> m_particles;
    int m_durationCounter = 0;
};