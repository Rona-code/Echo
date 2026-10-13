#include "CompanionMenuManager.h"
#include "../ui/Companion.h"
#include "../ui/ActionMenu.h"
#include "../ui/SettingsDialog.h"
#include "../ui/ExplorerWindow.h"
#include "../commun/TranslationManager.h"

#include <QApplication>
#include <QSettings>

ActionMenu* CompanionMenuManager::showContextMenu(Companion* companion, const QPoint& globalPos)
{
    if (!companion) return nullptr;

    auto* menu = new ActionMenu(companion->isWalkModeEnabled(), companion);

	menu->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(menu, &QObject::destroyed, companion, [companion]() {
        companion->clearActiveMenu();
    });

	PetAnimationData petData = companion->currentPetData();
	QString currentFrame = petData.getFirstAvailableFrame();

    if (!currentFrame.isEmpty()) {
        menu->setPetIcon(currentFrame);
    }

    QObject::connect(menu, &ActionMenu::walkModeToggled, companion, &Companion::setWalkModeEnabled);
    QObject::connect(menu, &ActionMenu::petChangedRequested, companion, &Companion::loadPet, Qt::QueuedConnection);
    QObject::connect(menu, &ActionMenu::languageChangedRequested, [](const QString& langCode) {
        TranslationManager::instance().setLanguage(langCode);
    });

    QObject::connect(menu, &ActionMenu::launchAppsRequested, companion, []() {
        // Logique pour lancer la liste des apps
    });

    QObject::connect(menu, &ActionMenu::closeAppsRequested, companion, []() {
        // Logique pour fermer les apps
    });

    QObject::connect(menu, &ActionMenu::openExplorerRequested, companion, []() {
		auto* explorer = new ExplorerWindow(nullptr);
		explorer->setAttribute(Qt::WA_DeleteOnClose);

		QScreen* screen = QGuiApplication::screenAt(explorer->geometry().center());
		QRect screenGeometry = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
		explorer->move(screenGeometry.center() - explorer->rect().center());

		explorer->setAttribute(Qt::WA_DeleteOnClose);
		explorer->show();
    });

    QObject::connect(menu, &ActionMenu::settingsRequested, companion, [companion]() {
        auto* dialog = new SettingsDialog(nullptr);
        dialog->setValues(companion->scale(), companion->animSpeed(), companion->inactivityTimeoutSec());

		dialog->adjustSize();
		QScreen* screen = QGuiApplication::screenAt(companion->geometry().center());
		QRect screenGeometry = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
		dialog->move(screenGeometry.center() - dialog->rect().center());

        QObject::connect(dialog, &SettingsDialog::settingsChanged, companion, [companion](double scale, int speed, int timeout) {
            companion->applySettings(static_cast<float>(scale), speed, timeout);
        });

        dialog->exec();
    });

    QObject::connect(menu, &ActionMenu::quitRequested, []() {
        qApp->quit();
    });

	menu->popupAt(globalPos, companion->geometry());
    return menu;
}