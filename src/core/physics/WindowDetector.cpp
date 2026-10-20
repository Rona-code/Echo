#include "WindowDetector.h"
#include <QGuiApplication>

#include <QScreen>
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
#include <X11/Xatom.h>
#include <unistd.h>
#endif

#ifdef Q_OS_LINUX
namespace {
    bool hasAtomInList(Display* d, Window w, Atom prop, Atom target) {
        Atom type; int fmt; unsigned long n, after; unsigned char* data = nullptr;
        bool found = false;
        if (XGetWindowProperty(d, w, prop, 0, 1024, False, XA_ATOM,
            &type, &fmt, &n, &after, &data) == Success && data) {
            Atom* atoms = reinterpret_cast<Atom*>(data);
            for (unsigned long i = 0; i < n; ++i)
                if (atoms[i] == target) { found = true; break; }
            XFree(data);
        }
        return found;
    }
}
#endif

QList<QRect> WindowDetector::getAllVisibleWindowGeometries() {
    QList<QRect> windows;

#ifdef Q_OS_WIN
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

#elif defined(Q_OS_LINUX)
    Display* display = XOpenDisplay(nullptr);
    if (!display) return windows;

    Window root = DefaultRootWindow(display);
    Atom clientList = XInternAtom(display, "_NET_CLIENT_LIST", False);
    Atom pidAtom = XInternAtom(display, "_NET_WM_PID", False);
    Atom stateAtom = XInternAtom(display, "_NET_WM_STATE", False);
    Atom hiddenAtom = XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);
    Atom typeAtom = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom dockAtom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom desktopAtom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    Atom extentsAtom = XInternAtom(display, "_NET_FRAME_EXTENTS", False);
    Atom curDeskAtom = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
    Atom winDeskAtom = XInternAtom(display, "_NET_WM_DESKTOP", False);

    const unsigned long currentPid = static_cast<unsigned long>(getpid());
    const qreal dpr = qGuiApp && qGuiApp->primaryScreen()
        ? qGuiApp->primaryScreen()->devicePixelRatio() : 1.0;

    long currentDesktop = -1;
    {
        Atom t; int f; unsigned long n, a; unsigned char* d = nullptr;
        if (XGetWindowProperty(display, root, curDeskAtom, 0, 1, False, XA_CARDINAL,
            &t, &f, &n, &a, &d) == Success && d) {
            if (n > 0) currentDesktop = *reinterpret_cast<long*>(d);
            XFree(d);
        }
    }

    Atom type; int format; unsigned long nItems = 0, bytesAfter = 0;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(display, root, clientList, 0, 4096, False, XA_WINDOW,
        &type, &format, &nItems, &bytesAfter, &prop) == Success && prop) {
        Window* clients = reinterpret_cast<Window*>(prop);

        for (unsigned long i = 0; i < nItems; ++i) {
            Window w = clients[i];

            {
                Atom t; int f; unsigned long n, a; unsigned char* d = nullptr;
                if (XGetWindowProperty(display, w, pidAtom, 0, 1, False, XA_CARDINAL,
                    &t, &f, &n, &a, &d) == Success && d) {
                    unsigned long winPid = (n > 0) ? *reinterpret_cast<unsigned long*>(d) : 0;
                    XFree(d);
                    if (winPid == currentPid) continue;
                }
            }

            if (hasAtomInList(display, w, stateAtom, hiddenAtom)) continue;

            if (hasAtomInList(display, w, typeAtom, dockAtom) ||
                hasAtomInList(display, w, typeAtom, desktopAtom)) continue;

            {
                Atom t; int f; unsigned long n, a; unsigned char* d = nullptr;
                if (XGetWindowProperty(display, w, winDeskAtom, 0, 1, False, XA_CARDINAL,
                    &t, &f, &n, &a, &d) == Success && d) {
                    long desk = (n > 0) ? *reinterpret_cast<long*>(d) : -1;
                    XFree(d);
                    if (desk != static_cast<long>(0xFFFFFFFFul) && currentDesktop >= 0 && desk != currentDesktop)
                        continue;
                }
            }

            XWindowAttributes attr;
            if (!XGetWindowAttributes(display, w, &attr) || attr.map_state != IsViewable) continue;

            int x, y; Window child;
            if (!XTranslateCoordinates(display, w, root, 0, 0, &x, &y, &child)) continue;

            long left = 0, right = 0, top = 0, bottom = 0;
            {
                Atom t; int f; unsigned long n, a; unsigned char* d = nullptr;
                if (XGetWindowProperty(display, w, extentsAtom, 0, 4, False, XA_CARDINAL,
                    &t, &f, &n, &a, &d) == Success && d) {
                    if (n == 4) {
                        long* e = reinterpret_cast<long*>(d);
                        left = e[0]; right = e[1]; top = e[2]; bottom = e[3];
                    }
                    XFree(d);
                }
            }

            QRect r(qRound((x - left) / dpr),
                qRound((y - top) / dpr),
                qRound((attr.width + left + right) / dpr),
                qRound((attr.height + top + bottom) / dpr));

            if (r.width() > 100 && r.height() > 100)
                windows.append(r);
        }
        XFree(prop);
    }

    XCloseDisplay(display);
#endif

    return windows;
}