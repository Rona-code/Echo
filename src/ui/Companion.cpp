#include "Companion.h"
#include <QPainter>

Companion::Companion(QWidget* parent)
    : QWidget(parent)
{
    //m_scale = 0.2f;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);

    setSprite("Asset/Renard/renards_Idle0.png");
}

void Companion::setScale(float newScale)
{
    if (newScale <= 0.01f || m_scale == newScale) return;

    m_scale = newScale;
    if (!m_currentImagePath.isEmpty()) {
        setSprite(m_currentImagePath);
    }
}

void Companion::setSprite(const QString& imagePath)
{
    m_currentImagePath = imagePath;
    m_pixmap = QPixmap(imagePath);

    if (!m_pixmap.isNull())
    {
        int newW = qMax(1, static_cast<int>(m_pixmap.width() * m_scale));
        int newH = qMax(1, static_cast<int>(m_pixmap.height() * m_scale));

        setFixedSize(newW, newH);

        update();
    }
}

void Companion::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (m_pixmap.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawPixmap(rect(), m_pixmap);
}

void Companion::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void Companion::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}