#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPaintEvent>

class ActionMenu : public QWidget
{
	Q_OBJECT

public:
	explicit ActionMenu(bool isWalkModeEnabled, QWidget* parent = nullptr);
	void setPetIcon(const QString& iconPath);

signals:
	void launchAppsRequested();
	void closeAppsRequested();
	void openExplorerRequested();
	void settingsRequested();
	void languageChangedRequested(const QString& langCode);
	void quitRequested();
	void petClicked();
	void petChangedRequested(const QString& petId);
	void walkModeToggled(bool enabled);

protected:
	void paintEvent(QPaintEvent* event) override;
	void changeEvent(QEvent* event) override;

private:
	void setupUI(bool isWalkModeEnabled);
	void loadStyleSheet();
	void retranslateUi();
	void updateComboToCurrentLanguage();

	QPushButton* m_btnPet{ nullptr };
	QPushButton* m_btnWalk{ nullptr };
	QLabel* m_titleLabel{ nullptr };
	QComboBox* m_langComboBox{ nullptr };
	QPushButton* m_btnLaunch{ nullptr };
	QPushButton* m_btnClose{ nullptr };
	QPushButton* m_btnExplorer{ nullptr };
	QPushButton* m_btnQuit{ nullptr };

	bool m_isInitializing{ true };
};
