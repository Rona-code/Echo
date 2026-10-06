#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>
#include <QPropertyAnimation>

#include "../core/PetLoader.h"

class ActionMenu;
class AnimationEngine;

enum class CompanionState {
    Idle,
    Walking,
	Sleeping
};

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
	void setupBehavior();
	void resetInactivityTimer();
    void updateAnimationState();
	void updateMovement();
    void chooseNextState();

	CompanionState m_state{ CompanionState::Idle };

    QPixmap m_pixmap;
    QString m_currentImagePath;
    QPoint m_dragPosition;
    QPointer<ActionMenu> m_activeMenu = nullptr;
	QTimer* m_inactivityTimer;
	QTimer* m_behaviorTimer{nullptr};
    QPointF m_targetPos;
	QRect getScreenBounds() const;
	QPropertyAnimation* m_bounceAnim = nullptr;

	float m_moveSpeed{ 2.0f };
    float m_scale{ 0.5f };
	bool m_isWalkModeEnabled = false;
	int m_animSpeed = 500;
    int m_stateTicks{ 0 };

    PetAnimationData m_currentPetData;
	AnimationEngine* m_animationEngine = nullptr;
};