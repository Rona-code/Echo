#include "AddSprites.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QEvent>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

AddSprites::AddSprites(QWidget* parent)
	: QDialog(parent)
{
	setupUI();
	retranslateUI();
}

void AddSprites::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // Nom du Pet
    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel(tr("Nom :"), this);
    m_nameInput = new QLineEdit(this);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameInput);
    mainLayout->addLayout(nameLayout);

    // Helper lambda pour créer une section (Label + ListWidget + Bouton Ajouter)
    auto createCategory = [this](const QString& title, QListWidget*& listWidget, QPushButton*& btn) {
        auto* boxLayout = new QVBoxLayout();
        auto* headerLayout = new QHBoxLayout();

        auto* label = new QLabel(title, this);
        btn = new QPushButton(tr("Ajouter"), this);

        headerLayout->addWidget(label);
        headerLayout->addStretch();
        headerLayout->addWidget(btn);

        listWidget = new QListWidget(this);
        listWidget->setFixedHeight(70);

        boxLayout->addLayout(headerLayout);
        boxLayout->addWidget(listWidget);

        connect(btn, &QPushButton::clicked, this, [this, listWidget]() {
            addImagesToList(listWidget);
            });

        return boxLayout;
        };

    mainLayout->addLayout(createCategory(tr("Mouvement"), m_listWalk, m_btnAddWalk));
    mainLayout->addLayout(createCategory(tr("Attente"), m_listIdle, m_btnAddIdle));
    mainLayout->addLayout(createCategory(tr("Pause"), m_listSleep, m_btnAddSleep));

    // Boutons de validation
    auto* actionLayout = new QHBoxLayout();
    m_btnCancel = new QPushButton(tr("Annuler"), this);
    m_btnConfirm = new QPushButton(tr("Confirmer"), this);

    actionLayout->addStretch();
    actionLayout->addWidget(m_btnCancel);
    actionLayout->addWidget(m_btnConfirm);
    mainLayout->addLayout(actionLayout);

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnConfirm, &QPushButton::clicked, this, &AddSprites::savePet);
}

void AddSprites::addImagesToList(QListWidget* targetList)
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Sélectionner des images PNG"),
        QString(),
        tr("Images PNG (*.png)")
    );

    for (const QString& file : files) {
        targetList->addItem(QDir::cleanPath(file));
    }
}

void AddSprites::savePet()
{
    QString petName = m_nameInput->text().trimmed();
    if (petName.isEmpty()) {
        QMessageBox::warning(this, tr("Erreur"), tr("Veuillez entrer un nom pour le compagnon."));
        return;
    }

    // 1. Définition du chemin AppData / Share de manière portable
    QString baseAppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir petsBaseDir(QDir::cleanPath(baseAppData + "/pets"));

    // Création sécurisée du dossier racine si inexistant
    if (!petsBaseDir.exists()) {
        petsBaseDir.mkpath(".");
    }

    // Dossier dédié au pet
    QDir targetPetDir(petsBaseDir.filePath(petName));
    if (targetPetDir.exists()) {
        QMessageBox::warning(this, tr("Erreur"), tr("Un compagnon portant ce nom existe déjà."));
        return;
    }

    targetPetDir.mkpath(".");

    // 2. Lambda portable pour la copie physique et la structure JSON
    auto copyCategoryFiles = [&targetPetDir](QListWidget* listWidget, const QString& subDir) -> QJsonArray {
        targetPetDir.mkdir(subDir);
        QDir subCategoryDir(targetPetDir.filePath(subDir));
        QJsonArray jsonPathList;

        for (int i = 0; i < listWidget->count(); ++i) {
            QString sourcePath = listWidget->item(i)->text();
            QFileInfo fileInfo(sourcePath);

            // Renommage propre (ex: frame_0.png) pour éviter les problèmes d'encodage/caractères spéciaux sous Linux
            QString targetFileName = QString("frame_%1.%2").arg(i).arg(fileInfo.suffix().toLower());
            QString destPath = subCategoryDir.filePath(targetFileName);

            if (QFile::copy(sourcePath, destPath)) {
                // On enregistre un chemin relatif universel avec '/' pour le JSON
                jsonPathList.append(subDir + "/" + targetFileName);
            }
        }
        return jsonPathList;
     };

    // 3. Construction du JSON
    QJsonObject petJson;
    petJson["name"] = petName;
    petJson["version"] = 1;
    petJson["walk"] = copyCategoryFiles(m_listWalk, "walk");
    petJson["idle"] = copyCategoryFiles(m_listIdle, "idle");
    petJson["sleep"] = copyCategoryFiles(m_listSleep, "sleep");

    // 4. Écriture du fichier manifest pet.json
    QFile jsonFile(targetPetDir.filePath("pet.json"));
    if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        jsonFile.write(QJsonDocument(petJson).toJson(QJsonDocument::Indented));
        jsonFile.close();
        accept(); // Fermeture et confirmation
    }
    else {
        QMessageBox::critical(this, tr("Erreur"), tr("Impossible d'enregistrer la configuration du compagnon."));
    }
}

void AddSprites::retranslateUI()
{
    setWindowTitle(tr("Ajouter des Sprites"));
    if (m_btnConfirm) m_btnConfirm->setText(tr("Confirmer"));
	if (m_btnCancel) m_btnCancel->setText(tr("Annuler"));
}

void AddSprites::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUI();
    }
    QDialog::changeEvent(event);
}