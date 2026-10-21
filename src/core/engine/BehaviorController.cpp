#include "BehaviorController.h"
#include "AnimationEngine.h"
#include "../physics/PhysicsController.h"
#include <QRandomGenerator>
#include <cstdlib>
#include <algorithm>

void BehaviorController::chooseNextState(const QRect& screenBounds, int companionWidth, int currentX, float moveSpeed, AnimationEngine* animEngine) {
    int roll = QRandomGenerator::global()->bounded(100);

    if (roll < 60) {
        m_state = CompanionState::Idle;
        m_stateTicks = QRandomGenerator::global()->bounded(20, 100);
    }
    else {
        m_state = CompanionState::Walking;

        const int minX = screenBounds.left();
        const int maxX = std::max(minX + 1, screenBounds.right() - companionWidth);
        const int minDistance = 250;

        int target = currentX;
        for (int i = 0; i < 10 && std::abs(target - currentX) < minDistance; ++i) {
            target = QRandomGenerator::global()->bounded(minX, maxX);
        }

        if (std::abs(target - currentX) < minDistance) {
            target = (currentX - minX > maxX - currentX) ? minX : maxX;
        }

        m_targetX = static_cast<float>(target);
        m_stateTicks = static_cast<int>(std::abs(target - currentX) / std::max(moveSpeed, 0.1f)) + 40;
    }

    if (animEngine) {
        auto engineState = (m_state == CompanionState::Walking)
            ? AnimationEngine::AnimationState::Walking
            : AnimationEngine::AnimationState::Idle;
        animEngine->setState(engineState);
    }
}

void BehaviorController::processBehaviorTick(
    QPoint currentPos,
    int width, int height,
    const QRect& screenBounds,
    bool isWalkModeEnabled,
    float moveSpeed,
    PhysicsController& physics,
    AnimationEngine* animEngine,
    std::function<void(const QPoint&)> moveCallback,
    std::function<void()> resetInactivityCallback)
{
    // 1. Sommeil & réveil spontané
    if (m_state == CompanionState::Sleeping) {
        m_stateTicks--;
        if (m_stateTicks <= 0) {
            chooseNextState(screenBounds, width, currentPos.x(), moveSpeed, animEngine);
            if (resetInactivityCallback) resetInactivityCallback();
        }
        return;
    }

    if (!isWalkModeEnabled) {
        if (m_state == CompanionState::Walking) {
            m_state = CompanionState::Idle;
            if (animEngine) animEngine->setState(AnimationEngine::AnimationState::Idle);
        }
        return;
    }

    // 2. Deplacement / Pause
    if (m_state == CompanionState::Walking) {
        bool isFlipped = false, reached = false;
        QPoint walkPos = physics.calculateNextWalkPosition(currentPos, m_targetX, moveSpeed, isFlipped, reached);

        if (animEngine) animEngine->setFlipped(isFlipped);

        if (reached || m_stateTicks <= 0) {
            m_state = CompanionState::Idle;
            m_stateTicks = QRandomGenerator::global()->bounded(20, 100);
            if (animEngine) animEngine->setState(AnimationEngine::AnimationState::Idle);
        }
        else {
            moveCallback(walkPos);
            m_stateTicks--;
        }
    }
    else {
        m_stateTicks--;
        if (m_stateTicks <= 0) {
            chooseNextState(screenBounds, width, currentPos.x(), moveSpeed, animEngine);
        }
    }
}