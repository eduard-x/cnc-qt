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


#ifndef READER_H
#define READER_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMutex>

#include <deque>
#include <utility>
#include "vec.h"

#include "GCode.h"
#include "Translator.h"


#define byte unsigned char

class cTranslator;


#define COORD_TOO_BIG 10e6
#define MAX_FILE_SIZE 20e6



#define ACCELERAT_CODE  0x11
#define DECELERAT_CODE  0x21
#define CONSTSPEED_CODE 0x01
#define FEED_LINE_CODE  0x31
#define RAPID_CODE      0x39


//
// result parsing of g-code
//
struct GCode_resultParse {
    //     QString FullStr; //
    QString GoodStr; // for decoded
    QString BadStr;  // for unknown
};



// enum axisEnum {
//     None = 0,
//     X = 1,
//     Y = 2,
//     Z = 4,
//     A = 8,
//     B = 16,
//     C = 32,
//     U = 64,
//     V = 128,
//     W = 256,
//     XZ = Z | X,
//     XYZ = XZ | Y,
//     ABC = C | B | A,
//     UVW = W | V | U,
//     All = UVW | ABC | XYZ,
// };


enum typeFileLoad {
    None,
    GCODE,
    PLT,
    DRL,
    SVG,
    EPS,
    DXF,
    GBR
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
        DataCollections(const QList<Point> &_Points, Instrument _intrument = (Instrument) {
            0, 0.0
        }) {
            TypeData = Points;
            points = _Points;
            intrument = _intrument;
        };
    public:
        typeCollections TypeData;
        QList<Point> points;
        Instrument intrument;
};


///
/// class for gerber file
///
class GerberData
{
        ///
        /// messure units, mm or inches
    public:
        QString UnitsType;

        ///
        /// spline types
        ///
        QList<typeSpline> typeSplines;

        ///
        /// points from file
        ///
        QList<grbPoint> points;

        // length of number
        int countDigitsX;
        int countDigitsY;
        // length of digs after dec.point
        int countPdigX;
        int countPdigY;


        int X_min;
        int X_max;

        int Y_min;
        int Y_max;

    public:
        GerberData();
        void CalculateGatePoints(int _accuracy);

};


// class for reading of different formats

class Reader : public cTranslator
{
    public:
        Reader();

        void BresenhamLine(QVector<QVector<byte> > &p, int x0, int y0, int x1, int y1, typeSpline _Splane);
        void BresenhamCircle(QVector<QVector<byte> > &p,  int x0, int y0, int radius, byte setvalue = 4, bool needFill = false);

        bool readFile( const QString &fileName);

        //         void loadGCode(const QString &filename);


        //         void loadGCodeFromText(QStringList lines);
        //         bool parserGCodeLine(const QString &value);
        bool OpenFile(QString &name);
        void SaveFile();
        QStringList getGoodList();
        QStringList getBadList();

    public:
        QList<DataCollections> data;
        std::deque<std::pair<float, std::vector<Vec2d> > > layers;
        std::vector<Vec3f> cached_lines;
        //         std::vector<Vec3f> cached_arcs;
        std::vector<Vec3f> cached_points;
        std::vector<Vec3f> cached_color;

        QList<GCodeCommand> gCodeList;

        //             signals:
        //                 void logMessage(const QString &s);

    private:
        void Swap(int &p1, int &p2);
        bool parseCoord(const QString &line, Vec3 &pos, float &E, const float coef, float *F = NULL);
        bool parseArc(const QString &line, Vec3 &pos, float &R, const float coef);
        bool addLine(GCodeCommand *c);
        bool addArc(GCodeCommand *c);
        void convertArcToLines(GCodeCommand *c);
        float determineAngle(const Vec3 &pos, const Vec3 &pos_c, PlaneEnum pl);
        bool readGCode( const QByteArray &gcode );
        bool readGBR( const QByteArray &gcode );
        bool readDRL( const QByteArray &gcode );
        bool readDXF( const QByteArray &gcode );
        bool readSVG( const QByteArray &gcode );
        bool readEPS( const QByteArray &gcode );
        bool readPLT( const QByteArray &gcode );
        //         void lock() const;
        //         void unlock() const;
        //         GCode_resultParse parseStringGCode(const QString &value);

    private:
        QChar fromDecimalPoint;
        QChar toDecimalPoint;
        typeFileLoad TypeFile;// = typeFileLoad.None;
        QStringList goodList; // only decoded G-code
        QStringList badList;

        //         mutable QMutex mutex;
};


#endif // READER_H
