#include "CompanionMenuManager.h"
#include "Companion.h"
#include "ActionMenu.h"
#include "ExplorerWindow.h"
#include "CalendarWindow.h"
#include "../dialogs/SettingsDialog.h"
#include "../../commun/TranslationManager.h"

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

        QSettings settings;
        QString rappelTime = settings.value("explorer/rappelTime", "08:00").toString();
        dialog->setValues(companion->scale(), companion->animSpeed(), companion->inactivityTimeoutSec(), rappelTime);

        // Connexion pour ouvrir le calendrier depuis les paramètres
        QObject::connect(dialog, &SettingsDialog::openCalendarRequested, companion, [=]() {
            if (dialog) {
                dialog->close();
            }

            auto* calendarWin = new CalendarWindow(nullptr);
            calendarWin->setAttribute(Qt::WA_DeleteOnClose);

            QScreen* screen = QGuiApplication::screenAt(calendarWin->geometry().center());
            QRect screenGeometry = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
            calendarWin->move(screenGeometry.center() - calendarWin->rect().center());

            calendarWin->show();
            });

        dialog->adjustSize();
        QScreen* screen = QGuiApplication::screenAt(companion->geometry().center());
        QRect screenGeometry = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
        dialog->move(screenGeometry.center() - dialog->rect().center());

        QObject::connect(dialog, &SettingsDialog::settingsChanged, companion, [companion](double scale, int speed, int timeout, const QString& newRappelTime) {
            companion->applySettings(static_cast<float>(scale), speed, timeout);

            QSettings s;
            s.setValue("explorer/rappelTime", newRappelTime);
            });

        dialog->exec();
    });

    QObject::connect(menu, &ActionMenu::quitRequested, []() {
        qApp->quit();
    });

	menu->popupAt(globalPos, companion->geometry());
    return menu;
}