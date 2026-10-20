#include "AchievementWindow.h"
#include "../../core/managers/AchievementManager.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>

AchievementsWindow::AchievementsWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle(tr("Trophées d'Echo"));
    resize(400, 350);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel(tr("🏆 Succès et Trophées"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    float maxSpeed = AchievementManager::instance().getMaxSpeedRecord();
    float speedKmH = maxSpeed * 3.6f;

    QLabel* speedRecordLabel = new QLabel(QString(tr("Record de vitesse de chute : %1 Km/h")).arg(speedKmH, 0, 'f', 1), this);
    speedRecordLabel->setStyleSheet("font-size: 12px; color: #a0a0a0; margin-bottom: 10px;");
    layout->addWidget(speedRecordLabel);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(
        "QListWidget { background-color: #2b2b2b; color: #ffffff; border-radius: 8px; padding: 5px; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #3d3d3d; }"
    );
    layout->addWidget(m_listWidget);

    QPushButton* resetButton = new QPushButton(tr("Réinitialiser les données"), this);
    resetButton->setStyleSheet(
        "QPushButton { background-color: #d9534f; color: white; border-radius: 6px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #c9302c; }"
    );
    connect(resetButton, &QPushButton::clicked, this, [this]() {
        int reply = QMessageBox::question(this, tr("Confirmation"),
            tr("Voulez-vous vraiment réinitialiser tous vos trophées et records ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            AchievementManager::instance().resetAll();
            loadAchievements();
        }
        });

    layout->addWidget(resetButton);

    loadAchievements();
}

void AchievementsWindow::loadAchievements() {
    m_listWidget->clear();
    auto achievements = AchievementManager::instance().getAllAchievements();

    QIcon lockedIcon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
    QIcon unlockedIcon = style()->standardIcon(QStyle::SP_DialogApplyButton);

    for (const auto& ach : achievements) {
        QString status = ach.unlocked ? tr("[Débloqué]") : tr("[Verrouillé]");
        QString text = QString("%1  %2\n   %3").arg(status, ach.title, ach.description);

        QListWidgetItem* item = new QListWidgetItem(text, m_listWidget);

        item->setIcon(ach.unlocked ? unlockedIcon : lockedIcon);

        if (!ach.unlocked) {
            item->setForeground(QColor("#888888"));
        }
        m_listWidget->addItem(item);
    }
}