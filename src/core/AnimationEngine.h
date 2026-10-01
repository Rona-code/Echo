#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>

#include "PetLoader.h"

class AnimationEngine : public QObject
{
	Q_OBJECT

public:
	static constexpr int DEFAULT_INTERVAL_MS = 500;

	enum class AnimationState {
		Idle,
		Walking,
		Sleeping
	};

	explicit AnimationEngine(QObject* parent = nullptr);

	void setPetData(const PetAnimationData& data);
	void setState(AnimationState state);

	void start(int intervalMs = DEFAULT_INTERVAL_MS);
	void stop();
	void setInterval(int intervalMs);

signals:
	void frameChanged(const QString& framePath);

private slots:
	void onTimeout();

private:
	QTimer m_timer;
	QStringList getCurrentFrameList() const;

	int m_currentFrameIndex = 0;

	PetAnimationData m_petData;
	AnimationState m_currentState = AnimationState::Idle;
};