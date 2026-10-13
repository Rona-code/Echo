#include "WindowDetector.h"
#include <QGuiApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <cmath>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_CHILD) return TRUE;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId()) return TRUE;

    int cloaked = 0;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) return TRUE;

    RECT rect;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect)))) {

        double scale = GetDpiForWindow(hwnd) / 96.0;
        if (scale <= 0.0) scale = 1.0;

        QRect r(qRound(rect.left / scale),
            qRound(rect.top / scale),
            qRound((rect.right - rect.left) / scale),
            qRound((rect.bottom - rect.top) / scale));

        if (r.width() > 100 && r.height() > 100) {
            auto* windowsList = reinterpret_cast<QList<QRect>*>(lParam);
            windowsList->append(r);
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
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

#elif defined(Q_OS_LINUX)
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
                    if (attr.map_state == IsViewable) {
                        int x, y;
                        Window child;
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