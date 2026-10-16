#include "AppInitializer.h"
#include "security/LockManager.h"
#include "../core/managers/AchievementManager.h"
#include "../core/managers/PetLoader.h"

#include <QSettings>

bool AppInitializer::init()
{
	if (!LockManager::acquire()) return false;
	 
	AchievementManager::instance().initConnections();

	return true;
}

QString AppInitializer::getSavedPetId()
{
	QSettings settings;
	return settings.value("pet/selectedId", PetLoader::DEFAULT_FOX_ID).toString();
}