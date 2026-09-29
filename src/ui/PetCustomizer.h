#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

class PetCustomizer : public QDialog
{
	Q_OBJECT

public:
	explicit PetCustomizer(QWidget* parent = nullptr);

signals:
	void petSelected(const QString& petName);

protected:
	void changeEvent(QEvent* event) override;

private:
	void setupUI();
	void loadStylesheet();
	void retranslateUI();
	void populatePetList();
	void openAddSpritesDialog();
	void updatePreview();
	void deleteSelectedPet();

	QListWidget* m_petListWidget{ nullptr };
	QLabel* m_previewLabel{ nullptr };

	QPushButton* m_btnSelect{ nullptr };
	QPushButton* m_btnAdd{ nullptr };
	QPushButton* m_btnDelete{ nullptr };
	QPushButton* m_btnClose{ nullptr };
};