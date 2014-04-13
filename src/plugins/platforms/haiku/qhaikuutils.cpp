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

#include "qhaikuutils.h"

color_space QHaikuUtils::imageFormatToColorSpace(QImage::Format format)
{
    color_space colorSpace = B_NO_COLOR_SPACE;
    switch (format) {
    case QImage::Format_Invalid:
        colorSpace = B_NO_COLOR_SPACE;
        break;
    case QImage::Format_MonoLSB:
        colorSpace = B_GRAY1;
        break;
    case QImage::Format_Indexed8:
        colorSpace = B_CMAP8;
        break;
    case QImage::Format_RGB32:
        colorSpace = B_RGB32;
        break;
    case QImage::Format_ARGB32:
        colorSpace = B_RGBA32;
        break;
    case QImage::Format_RGB16:
        colorSpace = B_RGB16;
        break;
    case QImage::Format_RGB555:
        colorSpace = B_RGB15;
        break;
    case QImage::Format_RGB888:
        colorSpace = B_RGB24;
        break;
    default:
        qWarning("Cannot convert image format %d to color space", format);
        Q_ASSERT(false);
        break;
    }

    return colorSpace;
}

QImage::Format QHaikuUtils::colorSpaceToImageFormat(color_space colorSpace)
{
    QImage::Format format = QImage::Format_Invalid;
    switch (colorSpace) {
    case B_NO_COLOR_SPACE:
        format = QImage::Format_Invalid;
        break;
    case B_GRAY1:
        format = QImage::Format_MonoLSB;
        break;
    case B_CMAP8:
        format = QImage::Format_Indexed8;
        break;
    case B_RGB32:
        format = QImage::Format_RGB32;
        break;
    case B_RGBA32:
        format = QImage::Format_ARGB32;
        break;
    case B_RGB16:
        format = QImage::Format_RGB16;
        break;
    case B_RGB15:
        format = QImage::Format_RGB555;
        break;
    case B_RGB24:
        format = QImage::Format_RGB888;
        break;
    default:
        qWarning("Cannot convert color space %d to image format", colorSpace);
        Q_ASSERT(false);
        break;
    }

    return format;
}

QT_END_NAMESPACE
