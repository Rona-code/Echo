#include "BehaviorController.h"
#include "PhysicsController.h"
#include "AnimationEngine.h"
#include <QRandomGenerator>

void BehaviorController::chooseNextState(const QRect& screenBounds, int companionWidth, AnimationEngine* animEngine) {
    int roll = QRandomGenerator::global()->bounded(100);

    if (roll < 60) {
        m_state = CompanionState::Idle;
        m_stateTicks = QRandomGenerator::global()->bounded(20, 100);
    }
    else {
        m_state = CompanionState::Walking;
        int minX = screenBounds.left();
        int maxX = std::max(minX, screenBounds.right() - companionWidth);
        m_targetX = static_cast<float>(QRandomGenerator::global()->bounded(minX, maxX));
        m_stateTicks = 300;
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
            chooseNextState(screenBounds, width, animEngine);
            if (resetInactivityCallback) resetInactivityCallback();
        }
        return;
    }

    // 2. Gravité
    QPoint newPos = physics.applyGravity(currentPos, width, height, screenBounds);
    if (newPos != currentPos) {
        moveCallback(newPos);
        return;
    }

    if (!isWalkModeEnabled) {
        if (m_state == CompanionState::Walking) {
            m_state = CompanionState::Idle;
            if (animEngine) animEngine->setState(AnimationEngine::AnimationState::Idle);
        }
        return;
    }

    // 3. Deplacement / Pause
    if (m_state == CompanionState::Walking) {
        bool isFlipped = false, reached = false;
        QPoint walkPos = physics.calculateNextWalkPosition(currentPos, m_targetX, moveSpeed, isFlipped, reached);

        if (animEngine) animEngine->setFlipped(isFlipped);

        if (reached || m_stateTicks <= 0) {
            m_state = CompanionState::Idle;
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
            chooseNextState(screenBounds, width, animEngine);
        }
    }
}