#include "PhysicsController.h"
#include "../ui/Companion.h"
#include <algorithm>
#include <cmath>
#include <QPoint>

QPoint PhysicsController::calculateNextWalkPosition(const QPoint& currentPos, float targetX, float moveSpeed, bool& outIsFlipped, bool& outReached) const {
    float deltaX = targetX - currentPos.x();

    if (std::abs(deltaX) <= moveSpeed) {
        outReached = true;
        return currentPos;
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


    for (const QRect& window : windowBounds)
    {

        if (petCenterX >= window.left() && petCenterX <= window.right())
        {
            int windowTop = window.top();

            if (windowTop < petBottomBefore - 20) {
                continue; 
            }

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
    }

    if (currentPos.y() < targetGroundY)
    {
        m_verticalVelocity += GRAVITY_ACCELERATION;
        int nextY = currentPos.y() + static_cast<int>(m_verticalVelocity);

        if (nextY >= targetGroundY) {
            nextY = targetGroundY;
            m_verticalVelocity = 0.0f;
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