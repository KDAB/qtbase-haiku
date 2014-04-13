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

#include "qhaikurasterbackingstore.h"
#include "qhaikurasterwindow.h"

#include <View.h>

QT_BEGIN_NAMESPACE

QHaikuRasterBackingStore::QHaikuRasterBackingStore(QWindow *window)
    : QPlatformBackingStore(window)
    , m_window(window)
{
}

QHaikuRasterBackingStore::~QHaikuRasterBackingStore()
{
}

QPaintDevice *QHaikuRasterBackingStore::paintDevice()
{
    if (platformWindow() && platformWindow()->hasBuffers())
        return platformWindow()->renderBuffer().image();

    return 0;
}

void QHaikuRasterBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset)

    QHaikuWindow *targetWindow = 0;
    if (window)
        targetWindow = static_cast<QHaikuWindow *>(window->handle());

    if (!targetWindow || targetWindow == platformWindow()) {
        // update the display with newly rendered content
        platformWindow()->post(region);
    }
}

void QHaikuRasterBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
    Q_UNUSED(size);
    Q_UNUSED(staticContents);

    // NOTE: defer resizing window buffers until next paint as
    // resize() can be called multiple times before a paint occurs
}

void QHaikuRasterBackingStore::beginPaint(const QRegion &region)
{
    Q_UNUSED(region);

    platformWindow()->adjustBufferSize();
}

void QHaikuRasterBackingStore::endPaint()
{
}

QHaikuRasterWindow *QHaikuRasterBackingStore::platformWindow() const
{
    Q_ASSERT(m_window->handle());
    return static_cast<QHaikuRasterWindow*>(m_window->handle());
}

QT_END_NAMESPACE
