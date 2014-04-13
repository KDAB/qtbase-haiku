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

#ifndef QHAIKURASTERWINDOW_H
#define QHAIKURASTERWINDOW_H

#include "qhaikuwindow.h"

#include <View.h>

QT_BEGIN_NAMESPACE

class HaikuViewProxy : public QObject, public BView
{
    Q_OBJECT

public:
    explicit HaikuViewProxy(BWindow *window, QObject *parent = 0);

    void MessageReceived(BMessage *message) Q_DECL_OVERRIDE;
    void Draw(BRect updateRect) Q_DECL_OVERRIDE;
    void MouseDown(BPoint pos) Q_DECL_OVERRIDE;
    void MouseUp(BPoint pos) Q_DECL_OVERRIDE;
    void MouseMoved(BPoint pos, uint32 code, const BMessage *dragMessage) Q_DECL_OVERRIDE;
    void KeyDown(const char *bytes, int32 numBytes) Q_DECL_OVERRIDE;
    void KeyUp(const char *bytes, int32 numBytes) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void mouseEvent(const QPoint &localPosition, const QPoint &globalPosition, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source);
    void wheelEvent(const QPoint &localPosition, const QPoint &globalPosition, int delta, Qt::Orientation orientation, Qt::KeyboardModifiers modifiers);
    void keyEvent(QEvent::Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text);
    void enteredView();
    void exitedView();
    void drawRequest(const QRect &rect);

private:
    Qt::MouseButtons keyStateToMouseButtons(uint32 keyState) const;
    Qt::KeyboardModifiers keyStateToModifiers(uint32 keyState) const;
    void handleKeyEvent(QEvent::Type type, BMessage *message);
};

class QHaikuRasterWindow : public QHaikuWindow
{
    Q_OBJECT

public:
    explicit QHaikuRasterWindow(QWindow *window);
    ~QHaikuRasterWindow();

    BView* nativeViewHandle() const;

private Q_SLOTS:
    void haikuMouseEvent(const QPoint &localPosition, const QPoint &globalPosition, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source);
    void haikuWheelEvent(const QPoint &localPosition, const QPoint &globalPosition, int delta, Qt::Orientation orientation, Qt::KeyboardModifiers modifiers);
    void haikuKeyEvent(QEvent::Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text);
    void haikuEnteredView();
    void haikuExitedView();
    void haikuDrawRequest(const QRect &rect);

private:
    HaikuViewProxy *m_view;
};

QT_END_NAMESPACE

#endif
