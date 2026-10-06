#include "Companion.h"
#include "ActionMenu.h"
#include "../commun/TranslationManager.h"
#include "../core/PetLoader.h"
#include "../core/AnimationEngine.h"
#include "SettingsDialog.h"

#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QRandomGenerator>
//#include <QGraphicsDropShadowEffect>

Companion::Companion(QWidget* parent)
	: QWidget(parent)
{
	QSettings settings;
	m_scale = settings.value("pet/scale", 0.5f).toFloat();
	m_animSpeed = settings.value("pet/animSpeed", 500).toInt();
	int idleTimeoutSec = settings.value("pet/idleTimeout", 15).toInt();

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_DeleteOnClose, false);
	setFixedSize(64, 64);

	m_animationEngine = new AnimationEngine(this);
	m_animationEngine->setInterval(m_animSpeed);
	connect(m_animationEngine, &AnimationEngine::frameChanged, this, &Companion::setSprite);

	m_inactivityTimer = new QTimer(this);
	m_inactivityTimer->setInterval(idleTimeoutSec * 1000);
	m_inactivityTimer->setSingleShot(true);
	connect(m_inactivityTimer, &QTimer::timeout, this, &Companion::onInactivityTimeout);

	setPetAnimationData(PetLoader::loadPet(PetLoader::DEFAULT_FOX_ID));

	m_bounceAnim = new QPropertyAnimation(this, "pos", this);
	m_bounceAnim->setDuration(180);
	m_bounceAnim->setEasingCurve(QEasingCurve::OutQuad);

	m_animationEngine->start();
	resetInactivityTimer();

	setupBehavior();
}

Companion::~Companion()
{
	if (m_behaviorTimer) {
		m_behaviorTimer->stop();
	}
	if (m_inactivityTimer) {
		m_inactivityTimer->stop();
	}
	if (m_animationEngine) {
		m_animationEngine->stop();
	}
}

void Companion::loadPet(const QString& petId)
{
	PetAnimationData data = PetLoader::loadPet(petId);
	m_currentPetData = data;

	if (m_animationEngine) {
		m_animationEngine->setPetData(data);
	}

	QSettings settings;
	settings.setValue("pet/selectedId", petId);
}

void Companion::setPetAnimationData(const PetAnimationData& data)
{
	m_currentPetData = data;
	if (m_animationEngine) {
		m_animationEngine->setPetData(data);
	}
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
	if (imagePath.isEmpty()) return;

	QPixmap tempPixmap(imagePath);

	if (tempPixmap.isNull()) {
		qWarning() << "[Companion] ERREUR: Impossible de charger le sprite :" << imagePath;
		return;
	}

	m_currentImagePath = imagePath;
	if (m_animationEngine && m_animationEngine->isFlipped()) {
		m_pixmap = tempPixmap.transformed(QTransform().scale(-1, 1));
	}
	else {
		m_pixmap = tempPixmap;
	}

	int newW = static_cast<int>(m_pixmap.width() * m_scale);
	int newH = static_cast<int>(m_pixmap.height() * m_scale);

	if (newW > 0 && newH > 0) {
		setFixedSize(newW, newH);
		update();
	}
}

// --- LOGIQUE DES ÉTATS ET TIMERS ---

void Companion::resetInactivityTimer()
{
	m_inactivityTimer->start();
	updateAnimationState();
}

void Companion::onInactivityTimeout()
{
	if (m_animationEngine) {
		m_animationEngine->setState(AnimationEngine::AnimationState::Sleeping);
	}
}

void Companion::updateAnimationState()
{
	if (!m_animationEngine) return;

	if (m_isWalkModeEnabled) {
		m_animationEngine->setState(AnimationEngine::AnimationState::Walking);
	}
	else {
		m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
	}
}

void Companion::setAnimationSpeed(int intervalMs)
{
	if (intervalMs <= 0 || !m_animationEngine) return;
	m_animSpeed = intervalMs;
	m_animationEngine->setInterval(intervalMs);
}

void Companion::setInactivityTimeout(int seconds)
{
	if (seconds <= 0 || !m_inactivityTimer) return;
	m_inactivityTimer->setInterval(seconds * 1000);
	resetInactivityTimer();
}

void Companion::setWalkModeEnabled(bool enabled)
{
	m_isWalkModeEnabled = enabled;
	resetInactivityTimer();

	if (!m_isWalkModeEnabled) {
		m_state = CompanionState::Idle;
		if (m_animationEngine) {
			m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
		}
	}
}

// --- ÉVÉNEMENTS UI ---

void Companion::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	if (m_pixmap.isNull() || width() <= 0 || height() <= 0) {
		return;
	}

	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	painter.drawPixmap(rect(), m_pixmap);
}

void Companion::mousePressEvent(QMouseEvent* event)
{
	resetInactivityTimer();

	if (event->button() == Qt::LeftButton)
	{
		QPoint current = pos();
		m_bounceAnim->stop();
		m_bounceAnim->setKeyValueAt(0.0, current);
		m_bounceAnim->setKeyValueAt(0.5, current - QPoint(0, 8));
		m_bounceAnim->setKeyValueAt(1.0, current);
		m_bounceAnim->start();

		m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
		event->accept();
	}
	else if (event->button() == Qt::RightButton)
	{
		if (m_activeMenu) {
			m_activeMenu->close();
			m_activeMenu->deleteLater();
		}

		auto* menu = new ActionMenu(isWalkModeEnabled(), nullptr);
		m_activeMenu = menu;

		QString currentFrame = m_currentPetData.getFirstAvailableFrame();
		if (!currentFrame.isEmpty()) {
			menu->setPetIcon(currentFrame);
		}

		connect(menu, &ActionMenu::walkModeToggled, this, &Companion::setWalkModeEnabled);

		connect(menu, &ActionMenu::petChangedRequested, this, &Companion::loadPet, Qt::QueuedConnection);

		connect(menu, &ActionMenu::languageChangedRequested, this, [](const QString& langCode) {
			TranslationManager::instance().setLanguage(langCode);
			});

		// Connexion aux autres actions
		connect(menu, &ActionMenu::launchAppsRequested, this, [this]() {
			// Logic pour lancer la liste des apps
			});

		connect(menu, &ActionMenu::closeAppsRequested, this, [this]() {
			// Logic pour fermer les apps
			});

		connect(menu, &ActionMenu::openExplorerRequested, this, [this]() {
			// Logic pour ouvrir l'explorateur de fichiers
			});

		connect(menu, &ActionMenu::settingsRequested, this, [this]() {
			auto* dialog = new SettingsDialog(this);

			// 1. Récupération des valeurs actuelles
			int currentTimeoutSec = m_inactivityTimer ? (m_inactivityTimer->interval() / 1000) : 15;
			int currentAnimSpeed = m_animSpeed;

			dialog->setValues(m_scale, currentAnimSpeed, currentTimeoutSec);

			// 2. Application des réglages
			connect(dialog, &SettingsDialog::settingsChanged, this, [this](double scale, int speed, int timeout) {
				setScale(static_cast<float>(scale));
				setAnimationSpeed(speed);
				setInactivityTimeout(timeout);

				QSettings settings;
				settings.setValue("pet/scale", scale);
				settings.setValue("pet/animSpeed", speed);
				settings.setValue("pet/idleTimeout", timeout);
				});

			dialog->exec();
			});

		connect(menu, &ActionMenu::quitRequested, this, []() {
			qApp->quit();
			});

		menu->move(event->globalPosition().toPoint());
		menu->show();
	}
}

void Companion::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		resetInactivityTimer();
		move(event->globalPosition().toPoint() - m_dragPosition);
		event->accept();
	}
}

// --- ANIMATIONS ---

void Companion::setupBehavior() {
	m_behaviorTimer = new QTimer(this);
	connect(m_behaviorTimer, &QTimer::timeout, this, &Companion::updateBehavior);
	m_behaviorTimer->start(50); // Loop de déplacement à ~20 FPS
}

QRect Companion::getScreenBounds() const {
	QScreen* screen = QGuiApplication::screenAt(geometry().center());
	if (!screen) screen = QGuiApplication::primaryScreen();
	return screen->availableGeometry();
}

void Companion::chooseNextState() {
	int roll = QRandomGenerator::global()->bounded(100);

	if (roll < 60) {
		// 60% de chance de rester Idle
		m_state = CompanionState::Idle;
		m_stateTicks = QRandomGenerator::global()->bounded(40, 100); // ~2s à 5s

		if (m_animationEngine) {
			m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
		}
	}
	else {
		// 40% de chance de marcher
		m_state = CompanionState::Walking;
		QRect screenRect = getScreenBounds();

		// Cible un point X aléatoire sur le bas de l'écran (mode Taskbar / Desktop)
		int targetX = QRandomGenerator::global()->bounded(screenRect.left(), screenRect.right() - width());
		m_targetPos = QPointF(targetX, y());
		m_stateTicks = 300;

		if (m_animationEngine) {
			m_animationEngine->setState(AnimationEngine::AnimationState::Walking);
		}
	}
}

void Companion::updateBehavior() {
	if (!m_isWalkModeEnabled) {
		if (m_state == CompanionState::Walking) {
			m_state = CompanionState::Idle;
			if (m_animationEngine) {
				m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
			}
		}
		return;
	}

	if (m_state == CompanionState::Sleeping) return;

	if (m_state == CompanionState::Walking) {
		updateMovement();
	}
	else {
		m_stateTicks--;
		if (m_stateTicks <= 0) {
			chooseNextState();
		}
	}
}

void Companion::updateMovement() {
	QPointF currentPos = pos();
	float deltaX = m_targetPos.x() - currentPos.x();

	if (std::abs(deltaX) <= m_moveSpeed || m_stateTicks <= 0) {
		m_state = CompanionState::Idle;
		m_stateTicks = QRandomGenerator::global()->bounded(40, 80);

		if (m_animationEngine) {
			m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
		}
		return;
	}

	float direction = (deltaX > 0) ? 1.0f : -1.0f;

	if (m_animationEngine) {
		m_animationEngine->setFlipped(direction < 0);
	}

	move(static_cast<int>(currentPos.x() + direction * m_moveSpeed), currentPos.y());

	m_stateTicks--;
}