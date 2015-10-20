/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2016 by Eduard Kalinowski                             *
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


#include "vec.h"


struct dPoint {
    float X;       // coord in mm
    float Y;       // coord in mm
    float Z;       // coord in mm
    float A;       // angle in °
};


class Geometry
{
    public:
        static dPoint GetZ(dPoint p1, dPoint p2, dPoint p3, dPoint p4, dPoint p5);
        static dPoint CalcPX(dPoint p1, dPoint p2, dPoint p0);
        static dPoint CalcPY(dPoint p1, dPoint p2, dPoint p0);

        float CubicHermite (Vec4f &v , float t);
        float BicubicHermitePatch(Vec4x4f &vv, float u, float v);
        bool gernerateBicubicHermiteField();
};



#endif // GEOMETRY_H
