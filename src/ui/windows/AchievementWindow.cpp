#include "AchievementWindow.h"
#include "../../core/managers/AchievementManager.h"
#include <QLabel>
#include <QHBoxLayout>

AchievementsWindow::AchievementsWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle(tr("Trophées d'Echo"));
    resize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel(tr("🏆 Succès et Trophées"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(
        "QListWidget { background-color: #2b2b2b; color: #ffffff; border-radius: 8px; padding: 5px; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #3d3d3d; }"
    );
    layout->addWidget(m_listWidget);

    loadAchievements();
}

void AchievementsWindow::loadAchievements() {
    m_listWidget->clear();
    auto achievements = AchievementManager::instance().getAllAchievements();

    for (const auto& ach : achievements) {
        QString status = ach.unlocked ? "✅ [Débloqué]" : "🔒 [Verrouillé]";
        QString text = QString("%1  %2\n   %3").arg(status, ach.title, ach.description);

        QListWidgetItem* item = new QListWidgetItem(text, m_listWidget);
        if (!ach.unlocked) {
            item->setForeground(QColor("#888888")); // Griser les trophées verrouillés
        }
        m_listWidget->addItem(item);
    }
}