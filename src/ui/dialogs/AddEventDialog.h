#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class AddEventDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddEventDialog(const QString& dateStr, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle(tr("Ajouter un rappel"));
        setFixedSize(300, 180);

        auto* layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel(tr("Date : %1").arg(dateStr), this));

        layout->addWidget(new QLabel(tr("Description :"), this));
        m_input = new QLineEdit(this);
        layout->addWidget(m_input);

        layout->addWidget(new QLabel(tr("Récurrence :"), this));
        m_recurrenceCombo = new QComboBox(this);
        m_recurrenceCombo->addItem(tr("Unique"), "none");
        m_recurrenceCombo->addItem(tr("Annuel (chaque année ce jour-là)"), "annual");
        layout->addWidget(m_recurrenceCombo);

        auto* btnSave = new QPushButton(tr("Enregistrer"), this);
        connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(btnSave);
    }

    QString text() const { return m_input->text().trimmed(); }
    QString recurrenceType() const { return m_recurrenceCombo->currentData().toString(); }

private:
    QLineEdit* m_input{ nullptr };
    QComboBox* m_recurrenceCombo{ nullptr };
};