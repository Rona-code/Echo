#include "PhysicsController.h"
#include "../../ui/windows/Companion.h"
#include "../managers/AchievementManager.h"
#include <algorithm>
#include <cmath>
#include <QPoint>

QPoint PhysicsController::calculateNextWalkPosition(const QPoint& currentPos, float targetX, float moveSpeed, bool& outIsFlipped, bool& outReached) const {
    float deltaX = targetX - currentPos.x();

    if (std::abs(deltaX) <= moveSpeed) {
        outReached = true;
        return QPoint(static_cast<int>(targetX), currentPos.y());
    }

    outReached = false;
    float direction = (deltaX > 0) ? 1.0f : -1.0f;
    outIsFlipped = (direction < 0);

    return QPoint(static_cast<int>(currentPos.x() + direction * moveSpeed), currentPos.y());
}

void PhysicsController::updatePhysics(Companion* companion, const QList<QRect>& windowBounds, const QRect& screenGeometry)
{
    if (!companion || companion->isDragging()) return;

    QPoint currentPos = companion->pos();
    int petHeight = companion->height();
    int petCenterX = currentPos.x() + (companion->width() / 2);

    int petBottomBefore = currentPos.y() + petHeight;
    int targetGroundY = screenGeometry.bottom() - petHeight;
    bool foundGround = false;


    for (int i = 0; i < windowBounds.size(); ++i)
    {
        const QRect& window = windowBounds[i];

        if (petCenterX < window.left() || petCenterX > window.right()) continue;

        int windowTop = window.top();
        if (windowTop < petBottomBefore - 20) continue;

        bool occluded = false;
        QPoint edgePoint(petCenterX, windowTop);
        for (int j = 0; j < i; ++j) {
            if (windowBounds[j].contains(edgePoint)) { occluded = true; break; }
        }
        if (occluded) continue;

        int projectedBottom = petBottomBefore + static_cast<int>(m_verticalVelocity);

        if (petBottomBefore <= windowTop && projectedBottom >= windowTop && m_verticalVelocity >= 0)
        {
            int candidateGroundY = windowTop - petHeight;
            if (!foundGround || candidateGroundY < targetGroundY) {
                targetGroundY = candidateGroundY;
                foundGround = true;
            }
        }
    }

    if (currentPos.y() < targetGroundY)
    {
        m_verticalVelocity += GRAVITY_ACCELERATION;

        if (m_verticalVelocity > m_maxSpeedReached) {
            m_maxSpeedReached = m_verticalVelocity;
        }

        int nextY = currentPos.y() + static_cast<int>(m_verticalVelocity);

        if (nextY >= targetGroundY) {
            nextY = targetGroundY;

            AchievementManager::instance().checkAndSaveMaxSpeed(m_maxSpeedReached);

            m_verticalVelocity = 0.0f;
            m_maxSpeedReached = 0.0f;
            companion->setGrounded(true);
        }
        else {
            companion->setGrounded(false);
        }
        companion->move(currentPos.x(), nextY);
    }
    else if (currentPos.y() > targetGroundY)
    {
        companion->move(currentPos.x(), targetGroundY);
        m_verticalVelocity = 0.0f;
        companion->setGrounded(true);
    }
    else
    {
        m_verticalVelocity = 0.0f;
        companion->setGrounded(true);
    }
}