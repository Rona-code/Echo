#include "Companion.h"
#include "ActionMenu.h"
#include "../core/CompanionMenuManager.h"
#include "../core/AnimationEngine.h"

#include <QPainter>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QRandomGenerator>

Companion::Companion(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(64, 64);

    m_animationEngine = new AnimationEngine(this);
    connect(m_animationEngine, &AnimationEngine::frameChanged, this, &Companion::setSprite);

    m_inactivityTimer = new QTimer(this);
    m_inactivityTimer->setSingleShot(true);
    connect(m_inactivityTimer, &QTimer::timeout, this, [this]() {
        m_behavior.setState(CompanionState::Sleeping);
        m_behavior.setTicks(QRandomGenerator::global()->bounded(200, 6001));
        if (m_animationEngine) m_animationEngine->setState(AnimationEngine::AnimationState::Sleeping);
        });

    m_behaviorTimer = new QTimer(this);
    connect(m_behaviorTimer, &QTimer::timeout, this, [this]() {
        QScreen* screen = QGuiApplication::screenAt(geometry().center());
        QRect screenBounds = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();

        m_behavior.processBehaviorTick(
            pos(), width(), height(), screenBounds,
            m_isWalkModeEnabled, m_moveSpeed, m_physics, m_animationEngine,
            [this](const QPoint& p) { move(p); },
            [this]() { resetInactivityTimer(); }
        );
        });

    m_bounceAnim = new QPropertyAnimation(this, "pos", this);
    m_bounceAnim->setDuration(180);
    m_bounceAnim->setEasingCurve(QEasingCurve::OutQuad);

    QSettings settings;
    applySettings(
        settings.value("pet/scale", 0.5f).toFloat(),
        settings.value("pet/animSpeed", 500).toInt(),
        settings.value("pet/idleTimeout", 15).toInt()
    );

    loadPet(PetLoader::DEFAULT_FOX_ID);
    m_animationEngine->start();
    m_behaviorTimer->start(50);
}

Companion::~Companion() {
    if (m_behaviorTimer) m_behaviorTimer->stop();
    if (m_inactivityTimer) m_inactivityTimer->stop();
    if (m_animationEngine) m_animationEngine->stop();
}

void Companion::applySettings(float scale, int speedMs, int timeoutSec) {
    m_scale = scale;
    m_animSpeed = speedMs;
    if (m_animationEngine) m_animationEngine->setInterval(speedMs);
    if (m_inactivityTimer) m_inactivityTimer->setInterval(timeoutSec * 1000);
    if (!m_currentImagePath.isEmpty()) setSprite(m_currentImagePath);

    QSettings settings;
    settings.setValue("pet/scale", scale);
    settings.setValue("pet/animSpeed", speedMs);
    settings.setValue("pet/idleTimeout", timeoutSec);
    resetInactivityTimer();
}

int Companion::inactivityTimeoutSec() const {
    return m_inactivityTimer ? (m_inactivityTimer->interval() / 1000) : 15;
}

void Companion::loadPet(const QString& petId) {
    m_currentPetData = PetLoader::loadPet(petId);
    if (m_animationEngine) m_animationEngine->setPetData(m_currentPetData);
    QSettings().setValue("pet/selectedId", petId);
}

void Companion::setSprite(const QString& imagePath) {
    if (imagePath.isEmpty()) return;
    QPixmap tempPixmap(imagePath);
    if (tempPixmap.isNull()) return;

    m_currentImagePath = imagePath;
    m_pixmap = (m_animationEngine && m_animationEngine->isFlipped())
        ? tempPixmap.transformed(QTransform().scale(-1, 1)) : tempPixmap;

    int newW = static_cast<int>(m_pixmap.width() * m_scale);
    int newH = static_cast<int>(m_pixmap.height() * m_scale);
    if (newW > 0 && newH > 0) { setFixedSize(newW, newH); update(); }
}

void Companion::resetInactivityTimer() {
    m_inactivityTimer->start();
    if (m_behavior.state() == CompanionState::Sleeping) m_behavior.setState(CompanionState::Idle);
    if (m_animationEngine) {
        auto state = m_isWalkModeEnabled ? AnimationEngine::AnimationState::Walking : AnimationEngine::AnimationState::Idle;
        m_animationEngine->setState(state);
    }
}

void Companion::setWalkModeEnabled(bool enabled) {
    m_isWalkModeEnabled = enabled;
    resetInactivityTimer();
    if (!enabled) m_behavior.setState(CompanionState::Idle);
}

void Companion::triggerBounce() {
    QPoint current = pos();
    m_bounceAnim->stop();
    m_bounceAnim->setKeyValueAt(0.0, current);
    m_bounceAnim->setKeyValueAt(0.5, current - QPoint(0, 8));
    m_bounceAnim->setKeyValueAt(1.0, current);
    m_bounceAnim->start();
}

void Companion::paintEvent(QPaintEvent*) {
    if (m_pixmap.isNull()) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawPixmap(rect(), m_pixmap);
}

void Companion::mousePressEvent(QMouseEvent* event) {
    resetInactivityTimer();
    if (event->button() == Qt::LeftButton) {
        triggerBounce();
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
    else if (event->button() == Qt::RightButton) {
        if (m_activeMenu) { 
            m_activeMenu->close(); 
            m_activeMenu->deleteLater();
			m_activeMenu = nullptr;
        }

        m_activeMenu = CompanionMenuManager::showContextMenu(this, event->globalPosition().toPoint());
        
		if (m_activeMenu) {
			connect(m_activeMenu, &QObject::destroyed, this, [this]() {
				m_activeMenu = nullptr;
		    });
		}
    }
}

void Companion::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        resetInactivityTimer();
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void Companion::clearActiveMenu() {
    m_activeMenu = nullptr;
}