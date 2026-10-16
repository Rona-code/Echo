#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QMap>

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

    void initConnections();
    void unlock(const QString& id);
    bool isUnlocked(const QString& id) const;
    QList<Achievement> getAllAchievements() const;

signals:
    void achievementUnlocked(const QString& title);

private:
    AchievementManager();
    ~AchievementManager() = default;

    void registerAchievement(const QString& id, const QString& title, const QString& desc);

    QSettings m_settings;
    QMap<QString, Achievement> m_achievements;
};