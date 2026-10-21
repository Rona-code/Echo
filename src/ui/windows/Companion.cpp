#include "Companion.h"
#include "ActionMenu.h"
#include "CompanionMenuManager.h"
#include "ConfettiOverlay.h"
#include "../../core/engine/AnimationEngine.h"
#include "../../core/physics/WindowDetector.h"
#include "../../core/managers/ReminderManager.h"
#include "../../core/managers/AchievementManager.h"

#include <QPainter>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QKeyEvent>
#include <algorithm>

Companion::Companion(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(64, 64);
    setFocusPolicy(Qt::StrongFocus);

    m_animationEngine = new AnimationEngine(this);
    connect(m_animationEngine, &AnimationEngine::frameChanged, this, &Companion::setSprite);

    m_inactivityTimer = new QTimer(this);
    m_inactivityTimer->setSingleShot(true);
    connect(m_inactivityTimer, &QTimer::timeout, this, [this]() {

        if (m_isWalkModeEnabled && m_behavior.state() == CompanionState::Walking) {
            m_inactivityTimer->start();
            return;
        }

        m_behavior.setState(CompanionState::Sleeping);
        m_behavior.setTicks(QRandomGenerator::global()->bounded(200, 600));
        if (m_animationEngine) m_animationEngine->setState(AnimationEngine::AnimationState::Sleeping);
        });

    m_behaviorTimer = new QTimer(this);
    connect(m_behaviorTimer, &QTimer::timeout, this, [this]() {
        QScreen* screen = QGuiApplication::screenAt(geometry().center());
        QRect screenBounds = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();

		QList<QRect> windows = WindowDetector::getAllVisibleWindowGeometries();
		m_physics.updatePhysics(this, windows, screenBounds);

        if (!m_isDragging) {
            m_behavior.processBehaviorTick(
                pos(), width(), height(), screenBounds,
                m_isWalkModeEnabled, m_moveSpeed, m_physics, m_animationEngine,
                [&](const QPoint& p) {

                    int clampedX = std::clamp(p.x(), screenBounds.left(), screenBounds.right() - width());
                    int clampedY = std::clamp(p.y(), screenBounds.top(), screenBounds.bottom() - height());
                    QPoint clampedPoint(clampedX, clampedY);

                    if (pos() != clampedPoint) {
                        move(clampedPoint);
                        AchievementManager::instance().notifyCompanionMoved();
                    }
                },
                [this]() { resetInactivityTimer(); }
            );
        }
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

    auto* reminderManager = new ReminderManager(this);
    connect(reminderManager, &ReminderManager::reminderTriggered, this, [this](const QString& eventText) {
        triggerBounce(); // Animation de saut
        QString message = QString(tr("Hé ! Petit rappel pour aujourd'hui :\n\n\"%1\"\n\nNe l'oublie pas !")).arg(eventText);
        QMessageBox::information(nullptr, tr("Alerte de ton Compagnon"), message);
        });
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
    
    if (width() != newW || height() != newH) {
        setFixedSize(newW, newH);
    }
    update();
}

void Companion::resetInactivityTimer() {
    m_inactivityTimer->start();
    if (m_behavior.state() == CompanionState::Sleeping) {
        m_behavior.setState(CompanionState::Idle);
        m_behavior.setTicks(0);
    }
    if (m_animationEngine) {
        auto state = (m_behavior.state() == CompanionState::Walking)
            ? AnimationEngine::AnimationState::Walking
            : AnimationEngine::AnimationState::Idle;
        m_animationEngine->setState(state);
    }
}

void Companion::setWalkModeEnabled(bool enabled) {
    m_isWalkModeEnabled = enabled;
    if (!enabled) m_behavior.setState(CompanionState::Idle);
    m_behavior.setTicks(0);
    resetInactivityTimer();
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

    activateWindow();
    setFocus();

    if (event->button() == Qt::LeftButton) {
        //triggerBounce();
        m_isDragging = true;
        m_bounceAnim->stop();
        AchievementManager::instance().notifyCompanionClicked();
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

void Companion::mouseReleaseEvent(QMouseEvent* event) {
    triggerBounce();
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void Companion::setGrounded(bool grounded) {
    m_grounded = grounded;
}

bool Companion::isGrounded() const {
    return m_grounded;
}

bool Companion::isDragging() const {
    return m_isDragging;
}

void Companion::clearActiveMenu() {
    m_activeMenu = nullptr;
}

void Companion::keyPressEvent(QKeyEvent* event) {
    static const QList<int> konamiCode = {
        Qt::Key_Up, Qt::Key_Up,
        Qt::Key_Down, Qt::Key_Down,
        Qt::Key_Left, Qt::Key_Right,
        Qt::Key_Left, Qt::Key_Right,
        Qt::Key_B, Qt::Key_A
    };

    static int currentIndex = 0;

    if (event->key() == konamiCode[currentIndex]) {
        currentIndex++;
        if (currentIndex == konamiCode.size()) {
            // --- SUCCÈS DÉBLOQUÉ & PLUIE DE CONFETTIS ---
            AchievementManager::instance().unlock("konami_master");
            new ConfettiOverlay();
            triggerBounce();

            currentIndex = 0;
        }
    }
    else {
        currentIndex = (event->key() == konamiCode[0]) ? 1 : 0;
    }

    QWidget::keyPressEvent(event);
}