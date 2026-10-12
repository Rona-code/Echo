#include "WindowDetector.h"
#include <QGuiApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>

// Callback pour parcourir toutes les fenêtres sous Windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;

    // Ignorer les fenêtres enfants ou les outils invisibles
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_CHILD) return TRUE;

    RECT rect;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect)))) {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        if (width > 100 && height > 100) {
            auto* windowsList = reinterpret_cast<QList<QRect>*>(lParam);
            windowsList->append(QRect(rect.left, rect.top, width, height));
        }
    }
    return TRUE;
}

#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

QList<QRect> WindowDetector::getAllVisibleWindowGeometries() {
    QList<QRect> windows;

#ifdef Q_OS_WIN
    // Énumération de toutes les fenêtres top-level sous Windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

#elif defined(Q_OS_LINUX)
    // Énumération des fenêtres sous Linux (X11)
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        Window root = DefaultRootWindow(display);
        Window parent;
        Window* children = nullptr;
        unsigned int num_children = 0;

        if (XQueryTree(display, root, &root, &parent, &children, &num_children)) {
            for (unsigned int i = 0; i < num_children; ++i) {
                XWindowAttributes attr;
                if (XGetWindowAttributes(display, children[i], &attr)) {
                    // Vérifier si la fenêtre est affichée (Viewable)
                    if (attr.map_state == IsViewable) {
                        int x, y;
                        Window child;
                        // Traduire les coordonnées par rapport à la racine (Root window)
                        if (XTranslateCoordinates(display, children[i], root, 0, 0, &x, &y, &child)) {
                            if (attr.width > 100 && attr.height > 100) {
                                windows.append(QRect(x, y, attr.width, attr.height));
                            }
                        }
                    }
                }
            }
            if (children) {
                XFree(children);
            }
        }
        XCloseDisplay(display);
    }
#endif

    return windows;
}