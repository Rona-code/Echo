#pragma once
#include <QRect>

class WindowDetector {
public:
    // Renvoie le QRect de la fenêtre au premier plan ou un QRect invalide si indisponible
    static QRect getTargetWindowGeometry();
};