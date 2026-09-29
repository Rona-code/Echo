#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>

class AddSprites : public QDialog
{
	Q_OBJECT

public:
	explicit AddSprites(QWidget* parent = nullptr);

protected:
	void changeEvent(QEvent* event) override;

private:
	void setupUI();
	void retranslateUI();
	void addImagesToList(QListWidget* targetList);
	void savePet();

	QLineEdit* m_nameInput{ nullptr };

	QListWidget* m_listWalk{ nullptr };
	QListWidget* m_listIdle{ nullptr };
	QListWidget* m_listSleep{ nullptr };

	QPushButton* m_btnAddWalk{ nullptr };
	QPushButton* m_btnAddIdle{ nullptr };
	QPushButton* m_btnAddSleep{ nullptr };

	QPushButton* m_btnConfirm{ nullptr };
	QPushButton* m_btnCancel{ nullptr };
};