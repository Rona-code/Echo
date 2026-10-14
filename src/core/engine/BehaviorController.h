#pragma once
#include <QRect>
#include <QPoint>

enum class CompanionState { Idle, Walking, Sleeping };

class PhysicsController;
class AnimationEngine;

class BehaviorController {
public:
    BehaviorController() = default;

    CompanionState state() const { return m_state; }
    void setState(CompanionState state) { m_state = state; }
    void setTicks(int ticks) { m_stateTicks = ticks; }

    void processBehaviorTick(
        QPoint currentPos,
        int width, int height,
        const QRect& screenBounds,
        bool isWalkModeEnabled,
        float moveSpeed,
        PhysicsController& physics,
        AnimationEngine* animEngine,
        std::function<void(const QPoint&)> moveCallback,
        std::function<void()> resetInactivityCallback
    );

private:
    void chooseNextState(const QRect& screenBounds, int companionWidth, AnimationEngine* animEngine);

    CompanionState m_state{ CompanionState::Idle };
    float m_targetX{ 0.0f };
    int m_stateTicks{ 0 };
};