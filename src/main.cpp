#include <QCoreApplication>
#include <QApplication>
#include <QIcon>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

#include "commun/AppInitializer.h"
#include "commun/TranslationManager.h"
#include "ui/windows/Companion.h"
#include "core/managers/PetLoader.h"

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("Stusoft");
    QCoreApplication::setApplicationName("Echo");

    QApplication a(argc, argv);

	a.setQuitOnLastWindowClosed(false);

    // Charge la langue de base
    TranslationManager::instance().loadSavedLanguage();

    if (!AppInitializer::init())
        return 0;

	auto* w = new Companion();
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->loadPet(AppInitializer::getSavedPetId());
    w->show();

	return a.exec();
}