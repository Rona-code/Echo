#pragma once

#include <QDialog>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QWidget* parent = nullptr);

	double scaleFactor() const;
	int animSpeedMs() const;
	int idleTimeoutSec() const;

	void setValues(double scale, int animSpeed, int idleTimeout);

signals:
	void settingsChanged(double scale, int animSpeed, int idleTimeout);

private:
	void setupUI();

	QSlider* m_scaleSlider{ nullptr };
	QLabel* m_scaleValLabel{ nullptr };
	QSlider* m_animSpeedSlider{ nullptr };
	QLabel* m_animSpeedValLabel{ nullptr };
	QSlider* m_idleTimeoutSlider{ nullptr };
	QLabel* m_idleTimeoutValLabel{ nullptr };
	QPushButton* m_btnSave{ nullptr };
};