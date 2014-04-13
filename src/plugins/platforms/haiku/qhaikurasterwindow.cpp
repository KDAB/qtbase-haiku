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

#include "qhaikurasterwindow.h"

#include "qhaikukeymapper.h"

#include <Bitmap.h>
#include <View.h>
#include <Window.h>

#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_METATYPE(QEvent::Type)
Q_DECLARE_METATYPE(Qt::MouseButtons)
Q_DECLARE_METATYPE(Qt::MouseEventSource)
Q_DECLARE_METATYPE(Qt::KeyboardModifiers)
Q_DECLARE_METATYPE(Qt::Orientation)

HaikuViewProxy::HaikuViewProxy(BWindow *window, QObject *parent)
    : QObject(parent)
    , BView(BRect(0, 0, window->Bounds().right, window->Bounds().bottom), 0, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
}

void HaikuViewProxy::MessageReceived(BMessage *message)
{
    switch (message->what) {
    case B_MOUSE_WHEEL_CHANGED:
        {
             float deltaX = 0;
             if (message->FindFloat("be:wheel_delta_x", &deltaX) != B_OK)
                 deltaX = 0;

             float deltaY = 0;
             if (message->FindFloat("be:wheel_delta_y", &deltaY) != B_OK)
                 deltaY = 0;

             if (deltaX != 0 || deltaY != 0) {
                BPoint localPos;
                uint32 keyState = 0;
                GetMouse(&localPos, &keyState);
                const Qt::KeyboardModifiers keyboardModifiers = keyStateToModifiers(modifiers());

                const BPoint globalPos = ConvertToScreen(localPos);
                const QPoint globalPosition = QPoint(globalPos.x, globalPos.y);
                const QPoint localPosition = QPoint(localPos.x, localPos.y);

                if (deltaX != 0)
                    Q_EMIT wheelEvent(localPosition, globalPosition, (deltaX * -120), Qt::Horizontal, keyboardModifiers);

                if (deltaY != 0)
                    Q_EMIT wheelEvent(localPosition, globalPosition, (deltaY * -120), Qt::Vertical, keyboardModifiers);
             }
             break;
        }
    default:
        BView::MessageReceived(message);
        break;
    }

}

void HaikuViewProxy::Draw(BRect updateRect)
{
    BView::Draw(updateRect);

    Q_EMIT drawRequest(QRect(updateRect.left, updateRect.top, updateRect.Width(), updateRect.Height()));
}

void HaikuViewProxy::MouseDown(BPoint localPos)
{
    BPoint dummyPos;
    uint32 keyState = 0;
    GetMouse(&dummyPos, &keyState);

    const Qt::MouseButtons mouseButtons = keyStateToMouseButtons(keyState);
    const Qt::KeyboardModifiers keyboardModifiers = keyStateToModifiers(modifiers());
    const Qt::MouseEventSource source = Qt::MouseEventNotSynthesized;

    const BPoint globalPos = ConvertToScreen(localPos);
    const QPoint globalPosition = QPoint(globalPos.x, globalPos.y);
    const QPoint localPosition = QPoint(localPos.x, localPos.y);

    Q_EMIT mouseEvent(localPosition, globalPosition, mouseButtons, keyboardModifiers, source);
}

void HaikuViewProxy::MouseUp(BPoint localPos)
{
    BPoint dummyPos;
    uint32 keyState = 0;
    GetMouse(&dummyPos, &keyState);

    const Qt::MouseButtons mouseButtons = keyStateToMouseButtons(keyState);
    const Qt::KeyboardModifiers keyboardModifiers = keyStateToModifiers(modifiers());
    const Qt::MouseEventSource source = Qt::MouseEventNotSynthesized;

    const BPoint globalPos = ConvertToScreen(localPos);
    const QPoint globalPosition = QPoint(globalPos.x, globalPos.y);
    const QPoint localPosition = QPoint(localPos.x, localPos.y);

    Q_EMIT mouseEvent(localPosition, globalPosition, mouseButtons, keyboardModifiers, source);
}

void HaikuViewProxy::MouseMoved(BPoint pos, uint32 code, const BMessage *dragMessage)
{
    switch (code) {
    case B_INSIDE_VIEW:
        {
            BPoint dummyPos;
            uint32 keyState = 0;
            GetMouse(&dummyPos, &keyState);

            const Qt::MouseButtons mouseButtons = keyStateToMouseButtons(keyState);
            const Qt::KeyboardModifiers keyboardModifiers = keyStateToModifiers(modifiers());
            const Qt::MouseEventSource source = Qt::MouseEventNotSynthesized;

            const BPoint globalPos = ConvertToScreen(pos);
            const QPoint globalPosition = QPoint(globalPos.x, globalPos.y);
            const QPoint localPosition = QPoint(pos.x, pos.y);

            Q_EMIT mouseEvent(localPosition, globalPosition, mouseButtons, keyboardModifiers, source);
        }
        break;
    case B_ENTERED_VIEW:
        Q_EMIT enteredView();
        break;
    case B_EXITED_VIEW:
        Q_EMIT exitedView();
        break;
    }

    BView::MouseMoved(pos, code, dragMessage);
}

void HaikuViewProxy::KeyDown(const char*, int32)
{
    handleKeyEvent(QEvent::KeyPress, Window()->CurrentMessage());
}

void HaikuViewProxy::KeyUp(const char*, int32)
{
    handleKeyEvent(QEvent::KeyRelease, Window()->CurrentMessage());
}

Qt::MouseButtons HaikuViewProxy::keyStateToMouseButtons(uint32 keyState) const
{
    Qt::MouseButtons mouseButtons(Qt::NoButton);
    if (keyState & B_PRIMARY_MOUSE_BUTTON)
        mouseButtons |= Qt::LeftButton;
    if (keyState & B_SECONDARY_MOUSE_BUTTON)
        mouseButtons |= Qt::RightButton;
    if (keyState & B_TERTIARY_MOUSE_BUTTON)
        mouseButtons |= Qt::MiddleButton;

    return mouseButtons;
}

Qt::KeyboardModifiers HaikuViewProxy::keyStateToModifiers(uint32 keyState) const
{
    Qt::KeyboardModifiers modifiers(Qt::NoModifier);

    if (keyState & B_SHIFT_KEY)
        modifiers |= Qt::ShiftModifier;
    if (keyState & B_CONTROL_KEY)
        modifiers |= Qt::AltModifier;
    if (keyState & B_COMMAND_KEY)
        modifiers |= Qt::ControlModifier;

    return modifiers;
}

void HaikuViewProxy::handleKeyEvent(QEvent::Type type, BMessage *message)
{
    int32 key = 0;
    uint32 code = 0;
    const char *bytes = 0;
    QString text;

    if (message) {
        if (message->FindString("bytes", &bytes) == B_OK) {
            text = QString::fromLocal8Bit(bytes, strlen(bytes));
        }

        if (message->FindInt32("key", &key) == B_OK) {
            code = QHaikuKeyMapper::translateKeyCode(key, (modifiers() & B_NUM_LOCK));
        }
    }

    const Qt::KeyboardModifiers keyboardModifiers = keyStateToModifiers(modifiers());

    Q_EMIT keyEvent(type, code, keyboardModifiers, text);
}


QHaikuRasterWindow::QHaikuRasterWindow(QWindow *window)
    : QHaikuWindow(window)
    , m_bitmap(0)
{
    qRegisterMetaType<QEvent::Type>();
    qRegisterMetaType<Qt::MouseButtons>();
    qRegisterMetaType<Qt::MouseEventSource>();
    qRegisterMetaType<Qt::KeyboardModifiers>();
    qRegisterMetaType<Qt::Orientation>();

    HaikuViewProxy *haikuView = new HaikuViewProxy(m_window);
    connect(haikuView, SIGNAL(mouseEvent(QPoint,QPoint,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::MouseEventSource)),
            this, SLOT(haikuMouseEvent(QPoint,QPoint,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::MouseEventSource)));
    connect(haikuView, SIGNAL(wheelEvent(QPoint,QPoint,int,Qt::Orientation,Qt::KeyboardModifiers)),
            this, SLOT(haikuWheelEvent(QPoint,QPoint,int,Qt::Orientation,Qt::KeyboardModifiers)));
    connect(haikuView, SIGNAL(keyEvent(QEvent::Type,int,Qt::KeyboardModifiers,QString)),
            this, SLOT(haikuKeyEvent(QEvent::Type,int,Qt::KeyboardModifiers,QString)));
    connect(haikuView, SIGNAL(enteredView()), this, SLOT(haikuEnteredView()));
    connect(haikuView, SIGNAL(exitedView()), this, SLOT(haikuExitedView()));
    connect(haikuView, SIGNAL(drawRequest(QRect)), SLOT(haikuDrawRequest(QRect)));

    m_view = haikuView;

    m_window->LockLooper();
    m_window->AddChild(m_view);
    m_view->MakeFocus();
    m_window->UnlockLooper();
}

QHaikuRasterWindow::~QHaikuRasterWindow()
{
    delete m_bitmap;
    m_bitmap = 0;

    m_window->LockLooper();
    m_view->RemoveSelf();
    m_window->UnlockLooper();

    delete m_view;
    m_view = 0;
}

void QHaikuRasterWindow::post(const QRegion &dirty)
{
    Q_UNUSED(dirty);

    m_view->LockLooper();
    m_view->DrawBitmap(m_bitmap);
    m_view->UnlockLooper();
}

QHaikuBuffer &QHaikuRasterWindow::renderBuffer()
{
    return m_buffer;
}

bool QHaikuRasterWindow::hasBuffers() const
{
    return !bufferSize().isEmpty();
}

void QHaikuRasterWindow::adjustBufferSize()
{
    if (window()->size() != bufferSize())
        setBufferSize(window()->size());
}

void QHaikuRasterWindow::setBufferSize(const QSize &size)
{
    if (m_bitmap)
        delete m_bitmap;

    m_bitmap = new BBitmap(BRect(0, 0, size.width(), size.height()), B_RGB32, false, true);
    m_buffer = QHaikuBuffer(m_bitmap);
    m_bufferSize = size;
}

QSize QHaikuRasterWindow::bufferSize() const
{
    return m_bufferSize;
}

void QHaikuRasterWindow::haikuMouseEvent(const QPoint &localPosition, const QPoint &globalPosition, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source)
{
    QWindowSystemInterface::handleMouseEvent(window(), localPosition, globalPosition,
                                             buttons, modifiers, source);
}

void QHaikuRasterWindow::haikuWheelEvent(const QPoint &localPosition, const QPoint &globalPosition, int delta, Qt::Orientation orientation, Qt::KeyboardModifiers modifiers)
{
    QWindowSystemInterface::handleWheelEvent(window(), localPosition, globalPosition, delta, orientation, modifiers);
}

void QHaikuRasterWindow::haikuKeyEvent(QEvent::Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text)
{
    QWindowSystemInterface::handleKeyEvent(window(), type, key, modifiers, text);
}

void QHaikuRasterWindow::haikuEnteredView()
{
    QWindowSystemInterface::handleEnterEvent(window());
}

void QHaikuRasterWindow::haikuExitedView()
{
    QWindowSystemInterface::handleLeaveEvent(window());
}

void QHaikuRasterWindow::haikuDrawRequest(const QRect &rect)
{
    post(QRegion(rect));
}

QT_END_NAMESPACE
