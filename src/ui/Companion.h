#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QPropertyAnimation>
#include <QTimer>

#include "../core/PetLoader.h"
#include "../core/PhysicsController.h"
#include "../core/BehaviorController.h"
#include "ActionMenu.h"

class ActionMenu;
class AnimationEngine;

class Companion : public QWidget {
    Q_OBJECT

public:
    explicit Companion(QWidget* parent = nullptr);
    ~Companion() override;

    float scale() const { return m_scale; }
    int animSpeed() const { return m_animSpeed; }
    int inactivityTimeoutSec() const;
    bool isWalkModeEnabled() const { return m_isWalkModeEnabled; }
    const PetAnimationData& currentPetData() const { return m_currentPetData; }

    void applySettings(float scale, int speedMs, int timeoutSec);
    void setWalkModeEnabled(bool enabled);
    void loadPet(const QString& petId);
    void clearActiveMenu();

public slots:
    void setSprite(const QString& imagePath);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void resetInactivityTimer();
    void updateAnimationState();
    void triggerBounce();

    QPixmap m_pixmap;
    QString m_currentImagePath;
    QPoint m_dragPosition;

    QTimer* m_inactivityTimer{ nullptr };
    QTimer* m_behaviorTimer{ nullptr };
    QPropertyAnimation* m_bounceAnim{ nullptr };

    float m_moveSpeed{ 2.0f };
    float m_scale{ 0.5f };
    bool m_isWalkModeEnabled{ false };
    int m_animSpeed{ 500 };

    PetAnimationData m_currentPetData;
    AnimationEngine* m_animationEngine{ nullptr };
    ActionMenu* m_activeMenu{ nullptr };
    PhysicsController m_physics;
    BehaviorController m_behavior;
};