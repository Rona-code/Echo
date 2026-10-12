#pragma once
#include <QRect>
#include <QList>

class WindowDetector {
public:
    static QList<QRect> getAllVisibleWindowGeometries();
};