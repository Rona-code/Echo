#include "AppInitializer.h"
#include "security/LockManager.h"
#include "../core/PetLoader.h"

#include <QSettings>

bool AppInitializer::init()
{
	if (!LockManager::acquire()) return false;
	 
	return true;
}

QString AppInitializer::getSavedPetId()
{
	QSettings settings;
	return settings.value("pet/selectedId", PetLoader::DEFAULT_FOX_ID).toString();
}