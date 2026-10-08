#include "WindowDetector.h"

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

QRect WindowDetector::getTargetWindowGeometry() {
#ifdef Q_OS_WIN
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        RECT rect;
        if (GetWindowRect(hwnd, &rect)) {
            return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
        }
    }
#elif defined(Q_OS_LINUX)
    Display* display = XOpenDisplay(NULL);
    if (display) {
        Window focused;
        int revert_to;
        XGetInputFocus(display, &focused, &revert_to);
        if (focused != None) {
            XWindowAttributes attr;
            if (XGetWindowAttributes(display, focused, &attr)) {
                int x, y;
                Window child;
                XTranslateCoordinates(display, focused, DefaultRootWindow(display), 0, 0, &x, &y, &child);
                XCloseDisplay(display);
                return QRect(x, y, attr.width, attr.height);
            }
        }
        XCloseDisplay(display);
    }
#endif
    return QRect(); // Retourne un QRect invalide (fallback écran)
}