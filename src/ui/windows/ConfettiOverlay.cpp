#include "ConfettiOverlay.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QRandomGenerator>

ConfettiOverlay::ConfettiOverlay(QWidget* parent) : QWidget(parent) {
    // Fenêtre transparente, par-dessus tout, sans bordure, qui ignore les clics (les clics traversent)
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Occupe tout l'écran principal ou la zone globale
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setGeometry(screenGeometry);

    // Générer 150 confettis aux couleurs aléatoires
    for (int i = 0; i < 150; ++i) {
        ConfettiParticle p;
        p.pos = QPointF(QRandomGenerator::global()->bounded(width()), QRandomGenerator::global()->bounded(-height(), 0));
        p.color = QColor(QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256));
        p.speed = QRandomGenerator::global()->bounded(2, 7);
        p.rotation = QRandomGenerator::global()->bounded(360);
        p.rotSpeed = QRandomGenerator::global()->bounded(-5, 6);
        p.size = QRandomGenerator::global()->bounded(6, 14);
        m_particles.append(p);
    }

    // Timer d'animation à ~60 FPS (16 ms)
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_durationCounter++;
        // Animation des particules
        for (auto& p : m_particles) {
            p.pos.ry() += p.speed;
            p.rotation += p.rotSpeed;
            // Si ça dépasse l'écran en bas, on le remet en haut
            if (p.pos.y() > height()) {
                p.pos.setY(QRandomGenerator::global()->bounded(-50, 0));
                p.pos.setX(QRandomGenerator::global()->bounded(width()));
            }
        }
        update();

        // Arrêter l'effet après 5 secondes (300 ticks * 16ms)
        if (m_durationCounter > 300) {
            close();
            deleteLater();
        }
        });
    m_timer->start(16);
    show();
}

void ConfettiOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto& p : m_particles) {
        painter.save();
        painter.translate(p.pos);
        painter.rotate(p.rotation);
        painter.setBrush(p.color);
        painter.setPen(Qt::NoPen);
        // Dessiner un petit rectangle de confetti
        painter.drawRect(QRectF(-p.size / 2, -p.size / 4, p.size, p.size / 2));
        painter.restore();
    }
}