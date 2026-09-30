#include "Companion.h"
#include "ActionMenu.h"
#include "../commun/TranslationManager.h"

#include <QPainter>
#include <QApplication>
#include <QDebug>

Companion::Companion(QWidget* parent)
    : QWidget(parent)
{
    m_scale = 0.2f;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);

    setFixedSize(64, 64);

    setPetAnimationData(PetLoader::loadPet(PetLoader::DEFAULT_FOX_ID));
}

Companion::~Companion()
{
    // On libère la pixmap et le menu avant la destruction de l'objet
    m_pixmap = QPixmap();
    if (m_activeMenu) {
        m_activeMenu->deleteLater();
    }
}

void Companion::setPetAnimationData(const PetAnimationData& data) {
    m_currentPetData = data;

    QString firstFrame = m_currentPetData.getFirstAvailableFrame();
    if (!firstFrame.isEmpty()) {
        setSprite(firstFrame);
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

    // Si l'image n'est pas chargée, ON S'ARRÊTE LÀ
    if (tempPixmap.isNull()) {
        qWarning() << "[Companion] ERREUR: Impossible de charger le sprite :" << imagePath;
        return;
    }

    m_currentImagePath = imagePath;
    m_pixmap = tempPixmap;

    int newW = static_cast<int>(m_pixmap.width() * m_scale);
    int newH = static_cast<int>(m_pixmap.height() * m_scale);

    // Sécurité dimensions
    if (newW > 0 && newH > 0) {
        setFixedSize(newW, newH);
        update();
    }
}

void Companion::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Protection absolue contre le crash dans le moteur de rendu Qt
    if (m_pixmap.isNull() || width() <= 0 || height() <= 0) {
        return;
    }

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
    else if (event->button() == Qt::RightButton) 
    {
        if (m_activeMenu) {
            m_activeMenu->close();
            m_activeMenu->deleteLater();
        }

		auto* menu = new ActionMenu(nullptr);
        m_activeMenu = menu;

        connect(menu, &ActionMenu::petChangedRequested, this, [this](const QString& petId) {
            PetAnimationData newPet = PetLoader::loadPet(petId);
            setPetAnimationData(newPet);
        }, Qt::QueuedConnection);

        // Connexion directe au TranslationManager
        connect(menu, &ActionMenu::languageChangedRequested, this, [](const QString& langCode) {
            TranslationManager::instance().setLanguage(langCode);
        });

        // Connexion aux autres actions
        connect(menu, &ActionMenu::launchAppsRequested, this, [this]() {
            // Logic pour lancer la liste des apps
        });

        connect(menu, &ActionMenu::closeAppsRequested, this, [this]() {
            // gic pour fermer les appsLo
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
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}