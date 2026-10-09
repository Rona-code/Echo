#pragma once

#include <QPoint>

class Companion;
class ActionMenu;

class CompanionMenuManager {
public:
    static ActionMenu* showContextMenu(Companion* companion, const QPoint& globalPos);
};