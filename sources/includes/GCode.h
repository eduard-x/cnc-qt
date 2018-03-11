/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
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


#ifndef GCODE_H
#define GCODE_H

#include <QString>
#include <QMutex>
#include <QList>
#include <QObject>
#include <QVector>
#include <QVector3D>

// #include <deque>
// #include <utility>
// #include "vec.h"


enum PlaneEnum {
    None = 0,
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
        int  numberInstruction; // set it, if instuction was sent

        int  pauseMSeconds;  // if waiting = 0, no pause = -1. other pause in milliseconds
        //
        // coordinates in mm
        QVector3D xyz;
        QVector3D abc;
        QVector3D ijk;
        QVector3D uvw;
        //         float axis[16];
#if 0
        float X;
        float Y;
        float Z;
        //
        // angle in grad
        float A;
        float B;
        float C;

        // curve settings: G02, G03
        float I;
        float J;
        float K;

        float U;
        float V;
        float W;
#endif
        PlaneEnum plane;

        int   vectSpeed; // telegr CA offset
        float vectorCoeff; // for the max from dH / dX of dH / dY ratio, in case XY plane

        float feedVelocity;

        int   movingCode;
        int   stepsCounter; // number of steps in current direction

        float Radius;
        // end of curves

        bool  spindelON;      // spinle on
        // normal is 1: one command is one cut
        // if line splitted, number of followed cuts.
        int   splits;
        int   numberLine;     // from g-code file

        MovingType typeMoving; // NONE, LINE, ARC_CW, ARC_CCW

        float diametr; // diameter of tool

        float angle; // angle between two lines around the actual point
        float deltaAngle;

        //
        // null constructor
        GCodeData();

        // Конструктор на основе существующей команды
        GCodeData(GCodeData *_cmd);
};


/**
 * for ant sorting function
 */
struct GCodeOptim {
    QVector3D coord;
    int lineBeg;
    int lineEnd;
    int gcodeBeg;
    int gcodeEnd;
};


class GCodeParser : public QObject
{
        Q_OBJECT
    public:
        GCodeParser(); // constructor
        ~GCodeParser(); // destructor

        bool readGCode(const QByteArray &gcode);
        QVector<QString> getGoodList();
        QVector<GCodeData> getGCodeData();
        QVector <GCodeOptim> getRapidPoints();

    private:
        float determineAngle(const QVector3D &pos, const QVector3D &pos_center, PlaneEnum pl);
        void convertArcToLines(GCodeData *endData);
        void calcAngleOfLines(int pos);
        bool parseArc(const QString &line, QVector3D &pos, float &R, const float coef);
        bool addLine(GCodeData* param);
        bool addArc(GCodeData* param);
        void detectMinMax(const GCodeData &d);
        bool parseCoord(const QString &line, QVector3D &pos, float &E, const float coef, float *F = NULL);

    protected:
        void sortGCode(const QVector<int> &antdata);

    signals:
        void logMessage(const QString &l);

    protected:
        QMutex mut;
        QVector<GCodeData> gCodeList;
        QVector<GCodeOptim> g0Points;
        QVector<QString> goodList; // only decoded G-code
        //         QVector<QString> badList;
};


#endif
