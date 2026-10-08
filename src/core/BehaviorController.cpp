#include "BehaviorController.h"
#include <QRandomGenerator>

void BehaviorController::chooseNextState(const QRect& screenBounds, int companionWidth) {
    int roll = QRandomGenerator::global()->bounded(100);

    if (roll < 60) {
        m_state = CompanionState::Idle;
        m_stateTicks = QRandomGenerator::global()->bounded(40, 100);
    }
    else {
        m_state = CompanionState::Walking;
        int minX = screenBounds.left();
        int maxX = std::max(minX, screenBounds.right() - companionWidth);
        m_targetX = static_cast<float>(QRandomGenerator::global()->bounded(minX, maxX));
        m_stateTicks = 300;
    }
}