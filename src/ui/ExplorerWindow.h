#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QTimer>

class ExplorerWindow : public QWidget
{
	Q_OBJECT

public:
	explicit ExplorerWindow(QWidget* parent = nullptr);

private slots:
	void loadDirectory(const QString& path);
	void onItemClicked(QListWidgetItem* item);
	void onSearchTextChanged(const QString& text);
	void onLastPathClicked();
	void onPathReturnPressed();

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	void setupUI();
	void searchInDirectory(const QString& directory, const QString& searchText);
	
	bool isHiddenFile(const QString& filePath) const;

	QLineEdit* m_pathInput{ nullptr };
	QLineEdit* m_searchInput{ nullptr };
	QPushButton* m_btnLastPath{ nullptr };
	QPushButton* m_btnBack{ nullptr };
	QListWidget* m_listWidget{ nullptr };
	QTimer* m_searchTimer{ nullptr };

	QStringList m_truePathList;
	QString m_currentPath;
	QString m_lastPath;
};