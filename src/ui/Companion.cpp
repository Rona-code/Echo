#include "Companion.h"
#include "ActionMenu.h"
#include "../commun/TranslationManager.h"
#include <QPainter>

Companion::Companion(QWidget* parent)
    : QWidget(parent)
{
    //m_scale = 0.2f;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);

    setSprite("resources/Renard/renards_Idle0.png");
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
    else if (event->button() == Qt::RightButton) 
    {
		auto* menu = new ActionMenu(this);

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

		connect(menu, &ActionMenu::quitRequested, this, [this]() {
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