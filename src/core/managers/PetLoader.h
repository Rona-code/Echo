#pragma once

#include <QString>
#include <QStringList>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>

struct PetAnimationData {
	QString id;
	QString displayName;
	QStringList idleFrames;
	QStringList walkFrames;
	QStringList sleepFrames;

	QString getFirstAvailableFrame() const {
		if (!walkFrames.isEmpty()) {
			const QString& frame = walkFrames.first();
			if (!frame.isEmpty()) return frame;
		}
		if (!idleFrames.isEmpty()) {
			const QString& frame = idleFrames.first();
			if (!frame.isEmpty()) return frame;
		}
		if (!sleepFrames.isEmpty()) {
			const QString& frame = sleepFrames.first();
			if (!frame.isEmpty()) return frame;
		}
		return QString();
	}
};

class PetLoader {
public:
	static inline const QString DEFAULT_FOX_ID = "default_fox";

	static PetAnimationData loadPet(const QString& petId) {
		PetAnimationData pet;
		pet.id = petId;

		if (petId == DEFAULT_FOX_ID) {
			pet.displayName = QCoreApplication::translate("PetLoader", "Renard (Défaut)");

			pet.idleFrames = { ":/resources/Renard/renards_Idle0.png", ":/resources/Renard/renards_Idle1.png" };
			pet.walkFrames = { ":/resources/Renard/renards_walk0.png", ":/resources/Renard/renards_walk1.png" };
			pet.sleepFrames = { ":/resources/Renard/renards_sleep0.png", ":/resources/Renard/renards_sleep1.png" };
			return pet;
		}
		
		QString baseAppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir petDir(QDir::cleanPath(baseAppData + "/pets/" + petId));
		QFile jsonFile(petDir.filePath("pet.json"));

		if (jsonFile.open(QIODevice::ReadOnly)) {
			QJsonObject json = QJsonDocument::fromJson(jsonFile.readAll()).object();
			jsonFile.close();

			pet.displayName = json["name"].toString(petId);

			auto readCategory = [&petDir](const QJsonArray& jsonArray) -> QStringList {
				QStringList list;
				for (const auto& item : jsonArray) {
					list.append(petDir.filePath(item.toString()));
				}
				return list;
			};

			pet.idleFrames = readCategory(json["idle"].toArray());
			pet.walkFrames = readCategory(json["walk"].toArray());
			pet.sleepFrames = readCategory(json["sleep"].toArray());
		}

		return pet;
	}
};