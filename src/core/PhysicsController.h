#pragma once

#include <QPoint>
#include <QRect>

class PhysicsController {
public:
    PhysicsController() = default;

    int calculateGroundY(const QPoint& currentPos, int width, int height, const QRect& screenBounds) const;
    QPoint applyGravity(const QPoint& currentPos, int width, int height, const QRect& screenBounds, int fallSpeed = 6) const;
    QPoint calculateNextWalkPosition(const QPoint& currentPos, float targetX, float moveSpeed, bool& outIsFlipped, bool& outReached) const;
};