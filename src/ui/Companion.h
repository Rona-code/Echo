#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>
#include <QPropertyAnimation>
#include <QTimer>

#include "../core/PetLoader.h"
#include "../core/PhysicsController.h"
#include "../core/BehaviorController.h"

class ActionMenu;
class AnimationEngine;

class Companion : public QWidget
{
    Q_OBJECT

public:
    explicit Companion(QWidget* parent = nullptr);
    ~Companion() override;

    float scale() const { return m_scale; }
    void setScale(float newScale);
    void setAnimationSpeed(int intervalMs);
    void setInactivityTimeout(int seconds);
    void setSprite(const QString& imagePath);
    void setPetAnimationData(const PetAnimationData& data);
    void setWalkModeEnabled(bool enabled);
    void loadPet(const QString& petId);

    bool isWalkModeEnabled() const { return m_isWalkModeEnabled; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onInactivityTimeout();
    void updateBehavior();

private:
    void setupTimers();
    void resetInactivityTimer();
    void updateAnimationState();
    QRect getScreenBounds() const;

    QPixmap m_pixmap;
    QString m_currentImagePath;
    QPoint m_dragPosition;

    QPointer<ActionMenu> m_activeMenu = nullptr;
    QTimer* m_inactivityTimer{ nullptr };
    QTimer* m_behaviorTimer{ nullptr };
    QPropertyAnimation* m_bounceAnim{ nullptr };

    float m_moveSpeed{ 2.0f };
    float m_scale{ 0.5f };
    bool m_isWalkModeEnabled{ false };
    int m_animSpeed{ 500 };

    PetAnimationData m_currentPetData;
    AnimationEngine* m_animationEngine{ nullptr };

    PhysicsController m_physics;
    BehaviorController m_behavior;
};