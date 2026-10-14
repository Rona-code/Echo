#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QPropertyAnimation>
#include <QTimer>

#include "ActionMenu.h"
#include "../../core/managers/PetLoader.h"
#include "../../core/physics/PhysicsController.h"
#include "../../core/engine/BehaviorController.h"

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
    bool isDragging() const;
    bool isGrounded() const;
    const PetAnimationData& currentPetData() const { return m_currentPetData; }

    void applySettings(float scale, int speedMs, int timeoutSec);
    void setWalkModeEnabled(bool enabled);
    void loadPet(const QString& petId);
    void clearActiveMenu();
	void setGrounded(bool grounded);

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
	bool m_grounded = false;
	bool m_isDragging = false;
    int m_animSpeed{ 500 };

    PetAnimationData m_currentPetData;
    AnimationEngine* m_animationEngine{ nullptr };
    ActionMenu* m_activeMenu{ nullptr };
    PhysicsController m_physics;
    BehaviorController m_behavior;
	std::unique_ptr<PhysicsController> m_physicsController;
};