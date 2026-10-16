#include "AchievementManager.h"
#include "../../commun/TranslationManager.h"
#include <QDebug>

AchievementManager::AchievementManager()
{
    // 1. Déclaration de tes trophées
    registerAchievement("first_launch", "Premier pas", "Lancer Echo pour la première fois.");
    registerAchievement("polyglot", "Polyglotte", "Changer la langue de l'application.");
    registerAchievement("traveler", "Grand Voyageur", "Déplacer le compagnon 10 fois.");

    // Succès automatique au lancement (par exemple)
    unlock("first_launch");
}

void AchievementManager::initConnections() {
    // 2. Centralisation des écoutes de signaux (Zéro code de trophée dans l'UI)

    // Exemple : Écoute du changement de langue
    connect(&TranslationManager::instance(), &TranslationManager::languageChanged, this, [this]() {
        unlock("polyglot");
        });

    /*
       Exemple futur pour un compteur (si ton Companion émet un signal de déplacement) :
       connect(companionInstance, &Companion::moved, this, [this]() {
           int current = m_settings.value("stats/move_count", 0).toInt() + 1;
           m_settings.setValue("stats/move_count", current);
           if (current >= 10) {
               unlock("traveler");
           }
       });
    */
}

void AchievementManager::registerAchievement(const QString& id, const QString& title, const QString& desc) {
    bool savedState = m_settings.value("achievements/" + id, false).toBool();
    m_achievements[id] = { id, title, desc, savedState };
}

void AchievementManager::unlock(const QString& id) {
    if (m_achievements.contains(id) && !m_achievements[id].unlocked) {
        m_achievements[id].unlocked = true;
        m_settings.setValue("achievements/" + id, true);

        emit achievementUnlocked(m_achievements[id].title);
        qDebug() << "🏆 Trophée débloqué :" << m_achievements[id].title;
    }
}

bool AchievementManager::isUnlocked(const QString& id) const {
    return m_achievements.value(id).unlocked;
}

QList<Achievement> AchievementManager::getAllAchievements() const {
    return m_achievements.values();
}