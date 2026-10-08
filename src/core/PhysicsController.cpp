#include "PhysicsController.h"
#include "WindowDetector.h"
#include <algorithm>
#include <cmath>

int PhysicsController::calculateGroundY(const QPoint& currentPos, int width, int height, const QRect& screenBounds) const {
    int defaultGroundY = screenBounds.bottom() - height;
    QRect targetWindow = WindowDetector::getTargetWindowGeometry();

    if (targetWindow.isValid()) {
        int windowTop = targetWindow.top() - height;
        int companionX = currentPos.x() + width / 2;

        if (companionX >= targetWindow.left() && companionX <= targetWindow.right()) {
            if (windowTop < defaultGroundY && windowTop >= currentPos.y() - 15) {
                return windowTop;
            }
        }
    }
    return defaultGroundY;
}

QPoint PhysicsController::applyGravity(const QPoint& currentPos, int width, int height, const QRect& screenBounds, int fallSpeed) const {
    int groundY = calculateGroundY(currentPos, width, height, screenBounds);
    if (currentPos.y() < groundY) {
        int nextY = std::min(currentPos.y() + fallSpeed, groundY);
        return QPoint(currentPos.x(), nextY);
    }
    return currentPos;
}

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