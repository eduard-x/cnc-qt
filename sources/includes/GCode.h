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


#ifndef GCODE_H
#define GCODE_H

#include <QString>
#include <QList>

#include <deque>
#include <utility>
#include "vec.h"


enum PlaneEnum {
    NonePlane,
    XY,
    YZ,
    ZX,
    UV,
    VW,
    WU
};

//
// g-code instruction
//
class GCodeData
{
    public:
        enum MovingType {
            NoType,
            Line,
            ArcCW,
            ArcCCW
        };

    public:
        bool changeInstrument; // to change the tool
        int  numberInstrument; // собственно номер tool

        //         bool   needPause;      // pause needed
        int  pauseMSeconds;  // if waiting = 0, no pause = -1. other pause in milliseconds
        //
        // coordinates in mm
        float X;
        float Y;
        float Z;
        //
        // angle in grad
        float A;

        // curve settings: G02, G03
        float I;
        float J;
        float K;

        PlaneEnum plane;

        int   vectSpeed; // telegr CA offset
        float vectorCoeff; // for the max from dH / dX of dH / dY ratio, in case XY plane

//         bool  changeDirection;
        int   accelCode;
        int   stepsCounter; // number of steps in current direction
        bool  feed; // true=G1 false=G0

        float Radius;
        // end of curves

        //         int   speed;          // speed
        bool  spindelON;      // spinle on
        //         int   numberInstruct; // g-code
        // normal is 1: one command is one cut
        // if line splitted, number of followed cuts.
        int   splits;
        int   numberLine;     // from g-code file

        MovingType typeMoving; // NONE, LINE, ARC_CW, ARC_CCW

        float diametr; // diameter of tool

        float angle; // angle between two lines around the actual point

        //
        // null constructor
        GCodeData();

        // Конструктор на основе существующей команды
        GCodeData(GCodeData *_cmd);
};



// struct GCodeData {
// };


class GCodeParser
{
    public:
        GCodeParser(); // constructor
        bool readGCode(const QByteArray &gcode);
        QStringList getGoodList();
        QStringList getBadList();

    private:
        float determineAngle(const Vec3 &pos, const Vec3 &pos_center, PlaneEnum pl);
        void convertArcToLines(GCodeData *endData);
        void calcAngleOfLines(int pos);
        bool parseArc(const QString &line, Vec3 &pos, float &R, const float coef);
        bool addLine(GCodeData* param);
        bool addArc(GCodeData* param);
        //         bool parseCoord(const QString &line, Vec3 &pos, float &E, const float coef, float *F);

        bool parseCoord(const QString &line, Vec3 &pos, float &E, const float coef, float *F = NULL);
        //         bool parseArc(const QString &line, Vec3 &pos, float &R, const float coef);
        //         bool addLine(GCodeData *c);
        //         bool addArc(GCodeData *c);
        //         void convertArcToLines(GCodeData *c);
        //         float determineAngle(const Vec3 &pos, const Vec3 &pos_c, PlaneEnum pl);
    public:
        QList<GCodeData> gCodeList;

    private:
        //         QList<GCodeData> gCodeList;
        QStringList goodList; // only decoded G-code
        QStringList badList;
};


#endif