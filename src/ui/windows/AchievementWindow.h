#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>

class AchievementsWindow : public QWidget {
    Q_OBJECT
public:
    AchievementsWindow(QWidget* parent = nullptr);

private:
    void loadAchievements();
    QListWidget* m_listWidget;
};