#include "PetCustomizer.h"
#include "AddSprites.h"
#include "../core/PetLoader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QEvent>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QPixmap>
#include <QListWidgetItem>

PetCustomizer::PetCustomizer(QWidget* parent)
	: QDialog(parent)
{
	setupUI();
	retranslateUI();
	loadStylesheet();
	populatePetList();
}

void PetCustomizer::setupUI()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // --- COLONNE GAUCHE (Liste des pets) ---
    auto* leftLayout = new QVBoxLayout();
    m_petListWidget = new QListWidget(this);
    leftLayout->addWidget(m_petListWidget);

    m_btnAdd = new QPushButton(this);
    leftLayout->addWidget(m_btnAdd);

    mainLayout->addLayout(leftLayout, 1);

    // --- COLONNE DROITE (Aperçu + Actions) ---
    auto* rightLayout = new QVBoxLayout();

    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedSize(160, 160);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("border: 2px dashed #F5A97F; border-radius: 12px; background: #1E1E2E;");
    rightLayout->addWidget(m_previewLabel, 0, Qt::AlignCenter);

    auto* btnLayout = new QHBoxLayout();
    m_btnSelect = new QPushButton(this);
    m_btnDelete = new QPushButton(this);
    m_btnClose = new QPushButton(this);

    btnLayout->addWidget(m_btnSelect);
    btnLayout->addWidget(m_btnDelete);
    rightLayout->addLayout(btnLayout);
    rightLayout->addStretch();
    rightLayout->addWidget(m_btnClose);

    mainLayout->addLayout(rightLayout, 1);

    // --- CONNEXIONS ---
    connect(m_btnAdd, &QPushButton::clicked, this, &PetCustomizer::openAddSpritesDialog);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_petListWidget, &QListWidget::itemSelectionChanged, this, &PetCustomizer::updatePreview);

    connect(m_btnSelect, &QPushButton::clicked, this, [this]() {
        if (auto* item = m_petListWidget->currentItem()) {
            QString petId = item->data(Qt::UserRole).toString();
            emit petSelected(petId);
            accept();
        }
    });

    connect(m_btnDelete, &QPushButton::clicked, this, &PetCustomizer::deleteSelectedPet);
}

void PetCustomizer::populatePetList()
{
    m_petListWidget->clear();

	// Ajout du compagnon par défaut
    auto* defaultItem = new QListWidgetItem(tr("Renard (Défaut)"), m_petListWidget);
    defaultItem->setData(Qt::UserRole, PetLoader::DEFAULT_FOX_ID);

	//Scan du dossier des compagnons
	QString baseAppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir petsDir(QDir::cleanPath(baseAppData + "/pets"));

	if (petsDir.exists()) {
		QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QString& folderName : petFolders) {
            QDir subDir(petsDir.filePath(folderName));
            if (subDir.exists("pet.json")) {
                auto* customItem = new QListWidgetItem(folderName, m_petListWidget);
                customItem->setData(Qt::UserRole, folderName);
            }
		}
	}
}

void PetCustomizer::updatePreview()
{
    m_previewLabel->clear();
    auto* currentItem = m_petListWidget->currentItem();
    if (!currentItem) {
        m_previewLabel->clear();
        return;
    }

    QString petId = currentItem->data(Qt::UserRole).toString();

    // Utilisation de PetLoader pour récupérer les données d'animation
    PetAnimationData petData = PetLoader::loadPet(petId);
    QString previewPath = petData.getFirstAvailableFrame();

    if (!previewPath.isEmpty()) {
        QPixmap pix(previewPath);
        if (!pix.isNull()) {
            m_previewLabel->setPixmap(pix.scaled(m_previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void PetCustomizer::deleteSelectedPet()
{
    auto* currentItem = m_petListWidget->currentItem();
    if (!currentItem) return;

    QString petId = currentItem->data(Qt::UserRole).toString();

    // Sécurité : Interdiction de supprimer le compagnon par défaut
    if (petId == PetLoader::DEFAULT_FOX_ID) {
        QMessageBox::information(this, tr("Info"), tr("Impossible de supprimer le compagnon par défaut."));
        return;
    }

    auto reply = QMessageBox::question(
        this,
        tr("Confirmation"),
        tr("Voulez-vous vraiment supprimer le compagnon '%1' ?").arg(currentItem->text()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QString baseAppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir petDir(QDir::cleanPath(baseAppData + "/pets/" + petId));

        if (petDir.exists()) {
            petDir.removeRecursively(); // Suppression propre du dossier et des assets
        }

        populatePetList();
        m_previewLabel->clear();
    }
}

void PetCustomizer::openAddSpritesDialog()
{
	AddSprites dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		populatePetList();
	}
}

void PetCustomizer::retranslateUI()
{
	setWindowTitle(tr("Changer de compagnon"));
	if (m_btnAdd) m_btnAdd->setText(tr("Ajouter compagnon"));
	if (m_btnSelect) m_btnSelect->setText(tr("Sélectionner"));
	if (m_btnDelete) m_btnDelete->setText(tr("Supprimer"));
	if (m_btnClose) m_btnClose->setText(tr("Retour"));
}

void PetCustomizer::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange) {
		retranslateUI();
	}
	QDialog::changeEvent(event);
}

void PetCustomizer::loadStylesheet()
{
	QFile file(":/style/ressources/style/menu.qss");
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QTextStream stream(&file);
		setStyleSheet(stream.readAll());
		file.close();
	}
}