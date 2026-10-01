#pragma once

#include <QString>

class AppInitializer {
public:
	static bool init();
	static QString getSavedPetId();
};