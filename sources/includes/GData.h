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

#include <QVector3D>
#include <QVector>

#define NO_CODE         0x00
#define CONSTSPEED_CODE 0x01
#define ACCELERAT_CODE  0x11
#define DECELERAT_CODE  0x21
#define FEED_LINE_CODE  0x31
#define RAPID_LINE_CODE 0x39


#ifndef GDATA_H
#define GDATA_H


/* enum class is C++11 feature */
enum class EXP {
    NONE = 0,
    MUL,
    DIV,
    PLUS,
    MINUS,
    NEG,
    POW,
    SQRT,
    SIN,
    COS,
    TAN,
    EQ,
    GE,
    LE,
    GT,
    LT
};

enum class DATA {
    NONE = 0,
    SVAL,
    FVAL,
    IVAL
};

enum class CMD {
    NOP =  0,
    IF,
    ELSE,
    EIF,
    WHILE,
    EWHILE,
    GOTO,
    DELAY
};


class DataOperation
{
    public :
        DataOperation(EXP code = EXP::NONE, DATA tp = DATA::NONE)
        {
            opCode = code;
            varType = tp;
            result = 0.0;
            expr1 = 0; // pointer init
            expr2 = 0; // pointer init
        }
        ~DataOperation()
        {
            if (expr1) {
                delete expr1;
            }

            if (expr2) {
                delete expr2;
            }

            if(varType == DATA::SVAL && vName.length()) {
                vName.clear();
            }
        }
        EXP   opCode; // mathematical operations + - / * , or variables FVAL, SVAL and so
        DATA  varType;
        float result; // in case of IVAL, FVAL or converted from SVAL
        QString vName;
        class DataOperation* expr1;
        class DataOperation* expr2;
};


enum Apertures {
    C_circle,
    R_rectangle,
    O_obround,
    P_polygon
};

// possible data types
enum typeCollections {
    Points,
    Instruments,
    Property,
};


struct typeSpline {
    int number;
    Apertures aperture;
    float size1;
    float size2;

    //     public typeSpline(int _number, Apertures _aperture, float _size1 = 0, float _size2 = 0)
    //     {
    //         number = _number;
    //         aperture = _aperture;
    //         size1 = _size1;
    //         size2 = _size2;
    //     }
};



// gerber point descriptor
struct grbPoint {
    int X;
    int Y;
    QString typePoint; // D1 - видимое движение D2 - невидимое движение D3 - точка
    int numberSplane;

    //     public grbPoint(int _x, int _y, QString _typePoint, int _numberSplane)
    //     {
    //         X = _x;
    //         Y = _y;
    //         typePoint = _typePoint;
    //         numberSplane = _numberSplane;
    //     }
};


// point descriptor
struct Point {
    float X;
    float Y;
    bool visible; //data to view
    int size; //line size

    //     public Point(float _x, float _y, bool _visible = true, int _size = 1)
    //     {
    //         X = _x;
    //         Y = _y;
    //         visible = _visible;
    //         size = _size;
    //     }
};

// tool descriptor
struct Instrument {
    int Number;
    float Diametr;

    //     public Instrument(int _number, float _diametr)
    //     {
    //         Numer = _number;
    //         Diametr = _diametr;
    //     }
};


//
class DataCollections
{
        ///
        ///points
    public:
        DataCollections(const QList<Point> &_Points, Instrument _intrument = (Instrument)
        {
            0, 0.0
        })
        {
            TypeData = Points;
            points = _Points;
            intrument = _intrument;
        };
    public:
        typeCollections TypeData;
        QList<Point> points;
        Instrument intrument;
};


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
        //         float angle; // angle between two lines around the actual point
        //         float deltaAngle;

    public:
        //
        // null constructor
        GCodeData();

        // constructor with copy from last data
        GCodeData(GCodeData *_cmd);
        QVector<class DataOperation*> opVector;
};


#endif // GDATA_H

