#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>

#include "../core/PetLoader.h"

class ActionMenu;

class Companion : public QWidget
{
    Q_OBJECT

public:
    explicit Companion(QWidget* parent = nullptr);
    ~Companion() override;

    float setScale() const { return m_scale; }

    void setScale(float newScale);
    void setSprite(const QString& imagePath);

    void setPetAnimationData(const PetAnimationData& data);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPixmap m_pixmap;
    QString m_currentImagePath;
    QPoint m_dragPosition;
    QPointer<ActionMenu> m_activeMenu = nullptr;

    float m_scale{ 0.5f };

    PetAnimationData m_currentPetData;
};