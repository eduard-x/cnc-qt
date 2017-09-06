/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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


#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QVector4D>
#include <QMatrix4x4>
#include <QAbstractTableModel>
// #include "vec.h"


// struct dPoint {
//   float X;       // coord in mm
//     float Y;       // coord in mm
//     float Z;       // coord in mm
//     float A;       // angle in °
// };

struct coord {
    float pos[4];
};


class Geometry
{
    public:
        static coord GetZ(const coord &p1, const coord &p2, const coord &p3, const coord &p4, const coord &p5);
        static coord CalcPX(const coord &p1, const coord &p2, const coord &p0);
        static coord CalcPY(const coord &p1, const coord &p2, const coord &p0);

        float cubicHermiteInterpolate (const float v[4], float t);
        float bicubicHermitePatch(const float vv[4][4], float u, float v);
        float bicubicInterpolate(QRectF borderRect, QAbstractTableModel *basePoints, float x, float y);
        bool gernerateBicubicHermiteField();

};



#endif // GEOMETRY_H
