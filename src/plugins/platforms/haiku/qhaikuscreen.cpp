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

#include "qhaikuscreen.h"

#include "qhaikucursor.h"
#include "qhaikuutils.h"

#include <qpa/qwindowsysteminterface.h>

#include <Screen.h>

QHaikuScreen::QHaikuScreen()
    : m_screen(new BScreen(B_MAIN_SCREEN_ID))
    , m_cursor(new QHaikuCursor)
{
    Q_ASSERT(m_screen->IsValid());
}

QHaikuScreen::~QHaikuScreen()
{
    delete m_screen;
    m_screen = 0;

    delete m_cursor;
    m_cursor = 0;
}

QRect QHaikuScreen::geometry() const
{
    const BRect frame = m_screen->Frame();
    return QRect(frame.left, frame.top, frame.right - frame.left, frame.bottom - frame.top);
}

int QHaikuScreen::depth() const
{
    switch (format()) {
    case QImage::Format_Invalid:
        return 0;
        break;
    case QImage::Format_MonoLSB:
        return 1;
        break;
    case QImage::Format_Indexed8:
        return 8;
        break;
    case QImage::Format_RGB16:
    case QImage::Format_RGB555:
        return 16;
        break;
    case QImage::Format_RGB888:
        return 24;
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    default:
        return 32;
        break;
    }
}

QImage::Format QHaikuScreen::format() const
{
    return QHaikuUtils::colorSpaceToImageFormat(m_screen->ColorSpace());
}

QPlatformCursor *QHaikuScreen::cursor() const
{
    return m_cursor;
}

QT_END_NAMESPACE
