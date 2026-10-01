#include "ActionMenu.h"
#include "../commun/TranslationManager.h"
#include "PetCustomizer.h"
#include "Companion.h"
#include "../core/PetLoader.h"

#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QPointer>
#include <QCheckBox>

class ToggleSwitch : public QCheckBox {
public:
    explicit ToggleSwitch(QWidget* parent = nullptr) : QCheckBox(parent) {
        setCursor(Qt::PointingHandCursor);
        setFixedSize(40, 22);
        setStyleSheet(R"(
            QCheckBox::indicator {
                width: 38px;
                height: 20px;
                border-radius: 10px;
                border: 1px solid #494D64;
            }
            QCheckBox::indicator:unchecked {
                background-color: #363A4F;
            }
            QCheckBox::indicator:checked {
                background-color: #F5A97F;
                border-color: #F5A97F;
            }
            QCheckBox::indicator:unchecked:hover {
                background-color: #494D64;
            }
            QCheckBox::indicator:checked:hover {
                background-color: #EE996F;
            }
        )");
    }
};

ActionMenu::ActionMenu(bool isWalkModeEnabled, QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);

    setAttribute(Qt::WA_DeleteOnClose);

    setupUI(isWalkModeEnabled);
    retranslateUi();
    loadStyleSheet();

    updateComboToCurrentLanguage();

    m_isInitializing = false;
}

void ActionMenu::setupUI(bool isWalkModeEnabled)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // --- EN-TÊTE ---
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(8);

    m_langComboBox = new QComboBox(this);
    m_langComboBox->setObjectName("langCombo");
    m_langComboBox->setCursor(Qt::PointingHandCursor);
    m_langComboBox->addItem("FR", "fr");
    m_langComboBox->addItem("EN", "en");
    m_langComboBox->addItem("JA", "ja");

    m_btnPet = new QPushButton(this);
    m_btnPet->setObjectName("petBtn");
    m_btnPet->setCursor(Qt::PointingHandCursor);
    m_btnPet->setFixedSize(32, 32);
    m_btnPet->setFlat(true);

    PetAnimationData defaultPet = PetLoader::loadPet(PetLoader::DEFAULT_FOX_ID);
    QString initialIcon = defaultPet.getFirstAvailableFrame();
    if (!initialIcon.isEmpty()) {
        QPixmap petPixmap(initialIcon);
        if (!petPixmap.isNull()) {
            m_btnPet->setIcon(QIcon(petPixmap.scaled(28, 28, Qt::KeepAspectRatio, Qt::FastTransformation)));
            m_btnPet->setIconSize(QSize(28, 28));
        }
    }

    auto* toggleWalk = new ToggleSwitch(this);
    toggleWalk->setChecked(isWalkModeEnabled);

    headerLayout->addWidget(m_langComboBox);
    headerLayout->addWidget(m_btnPet);
    headerLayout->addStretch();
    headerLayout->addWidget(toggleWalk);

    mainLayout->addLayout(headerLayout);

    // --- CORPS (Boutons standard) ---
    auto* btnGrid = new QHBoxLayout();
    btnGrid->setSpacing(8);

    m_btnLaunch = new QPushButton(tr("Lancer apps"), this);
    m_btnLaunch->setCursor(Qt::PointingHandCursor);

    m_btnClose = new QPushButton(tr("Quitter apps"), this);
    m_btnClose->setCursor(Qt::PointingHandCursor);

    btnGrid->addWidget(m_btnLaunch);
    btnGrid->addWidget(m_btnClose);
    mainLayout->addLayout(btnGrid);

    m_btnExplorer = new QPushButton(tr("Explorateur de fichiers"), this);
    m_btnExplorer->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_btnExplorer);

    // --- PIED DE PAGE (Quitter + Paramètres) ---
    auto* footerLayout = new QHBoxLayout();
    footerLayout->setSpacing(8);

    m_btnQuit = new QPushButton(tr("Quitter Echo"), this);
    m_btnQuit->setObjectName("quitBtn"); // Attribue le style alerte pastel
    m_btnQuit->setCursor(Qt::PointingHandCursor);

    auto* btnSettings = new QPushButton("⚙️", this);
    btnSettings->setObjectName("iconBtn"); // Attribue le style rond 26x26px
    btnSettings->setCursor(Qt::PointingHandCursor);

    footerLayout->addWidget(m_btnQuit, 1);
    footerLayout->addWidget(btnSettings);

    mainLayout->addLayout(footerLayout);

    // --- CONNEXIONS ---
    connect(toggleWalk, &QCheckBox::toggled, this, [this](bool checked) {
        emit walkModeToggled(checked);
        });

    connect(m_btnPet, &QPushButton::clicked, this, [this]() {
        this->hide();
        auto* customizer = new PetCustomizer(nullptr);
        customizer->setAttribute(Qt::WA_DeleteOnClose);
        connect(customizer, &PetCustomizer::petSelected, this, [this](const QString& petId) {
            emit petChangedRequested(petId);
            });
        customizer->exec();
        this->deleteLater();
        });

    connect(m_btnLaunch, &QPushButton::clicked, this, [this]() { emit launchAppsRequested(); close(); });
    connect(m_btnClose, &QPushButton::clicked, this, [this]() { emit closeAppsRequested(); close(); });
    connect(m_btnExplorer, &QPushButton::clicked, this, [this]() { emit openExplorerRequested(); close(); });
    connect(btnSettings, &QPushButton::clicked, this, [this]() { emit settingsRequested(); close(); });
    connect(m_btnQuit, &QPushButton::clicked, this, [this]() { hide(); emit quitRequested(); });

    // Ombre portée
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
}

void ActionMenu::updateComboToCurrentLanguage()
{
    m_langComboBox->blockSignals(true);

    QString currentLang = TranslationManager::instance().currentLanguage();
    int index = m_langComboBox->findData(currentLang);
    if (index != -1) {
        m_langComboBox->setCurrentIndex(index);
    }
    m_langComboBox->blockSignals(false);
}

void ActionMenu::retranslateUi()
{
    m_btnLaunch->setText(tr("Lancer apps"));
    m_btnClose->setText(tr("Quitter apps"));
    m_btnExplorer->setText(tr("Explorateur de fichiers"));
    m_btnQuit->setText(tr("Quitter Echo"));

    const auto buttons = findChildren<QPushButton*>();
    for (auto* btn : buttons)
    {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        btn->update();
    }

    adjustSize();
    update();
}

void ActionMenu::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void ActionMenu::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF menuRect = rect().adjusted(2, 2, -2, -2);

    QPainterPath path;
    path.addRoundedRect(menuRect, 20, 20);

    QColor backgroundColor("#24273A");
    QColor borderColor("#F5A97F");

    painter.fillPath(path, backgroundColor);
    painter.strokePath(path, QPen(borderColor, 2));
}

void ActionMenu::setPetIcon(const QString& iconPath)
{
	if (m_btnPet && !iconPath.isEmpty())
	{
		m_btnPet->setIcon(QIcon(iconPath));
		m_btnPet->setIconSize(QSize(32, 32));
	}
}

void ActionMenu::loadStyleSheet()
{
    QFile file(":/style/menu.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        setStyleSheet(stream.readAll());
        file.close();
    }
}