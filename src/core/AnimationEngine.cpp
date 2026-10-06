#include "AnimationEngine.h"

AnimationEngine::AnimationEngine(QObject* parent)
	: QObject(parent)
{
	m_timer.setInterval(DEFAULT_INTERVAL_MS);
	connect(&m_timer, &QTimer::timeout, this, &AnimationEngine::onTimeout);
}

void AnimationEngine::setInterval(int intervalMs)
{
	if (intervalMs > 0) {
		m_timer.setInterval(intervalMs);
	}
}

void AnimationEngine::setPetData(const PetAnimationData& data)
{
	m_petData = data;
	m_currentFrameIndex = 0;
	setState(AnimationState::Idle);
}

void AnimationEngine::setState(AnimationState state)
{
	if (m_currentState == state) return;
	m_currentState = state;
	m_currentFrameIndex = 0;
	QStringList frames = getCurrentFrameList();
	if (!frames.isEmpty()) {
		emit frameChanged(frames.at(0));
	}
}

void AnimationEngine::start(int intervalMs)
{
	m_timer.start(intervalMs);
	m_timer.start();
}

void AnimationEngine::stop()
{
	m_timer.stop();
}

QStringList AnimationEngine::getCurrentFrameList() const
{
	switch (m_currentState) {
		case AnimationState::Walking:  return !m_petData.walkFrames.isEmpty() ? m_petData.walkFrames : m_petData.idleFrames;
		case AnimationState::Sleeping: return !m_petData.sleepFrames.isEmpty() ? m_petData.sleepFrames : m_petData.idleFrames;
		case AnimationState::Idle:
		default:           return m_petData.idleFrames;
	}
}

void AnimationEngine::onTimeout()
{
	QStringList frames = getCurrentFrameList();
	if (frames.isEmpty()) return;
	
	if (m_currentFrameIndex >= frames.size()) {
		m_currentFrameIndex = 0;
	}

	emit frameChanged(frames.at(m_currentFrameIndex));
	m_currentFrameIndex++;
}

void AnimationEngine::setFlipped(bool flipped)
{
	if (m_isFlipped != flipped) {
		m_isFlipped = flipped;
		updateFrame();
	}
}

void AnimationEngine::updateFrame()
{
	QStringList frames = getCurrentFrameList();
	if (frames.isEmpty()) return;

	if (m_currentFrameIndex >= frames.size()) {
		m_currentFrameIndex = 0;
	}

	emit frameChanged(frames.at(m_currentFrameIndex));
}