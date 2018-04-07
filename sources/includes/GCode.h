/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
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


#ifndef GCODE_H
#define GCODE_H

#include <QString>
#include <QMutex>
#include <QList>
#include <QObject>
#include <QVector>
#include <QVector3D>

// #include "Reader.h"

// #include <deque>
// #include <utility>
// #include "vec.h"

/* Externalize variables used by the scanner and parser. */



extern FILE * gcode_in;
void gcode_restart (FILE *);

__BEGIN_DECLS

/* Available functions of the checker. */
int gcode_parse (void);
int gcode_error (const char *);
int gcode_lex (void);
int gcode_lex_destroy (void);
// int gcode_check (qucs::dataset *);

// static int gcode_lineno = 0;

__END_DECLS

enum PlaneEnum {
    None = 0,
    XY,
    YZ,
    ZX,
    UV,
    VW,
    WU
};

enum CoordEnum {
    NoEXT = 0,
    ABC,
    IJK,
    UVW
};

//
// g-code instruction
//
class GCodeData
{
    public:
        int   gCmd;
        int   mCmd;

        bool  toolChange; // to change the tool
        int   toolNumber; // собственно номер tool
        float toolDiameter; // diameter of tool

        int   commandNum; // set it, if instuction was sent

        int   labelNum;

        int   pauseMSec;  // if waiting = 0, no pause = -1. other pause in milliseconds
        //
        // coordinates in mm
        QVector3D baseCoord; // XYZ
        CoordEnum useExtCoord;
        QVector3D extCoord;

        // if arc splitted, number of followed cuts.
        int   splits; // TODO we need this?
        // for convertion from G02/G03 to G01
        QVector<QVector3D> arcCoord;

        bool decoded;

        PlaneEnum plane; // XY, YZ, ZX

        QString lineComment;

        int   vectSpeed; // telegr CA offset
        float vectorCoeff; // for the max from dH / dX of dH / dY ratio, in case XY plane

        float rapidVelo;

        int   movingCode;
        int   stepsCounter; // number of steps in current direction

        float radius;
        // end of curves

        bool  spindelOn;      // spinle on
        bool  mistOn;
        bool  coolantOn;

        int   numberLine;     // from g-code file

        // TODO local data for calculations?
        float angle; // angle between two lines around the actual point
        float deltaAngle;

    public:
        //
        // null constructor
        GCodeData();

        // constructor with copy from last data
        GCodeData(GCodeData *_cmd);
};



class GData : public QObject
{
        Q_OBJECT
    public:
        explicit GData(); // constructor
        ~GData(); // destructor

        QVector<GCodeData> *dataVector();
        bool readGCode(char *indata);

    private:
        void gcodeInit();
        void gcodeDestroy();


    signals:
        void logMessage(const QString &l);

    public:
        static QVector<GCodeData> gCodeVector;
        QMutex mut;
};


#endif
