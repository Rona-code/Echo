#include "AppInitializer.h"
#include "security/LockManager.h"

bool AppInitializer::init()
{
	if (!LockManager::acquire()) return false;
	
	return true;
}