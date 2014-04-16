/***************************************************************************
**
** Copyright (C) 2013 - 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias Koenig <tobias.koenig@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhaikuwindow.h"

#include "private/qguiapplication_p.h"

#include <QCoreApplication>
#include <QThread>
#include <QWindow>
#include <qpa/qwindowsysteminterface.h>

#include <Rect.h>
#include <Window.h>

QT_BEGIN_NAMESPACE

enum {
    DefaultWindowWidth = 160,
    DefaultWindowHeight = 160
};

HaikuWindowProxy::HaikuWindowProxy(QWindow *window, const QRect &rect, QObject *parent)
    : QObject(parent)
    , BWindow(BRect(rect.x(), rect.y(), rect.right() - 1, rect.bottom() - 1),
              window->title().toUtf8(), B_NO_BORDER_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0)
    , m_qtCalledZoom(false)
    , m_zoomInProgress(false)
{
}

void HaikuWindowProxy::FrameMoved(BPoint pos)
{
    Q_EMIT moved(QPoint(pos.x, pos.y));
}

void HaikuWindowProxy::FrameResized(float width, float height)
{
    Q_EMIT resized(QSize(static_cast<int>(width), static_cast<int>(height)), m_zoomInProgress);

    m_zoomInProgress = false;
}

void HaikuWindowProxy::WindowActivated(bool activated)
{
    Q_EMIT windowActivated(activated);
}

void HaikuWindowProxy::Minimize(bool minimize)
{
    BWindow::Minimize(minimize);

    Q_EMIT minimized(minimize);
}

void HaikuWindowProxy::Zoom(BPoint pos, float width, float height)
{
    m_zoomInProgress = true;
    BWindow::Zoom(pos, width, height);

    // Only notify about Zoom invocations from the Haiku windowing system
    if (!m_qtCalledZoom)
        Q_EMIT zoomed();
}

bool HaikuWindowProxy::QuitRequested()
{
    Q_EMIT quitRequested();

    // Return false to prevent Haiku windowing system to clean up
    // the BWindow and BView instances. We will do that ourself when
    // Qt invokes the dtor of QHaikuWindow
    return false;
}

void HaikuWindowProxy::zoomByQt()
{
    m_qtCalledZoom = true;
    BWindow::Zoom();
    m_qtCalledZoom = false;
}

QHaikuWindow::QHaikuWindow(QWindow *window)
    : QPlatformWindow(window)
    , m_window(0)
    , m_windowState(Qt::WindowNoState)
{
    const QRect rect = initialGeometry(window, window->geometry(), DefaultWindowWidth, DefaultWindowHeight);

    HaikuWindowProxy *haikuWindow = new HaikuWindowProxy(window, rect, 0);
    connect(haikuWindow, SIGNAL(moved(QPoint)), SLOT(haikuWindowMoved(QPoint)));
    connect(haikuWindow, SIGNAL(resized(QSize,bool)), SLOT(haikuWindowResized(QSize,bool)));
    connect(haikuWindow, SIGNAL(windowActivated(bool)), SLOT(haikuWindowActivated(bool)));
    connect(haikuWindow, SIGNAL(minimized(bool)), SLOT(haikuWindowMinimized(bool)));
    connect(haikuWindow, SIGNAL(zoomed()), SLOT(haikuWindowZoomed()));
    connect(haikuWindow, SIGNAL(quitRequested()), SLOT(haikuWindowQuitRequested()), Qt::BlockingQueuedConnection);

    m_window = haikuWindow;

    if (!m_window)
        qFatal("QHaikuWindow: failed to create window");

    setWindowFlags(window->flags());
}

QHaikuWindow::~QHaikuWindow()
{
    m_window->LockLooper();
    m_window->Quit();

    m_window = 0;
}

void QHaikuWindow::setGeometry(const QRect &rect)
{
    QPlatformWindow::setGeometry(rect);

    m_window->MoveTo(rect.x(), rect.y());
    m_window->ResizeTo(rect.width(), rect.height());
}

void QHaikuWindow::setVisible(bool visible)
{
    if (visible) {
        m_window->Show();
    } else {
        m_window->Hide();
    }

    window()->requestActivate();

    QWindowSystemInterface::handleExposeEvent(window(), window()->geometry());
/*
    if (visible) {
        applyWindowState();
    }
*/
}

bool QHaikuWindow::isExposed() const
{
    return !m_window->IsHidden();
}

WId QHaikuWindow::winId() const
{
    return (WId)static_cast<BWindow*>(m_window);
}

BWindow* QHaikuWindow::nativeHandle() const
{
    return m_window;
}

void QHaikuWindow::requestActivateWindow()
{
    m_window->Activate(true);
}


void QHaikuWindow::setWindowState(Qt::WindowState state)
{
    if (m_windowState == state)
        return;

    const Qt::WindowState oldState = m_windowState;

    m_windowState = state;

    if (m_windowState == Qt::WindowMaximized) {
        m_window->zoomByQt();
    } else if (m_windowState == Qt::WindowMinimized) {
        m_window->Minimize(true);
    } else if (m_windowState == Qt::WindowNoState) {
        if (oldState == Qt::WindowMaximized)
            m_window->zoomByQt(); // undo zoom

        if (oldState == Qt::WindowMinimized)
            m_window->Minimize(false); // undo minimize
    }
}

void QHaikuWindow::setWindowFlags(Qt::WindowFlags flags)
{
    const Qt::WindowType type = static_cast<Qt::WindowType>(static_cast<int>(flags & Qt::WindowType_Mask));

    const bool isPopup = (type == Qt::Popup);
    const bool isSplashScreen = (type == Qt::SplashScreen);
    const bool isDialog = ((type == Qt::Dialog) || (type == Qt::Sheet) || (type == Qt::MSWindowsFixedSizeDialogHint));
    const bool isTool = ((type == Qt::Tool) || (type == Qt::Drawer));
    const bool isToolTip = (type == Qt::ToolTip);

    window_look wlook = B_TITLED_WINDOW_LOOK;
    window_feel wfeel = B_NORMAL_WINDOW_FEEL;
    uint32 wflag = (B_NO_WORKSPACE_ACTIVATION | B_NOT_ANCHORED_ON_ACTIVATE);

    if (isTool) {
        wlook = B_FLOATING_WINDOW_LOOK;
        wflag |= B_WILL_ACCEPT_FIRST_CLICK;
    }

    if (isSplashScreen) {
        wlook = B_NO_BORDER_WINDOW_LOOK;
    }

    if (isPopup) {
        wlook = B_NO_BORDER_WINDOW_LOOK;
        wflag |= (B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FRONT | B_AVOID_FOCUS);
        flags |= Qt::WindowStaysOnTopHint;
    }

    if (isDialog) {
        if (window()->modality() == Qt::WindowModal)
            wfeel = B_MODAL_SUBSET_WINDOW_FEEL;
        else if (window()->modality() == Qt::ApplicationModal)
            wfeel = B_MODAL_APP_WINDOW_FEEL;
    }

    if (isToolTip) {
        wlook = B_NO_BORDER_WINDOW_LOOK;
        wflag |= (B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FOCUS);
        flags |= Qt::WindowStaysOnTopHint;
    }

    if (flags & Qt::FramelessWindowHint)
        wlook = B_NO_BORDER_WINDOW_LOOK;

    if (flags & Qt::MSWindowsFixedSizeDialogHint)
        wflag |= (B_NOT_RESIZABLE | B_NOT_ZOOMABLE);

    if (flags & Qt::CustomizeWindowHint) {
        if (!(flags & Qt::WindowMinimizeButtonHint))
            wflag |= B_NOT_MINIMIZABLE;
        if (!(flags & Qt::WindowMaximizeButtonHint))
            wflag |= B_NOT_ZOOMABLE;
        if (!(flags & Qt::WindowCloseButtonHint))
            wflag |= B_NOT_CLOSABLE;
    }

    if (flags & Qt::WindowStaysOnTopHint)
        wfeel = B_FLOATING_ALL_WINDOW_FEEL;

    m_window->SetLook(wlook);
    m_window->SetFeel(wfeel);
    m_window->SetFlags(wflag);
}

void QHaikuWindow::propagateSizeHints()
{
    m_window->SetSizeLimits(window()->minimumSize().width(),
                            window()->maximumSize().width(),
                            window()->minimumSize().height(),
                            window()->maximumSize().height());

    m_window->SetZoomLimits(window()->maximumSize().width(),
                            window()->maximumSize().height());
}

void QHaikuWindow::haikuWindowMoved(const QPoint &pos)
{
    const QRect newGeometry(pos, geometry().size());

    QPlatformWindow::setGeometry(newGeometry);
    QWindowSystemInterface::setSynchronousWindowsSystemEvents(true);
    QWindowSystemInterface::handleGeometryChange(window(), newGeometry);
    QWindowSystemInterface::handleExposeEvent(window(), newGeometry);
    QWindowSystemInterface::setSynchronousWindowsSystemEvents(false);
}

void QHaikuWindow::haikuWindowResized(const QSize &size, bool zoomInProgress)
{
    const QRect newGeometry(geometry().topLeft(), size);

    QPlatformWindow::setGeometry(newGeometry);
    QWindowSystemInterface::setSynchronousWindowsSystemEvents(true);
    QWindowSystemInterface::handleGeometryChange(window(), newGeometry);
    QWindowSystemInterface::handleExposeEvent(window(), newGeometry);
    QWindowSystemInterface::setSynchronousWindowsSystemEvents(false);

    if ((m_windowState == Qt::WindowMaximized) && !zoomInProgress) {
        // the user has resized the window while maximized -> reset maximized flag
        m_windowState = Qt::WindowNoState;

        QWindowSystemInterface::handleWindowStateChanged(window(), m_windowState);
    }
}

void QHaikuWindow::haikuWindowActivated(bool activated)
{
    QWindowSystemInterface::handleWindowActivated(activated ? window() : 0);
}

void QHaikuWindow::haikuWindowMinimized(bool minimize)
{
    m_windowState = (minimize ? Qt::WindowMinimized : Qt::WindowNoState);

    QWindowSystemInterface::handleWindowStateChanged(window(), m_windowState);
}

void QHaikuWindow::haikuWindowZoomed()
{
    m_windowState = (m_windowState == Qt::WindowMaximized ? Qt::WindowNoState : Qt::WindowMaximized);

    QWindowSystemInterface::handleWindowStateChanged(window(), m_windowState);
}

void QHaikuWindow::haikuWindowQuitRequested()
{
    QWindowSystemInterface::handleCloseEvent(window());
}

void QHaikuWindow::haikuMouseEvent(const QPoint &localPosition, const QPoint &globalPosition, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source)
{
    QWindowSystemInterface::handleMouseEvent(window(), localPosition, globalPosition,
                                             buttons, modifiers, source);
}

void QHaikuWindow::haikuWheelEvent(const QPoint &localPosition, const QPoint &globalPosition, int delta, Qt::Orientation orientation, Qt::KeyboardModifiers modifiers)
{
    QWindowSystemInterface::handleWheelEvent(window(), localPosition, globalPosition, delta, orientation, modifiers);
}

void QHaikuWindow::haikuKeyEvent(QEvent::Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text)
{
    QWindowSystemInterface::handleKeyEvent(window(), type, key, modifiers, text);
}

void QHaikuWindow::haikuEnteredView()
{
    QWindowSystemInterface::handleEnterEvent(window());
}

void QHaikuWindow::haikuExitedView()
{
    QWindowSystemInterface::handleLeaveEvent(window());
}

void QHaikuWindow::haikuDrawRequest(const QRect &rect)
{
    QWindowSystemInterface::handleExposeEvent(window(), QRegion(rect));
}

QT_END_NAMESPACE
