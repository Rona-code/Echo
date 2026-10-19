#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QMap>
#include <QTimer>

struct Achievement {
    QString id;
    QString title;
    QString description;
    bool unlocked;
};

class AchievementManager : public QObject {
    Q_OBJECT
public:
    static AchievementManager& instance() {
        static AchievementManager inst;
        return inst;
    }

    AchievementManager(const AchievementManager&) = delete;
    AchievementManager& operator=(const AchievementManager&) = delete;

    void initAchievements();
    void initConnections();
    void unlock(const QString& id);
    void checkAndSaveMaxSpeed(float speed);
    void resetAll();

    void addPlayTimeMinutes(int minutes);
    void notifyCompanionAdded();
    void notifyCompanionChanged();
    void notifyCompanionMoved();
    void notifyCompanionClicked();
    void notifyScaleChanged(float scale);
    void notifyAnimSpeedChanged(int speedMs);
    void notifyTimeoutChanged(int timeoutSec);
    void notifyCalendarEventAdded();
    
    bool isUnlocked(const QString& id) const;
    float getMaxSpeedRecord() const;
    QList<Achievement> getAllAchievements() const;

signals:
    void achievementUnlocked(const QString& title);

private:
    AchievementManager();
    ~AchievementManager() = default;

    void registerAchievement(const QString& id, const QString& title, const QString& desc);
    void checkNightOwlTime();

    QSettings m_settings;
    QMap<QString, Achievement> m_achievements;
    QTimer* m_nightOwlTimer = nullptr;
};