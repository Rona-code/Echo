#include "ExplorerWindow.h"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QProcess>
#include <QSettings>
#include <QMessageBox>

ExplorerWindow::ExplorerWindow(QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(tr("Explorateur de fichiers"));
	resize(720, 480);
	setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

	setupUI();

	QSettings settings;

	m_currentPath = settings.value("explorer/lastPath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
	m_lastPath = m_currentPath;
	m_pathInput->setText(m_currentPath);

	loadDirectory(m_currentPath);
}

void ExplorerWindow::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // Barre supérieure : Chemin et Bouton Retour
    auto* topLayout = new QHBoxLayout();
    m_btnLastPath = new QPushButton(tr("Racine/Dernier"), this);
    m_btnBack = new QPushButton(tr("Fermer"), this);
    m_pathInput = new QLineEdit(this);

    topLayout->addWidget(m_btnLastPath);
    topLayout->addWidget(m_pathInput, 1);
    topLayout->addWidget(m_btnBack);
    mainLayout->addLayout(topLayout);

    // Barre de recherche
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Rechercher dans ce dossier et sous-dossiers..."));
    mainLayout->addWidget(m_searchInput);

    // Liste des fichiers / dossiers
    m_listWidget = new QListWidget(this);
    mainLayout->addWidget(m_listWidget);

	// Initialisation du timer de "Debounce" (300 ms de pause après la frappe)
	m_searchTimer = new QTimer(this);
	m_searchTimer->setSingleShot(true);
	m_searchTimer->setInterval(1000);

    // Connexions
    connect(m_pathInput, &QLineEdit::returnPressed, this, &ExplorerWindow::onPathReturnPressed);
    connect(m_btnLastPath, &QPushButton::clicked, this, &ExplorerWindow::onLastPathClicked);
    connect(m_btnBack, &QPushButton::clicked, this, &ExplorerWindow::close);
	connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &ExplorerWindow::onItemClicked);
	connect(m_searchInput, &QLineEdit::textChanged, this, [this]() {
		m_searchTimer->start();
	});
	connect(m_searchTimer, &QTimer::timeout, this, [this]() {
		onSearchTextChanged(m_searchInput->text());
	});

}

void ExplorerWindow::loadDirectory(const QString& path)
{
	QDir dir(path);
	if (!dir.exists()) return;

	m_currentPath = path;
	m_pathInput->setText(m_currentPath);
	m_listWidget->clear();
	m_truePathList.clear();

	QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden;
	QFileInfoList entries = dir.entryInfoList(filters, QDir::DirsFirst);

	for (const QFileInfo& fileInfo : entries) {
		if (isHiddenFile(fileInfo.absoluteFilePath())) continue;
        
		if (fileInfo.isDir()) {
			m_listWidget->addItem(tr("[Dossier]") + fileInfo.fileName());
		}
		else {
			m_listWidget->addItem(fileInfo.fileName());
		}
		m_truePathList.append(fileInfo.absoluteFilePath());
	}
}

void ExplorerWindow::onItemClicked(QListWidgetItem* item)
{
	if (!item) return;
	int row = m_listWidget->row(item);
	if (row < 0 || row >= m_truePathList.size()) return;

	QString fullPath = m_truePathList[row];
	QFileInfo fileInfo(fullPath);

	if (fileInfo.isDir()) {
		m_lastPath = m_currentPath;
		loadDirectory(fullPath);
		m_searchInput->clear();
	}
	else {
		QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
	}
}

void ExplorerWindow::onPathReturnPressed()
{
	QString path = m_pathInput->text().trimmed();
	if (QDir(path).exists()) {
		m_lastPath = m_currentPath;
		loadDirectory(path);
		m_searchInput->clear();
	}
	else {
		QMessageBox::warning(this, tr("Erreur"), tr("Le chemin spécifié n'existe pas."));
	}
}

void ExplorerWindow::onLastPathClicked()
{
	if (QDir(m_lastPath).exists()) {
		loadDirectory(m_lastPath);
		m_searchInput->clear();
	}
	else {
		QMessageBox::warning(this, tr("Erreur"), tr("Le chemin précédent n'existe pas."));
	}
}

void ExplorerWindow::onSearchTextChanged(const QString& text)
{
	QString searchText = text.trimmed().toLower();
	if (searchText.isEmpty()) {
		loadDirectory(m_currentPath);
		return;
	}

	m_listWidget->clear();
	m_truePathList.clear();
	searchInDirectory(m_currentPath, searchText);
}

void ExplorerWindow::searchInDirectory(const QString& directory, const QString& searchText)
{
	QDir dir(directory);
	if (!dir.exists()) return;

	QDirIterator it(directory, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

	// Listes temporaires pour accumuler en mémoire sans bloquer l'affichage
	QStringList itemsToAdd;
	QStringList pathsToAdd;

	while (it.hasNext()) {
		QString filePath = it.next();
		QFileInfo fileInfo(filePath);

		if (isHiddenFile(filePath)) continue;

		if (fileInfo.fileName().toLower().contains(searchText)) {
			if (fileInfo.isDir()) {
				itemsToAdd.append(tr("[Dossier] ") + fileInfo.fileName());
			}
			else {
				itemsToAdd.append(fileInfo.fileName());
			}
			pathsToAdd.append(filePath);
		}
	}

	// Injection unique dans l'interface : 0 lag de rendu !
	m_listWidget->addItems(itemsToAdd);
	m_truePathList.append(pathsToAdd);
}

bool ExplorerWindow::isHiddenFile(const QString& filePath) const
{
	QFileInfo fileInfo(filePath);
#if defined(Q_OS_UNIX)
	return fileInfo.isHidden() || fileInfo.fileName().startsWith('.');
#else
	return fileInfo.isHidden();
#endif
}

void ExplorerWindow::closeEvent(QCloseEvent* event)
{
	// Sauvegarder le dernier chemin visité
	QSettings settings;
	settings.setValue("explorer/lastPath", m_currentPath);
	QWidget::closeEvent(event);
}