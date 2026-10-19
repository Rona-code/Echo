#include "AchievementManager.h"
#include "../../commun/TranslationManager.h"
#include <QCoreApplication>
#include <QTimer>
#include <QTime>
#include <QDebug>

AchievementManager::AchievementManager()
{
    initAchievements();
    unlock("first_launch");

    if (!isUnlocked("night_owl")) {
        checkNightOwlTime();
        if (!isUnlocked("night_owl")) {
            m_nightOwlTimer = new QTimer(this);
            connect(m_nightOwlTimer, &QTimer::timeout, this, &AchievementManager::checkNightOwlTime);
            m_nightOwlTimer->start(60000);
        }
    }

    QTimer* playTimer = new QTimer(this);
    connect(playTimer, &QTimer::timeout, this, [this]() {
        addPlayTimeMinutes(1);
        });
    playTimer->start(60000);
}

void AchievementManager::initAchievements() {
    registerAchievement("first_launch",
        QCoreApplication::translate("AchievementManager", "Premier pas"),
        QCoreApplication::translate("AchievementManager", "Là où tout commence... Éveillez votre compagnon."));

    registerAchievement("polyglot",
        QCoreApplication::translate("AchievementManager", "Polyglotte"),
        QCoreApplication::translate("AchievementManager", "Parlez une langue que les murs ne comprennent pas."));

    registerAchievement("speed_demon",
        QCoreApplication::translate("AchievementManager", "Démon de la vitesse"),
        QCoreApplication::translate("AchievementManager", "Frôlez la vitesse de la lumière (ou du moins, tombez très vite)"));
   
    registerAchievement("night_owl",
        QCoreApplication::translate("AchievementManager", "Oiseau de nuit"),
        QCoreApplication::translate("AchievementManager", "Actif quand le monde entier est plongé dans le noir."));

    registerAchievement("traveler",
        QCoreApplication::translate("AchievementManager", "Grand Voyageur"),
        QCoreApplication::translate("AchievementManager", "Arpentez l'écran de long en large, encore et encore."));

    registerAchievement("loyal_companion",
        QCoreApplication::translate("AchievementManager", "Fidèle"),
        QCoreApplication::translate("AchievementManager", "Le temps s'efface quand on est bien accompagné."));

    registerAchievement("new_companion",
        QCoreApplication::translate("AchievementManager", "Nouvelle recrue"),
        QCoreApplication::translate("AchievementManager", "Accueillez une âme supplémentaire sous votre toit."));

    registerAchievement("companion_switcher",
        QCoreApplication::translate("AchievementManager", "Métamorphe"),
        QCoreApplication::translate("AchievementManager", "Changez de visage comme de chemise."));

    registerAchievement("click_addict",
        QCoreApplication::translate("AchievementManager", "Chatouilleux"),
        QCoreApplication::translate("AchievementManager", "À force de le pousser à bout, il finira par réagir."));
    
    registerAchievement("scale_min",
        QCoreApplication::translate("AchievementManager", "Minuscule"),
        QCoreApplication::translate("AchievementManager", "Réduisez votre compagnon à sa plus simple expression."));

    registerAchievement("scale_max",
        QCoreApplication::translate("AchievementManager", "Titan"),
        QCoreApplication::translate("AchievementManager", "Donnez à votre compagnon des proportions gigantesques."));

    registerAchievement("anim_speed_min",
        QCoreApplication::translate("AchievementManager", "Hyperactif"),
        QCoreApplication::translate("AchievementManager", "Poussez la vitesse d'animation au maximum de sa folie."));

    registerAchievement("anim_speed_max",
        QCoreApplication::translate("AchievementManager", "Au ralenti"),
        QCoreApplication::translate("AchievementManager", "Profitez de chaque image dans une lenteur absolue."));

    registerAchievement("timeout_min",
        QCoreApplication::translate("AchievementManager", "Marmotte"),
        QCoreApplication::translate("AchievementManager", "Réglez le délai d'inactivité au plus court."));

    registerAchievement("timeout_max",
        QCoreApplication::translate("AchievementManager", "Insomniaque"),
        QCoreApplication::translate("AchievementManager", "Laissez-lui tout le temps du monde avant de piquer un somme."));

    registerAchievement("calendar_event",
        QCoreApplication::translate("AchievementManager", "Organisé"),
        QCoreApplication::translate("AchievementManager", "Planifiez votre avenir en ajoutant un événement au calendrier."));

    registerAchievement("konami_master",
        QCoreApplication::translate("AchievementManager", "???"),
        QCoreApplication::translate("AchievementManager", "Vous avez percé le secret des développeurs !"));
}

//--- Logique d'unlock ---
void AchievementManager::registerAchievement(const QString& id, const QString& title, const QString& desc) {
    bool savedState = m_settings.value("achievements/" + id, false).toBool();
    m_achievements[id] = { id, title, desc, savedState };
}

void AchievementManager::unlock(const QString& id) {
    if (m_achievements.contains(id) && !m_achievements[id].unlocked) {
        m_achievements[id].unlocked = true;
        m_settings.setValue("achievements/" + id, true);

        emit achievementUnlocked(m_achievements[id].title);
    }
}

bool AchievementManager::isUnlocked(const QString& id) const {
    return m_achievements.value(id).unlocked;
}

QList<Achievement> AchievementManager::getAllAchievements() const {
    return m_achievements.values();
}
//--- End Logique d'unlock ---

//--- Scoring ---
void AchievementManager::checkAndSaveMaxSpeed(float speed) {
    float currentMax = getMaxSpeedRecord();
    if (speed > currentMax) {
        m_settings.setValue("stats/max_fall_speed", speed);

        if (speed >= 100.0f) {
            unlock("speed_demon");
        }
    }
}

float AchievementManager::getMaxSpeedRecord() const {
    return m_settings.value("stats/max_fall_speed", 0.0f).toFloat();
}

void AchievementManager::resetAll() {
    m_settings.remove("achievements");
    m_settings.remove("stats/max_fall_speed");

    m_settings.remove("stats/play_time_minutes");
    m_settings.remove("stats/companion_changes");
    m_settings.remove("stats/move_count");
    m_settings.remove("stats/click_count");

    m_achievements.clear();
    
    initAchievements();
}
//--- End Scoring ---

//--- Condition d'unlock & Notif ---
void AchievementManager::initConnections() {
    connect(&TranslationManager::instance(), &TranslationManager::languageChanged, this, [this]() {
        unlock("polyglot");
        });
}

void AchievementManager::checkNightOwlTime() {
    if (isUnlocked("night_owl")) return;

    QTime currentTime = QTime::currentTime();
    if (currentTime.hour() >= 0 && currentTime.hour() < 5) {
        unlock("night_owl");

        if (m_nightOwlTimer) {
            m_nightOwlTimer->stop();
        }
    }
}

void AchievementManager::addPlayTimeMinutes(int minutes) {
    int totalMinutes = m_settings.value("stats/play_time_minutes", 0).toInt() + minutes;
    m_settings.setValue("stats/play_time_minutes", totalMinutes);

    if (totalMinutes >= 60) {
        unlock("loyal_companion");
    }
}

void AchievementManager::notifyCompanionAdded() {
    unlock("new_companion");
}

void AchievementManager::notifyCompanionChanged() {
    int count = m_settings.value("stats/companion_changes", 0).toInt() + 1;
    m_settings.setValue("stats/companion_changes", count);

    if (count >= 5) {
        unlock("companion_switcher");
    }
}

void AchievementManager::notifyCompanionMoved() {
    int count = m_settings.value("stats/move_count", 0).toInt() + 1;
    m_settings.setValue("stats/move_count", count);

    if (count >= 100000) {
        unlock("traveler");
    }
}

void AchievementManager::notifyCompanionClicked() {
    int count = m_settings.value("stats/click_count", 0).toInt() + 1;
    m_settings.setValue("stats/click_count", count);

    if (count >= 100) {
        unlock("click_addict");
    }
}

void AchievementManager::notifyScaleChanged(float scale) {
    // Ajuste les valeurs selon ton échelle min/max réelle (ex: 0.2f et 2.0f)
    if (scale <= 0.05f) {
        unlock("scale_min");
    }
    else if (scale >= 3.0f) {
        unlock("scale_max");
    }
}

void AchievementManager::notifyAnimSpeedChanged(int speedMs) {
    // Vitesse max en ms (plus le chiffre est bas, plus c'est rapide) ou inversement selon ton UI
    if (speedMs <= 100) {
        unlock("anim_speed_min");
    }
    else if (speedMs >= 1000) {
        unlock("anim_speed_max");
    }
}

void AchievementManager::notifyTimeoutChanged(int timeoutSec) {
    if (timeoutSec <= 5) {
        unlock("timeout_min");
    }
    else if (timeoutSec >= 600) {
        unlock("timeout_max");
    }
}

void AchievementManager::notifyCalendarEventAdded() {
    unlock("calendar_event");
}
//--- End Condition d'unlock ---