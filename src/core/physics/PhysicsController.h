#pragma once

#include <QPoint>
#include <QRect>
#include <QList>

class Companion;

class PhysicsController {
public:
    PhysicsController() = default;

    QPoint calculateNextWalkPosition(const QPoint& currentPos, float targetX, float moveSpeed, bool& outIsFlipped, bool& outReached) const;

	void updatePhysics(Companion* companion, const QList<QRect>& windowBounds, const QRect& screenGeometry);

private:
	float m_verticalVelocity = 0.0f;
	float m_maxSpeedReached = 0.0f;
	static constexpr float GRAVITY_ACCELERATION = 1.2f;
};