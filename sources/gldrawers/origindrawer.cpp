/****************************************************************************
 * This file is a part of "grblControl" application.                        *
 * Copyright 2015 Hayrullin Denis Ravilevich                                *
 * https://github.com/Denvi/grblControl                                     *
 *                                                                          *
 * Qt, Linux developing                                                     *
 * Copyright (C) 2015-2018 by Eduard Kalinowski                             *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * C# project CNC-controller-for-mk1                                        *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
 *                                                                          *
 * The Qt project                                                           *
 * https://github.com/eduard-x/cnc-qt                                       *
 *                                                                          *
 * CNC-Qt is free software; may be distributed and/or modified under the    *
 * terms of the GNU General Public License version 3 as published by the    *
 * Free Software Foundation and appearing in the file LICENSE_GPLv3         *
 * included in the packaging of this file.                                  *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU Lesser General Public         *
 * License along with CNC-Qt. If not, see  http://www.gnu.org/licenses      *
 ****************************************************************************/

#include "origindrawer.h"

OriginDrawer::OriginDrawer()
{
}

bool OriginDrawer::updateData()
{
    m_lines = {
        // X-axis
        {QVector3D(0, 0, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(9, 0, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(10, 0, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(8, 0.5, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(8, 0.5, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(8, -0.5, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(8, -0.5, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(10, 0, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},

        // Y-axis
        {QVector3D(0, 0, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 9, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 10, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0.5, 8, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0.5, 8, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-0.5, 8, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-0.5, 8, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 10, 0), QColor(0.0, 1.0, 0.0), QVector3D(sNan, sNan, sNan)},

        // Z-axis
        {QVector3D(0, 0, 0), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 0, 9), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 0, 10), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0.5, 0, 8), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0.5, 0, 8), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-0.5, 0, 8), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-0.5, 0, 8), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(0, 0, 10), QColor(0.0, 0.0, 1.0), QVector3D(sNan, sNan, sNan)},

        // 2x2 rect
        {QVector3D(1, 1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-1, 1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-1, 1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-1, -1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(-1, -1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(1, -1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(1, -1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(1, 1, 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)}
    };
    return true;
}
