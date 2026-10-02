#include "SettingsDialog.h"
#include <QHBoxLayout>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Paramètres"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    setupUI();
}

void SettingsDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Title
    auto* titleLabel = new QLabel(tr("Paramètres du Compagnon"), this);
    titleLabel->setObjectName("menuTitle");
    mainLayout->addWidget(titleLabel);

    // --- 1. SLIDER SCALING (Echelle du sprite : 0.05x à 3.00x) ---
    auto* scaleLayout = new QVBoxLayout();
    auto* scaleHeader = new QHBoxLayout();
    auto* scaleLabel = new QLabel(tr("Échelle du Compagnon :"), this);
    m_scaleValLabel = new QLabel("1.00x", this);
    scaleHeader->addWidget(scaleLabel);
    scaleHeader->addStretch();
    scaleHeader->addWidget(m_scaleValLabel);

    m_scaleSlider = new QSlider(Qt::Horizontal, this);
    // Plage d'entiers 5 à 300 pour représenter 0.05x à 3.00x (valeur / 100.0)
    m_scaleSlider->setRange(5, 300);
    m_scaleSlider->setValue(50); // Valeur par défaut : 0.50x

    scaleLayout->addLayout(scaleHeader);
    scaleLayout->addWidget(m_scaleSlider);
    mainLayout->addLayout(scaleLayout);

    // --- 2. SLIDER TIMING ANIMATION (Vitesse des frames : 100ms à 1000ms) ---
    auto* animLayout = new QVBoxLayout();
    auto* animHeader = new QHBoxLayout();
    auto* animLabel = new QLabel(tr("Vitesse d'animation :"), this);
    m_animSpeedValLabel = new QLabel("500 ms", this);
    animHeader->addWidget(animLabel);
    animHeader->addStretch();
    animHeader->addWidget(m_animSpeedValLabel);

    m_animSpeedSlider = new QSlider(Qt::Horizontal, this);
    m_animSpeedSlider->setRange(100, 1000); // intervalle en millisecondes
    m_animSpeedSlider->setValue(500);

    animLayout->addLayout(animHeader);
    animLayout->addWidget(m_animSpeedSlider);
    mainLayout->addLayout(animLayout);

    // --- 3. SLIDER TIMEOUT SLEEP (Inactivité avant dodo : 5s à 600s) ---
    auto* timeoutLayout = new QVBoxLayout();
    auto* timeoutHeader = new QHBoxLayout();
    auto* timeoutLabel = new QLabel(tr("Délai avant sommeil :"), this);
    m_idleTimeoutValLabel = new QLabel("15 s", this);
    timeoutHeader->addWidget(timeoutLabel);
    timeoutHeader->addStretch();
    timeoutHeader->addWidget(m_idleTimeoutValLabel);

    m_idleTimeoutSlider = new QSlider(Qt::Horizontal, this);
    m_idleTimeoutSlider->setRange(5, 600); // secondes
    m_idleTimeoutSlider->setValue(15);

    timeoutLayout->addLayout(timeoutHeader);
    timeoutLayout->addWidget(m_idleTimeoutSlider);
    mainLayout->addLayout(timeoutLayout);

    mainLayout->addStretch();

    // --- BOUTON DE VALIDATION ---
    m_btnSave = new QPushButton(tr("Enregistrer"), this);
    m_btnSave->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_btnSave);

    // --- CONNEXIONS POUR METTRE À JOUR LES LABELS EN DIRECT ---
    connect(m_scaleSlider, &QSlider::valueChanged, this, [this](int val) {
        m_scaleValLabel->setText(QString("%1x").arg(val / 100.0, 0, 'f', 2));
        });

    connect(m_animSpeedSlider, &QSlider::valueChanged, this, [this](int val) {
        m_animSpeedValLabel->setText(QString("%1 ms").arg(val));
        });

    connect(m_idleTimeoutSlider, &QSlider::valueChanged, this, [this](int val) {
        m_idleTimeoutValLabel->setText(QString("%1 s").arg(val));
        });

    connect(m_btnSave, &QPushButton::clicked, this, [this]() {
        emit settingsChanged(scaleFactor(), animSpeedMs(), idleTimeoutSec());
        accept();
        });
}

double SettingsDialog::scaleFactor() const {
    return m_scaleSlider->value() / 100.0;
}

int SettingsDialog::animSpeedMs() const {
    return m_animSpeedSlider->value();
}

int SettingsDialog::idleTimeoutSec() const {
    return m_idleTimeoutSlider->value();
}

void SettingsDialog::setValues(double scale, int animSpeed, int idleTimeout) {
    m_scaleSlider->setValue(static_cast<int>(scale * 100.0));
    m_animSpeedSlider->setValue(animSpeed);
    m_idleTimeoutSlider->setValue(idleTimeout);

    m_scaleValLabel->setText(QString("%1x").arg(scale, 0, 'f', 2));
    m_animSpeedValLabel->setText(QString("%1 ms").arg(animSpeed));
    m_idleTimeoutValLabel->setText(QString("%1 s").arg(idleTimeout));
}