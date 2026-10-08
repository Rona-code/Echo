#pragma once
#include <QRect>

enum class CompanionState {
    Idle,
    Walking,
    Sleeping
};

class BehaviorController {
public:
    BehaviorController() = default;

    CompanionState state() const { return m_state; }
    void setState(CompanionState state) { m_state = state; }

    float targetX() const { return m_targetX; }
    int stateTicks() const { return m_stateTicks; }

    void decrementTicks() { if (m_stateTicks > 0) m_stateTicks--; }
    bool isTicksExpired() const { return m_stateTicks <= 0; }

    void chooseNextState(const QRect& screenBounds, int companionWidth);

private:
    CompanionState m_state{ CompanionState::Idle };
    float m_targetX{ 0.0f };
    int m_stateTicks{ 0 };
};