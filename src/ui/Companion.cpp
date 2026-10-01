#include "Companion.h"
#include "ActionMenu.h"
#include "../commun/TranslationManager.h"
#include "../core/PetLoader.h"
#include "../core/AnimationEngine.h"

#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QSettings>

Companion::Companion(QWidget* parent)
    : QWidget(parent)
{
    m_scale = 0.2f;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFixedSize(64, 64);

    m_animationEngine = new AnimationEngine(this);
    connect(m_animationEngine, &AnimationEngine::frameChanged, this, &Companion::setSprite);

    m_inactivityTimer = new QTimer(this);
    m_inactivityTimer->setInterval(15000);
    m_inactivityTimer->setSingleShot(true);
    connect(m_inactivityTimer, &QTimer::timeout, this, &Companion::onInactivityTimeout);

    setPetAnimationData(PetLoader::loadPet(PetLoader::DEFAULT_FOX_ID));

	m_animationEngine->start();
	resetInactivityTimer();
}

Companion::~Companion()
{
    m_pixmap = QPixmap();
    if (m_activeMenu) {
        m_activeMenu->deleteLater();
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
    m_pixmap = tempPixmap;

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
    
    if(m_isWalkModeEnabled) {
		m_animationEngine->setState(AnimationEngine::AnimationState::Walking);
	}
	else {
		m_animationEngine->setState(AnimationEngine::AnimationState::Idle);
	}
}

void Companion::setWalkModeEnabled(bool enabled)
{
	m_isWalkModeEnabled = enabled;
	resetInactivityTimer();
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
			// Logic pour ouvrir les paramètres
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