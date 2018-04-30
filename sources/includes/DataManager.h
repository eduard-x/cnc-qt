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


#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMutexLocker>
// #include <QMutex>
#include <QPair>

#include "GCode.h"

class GCodeParser;


// #define COORD_TOO_BIG 10e6
#define MAX_FILE_SIZE 20e6




//
// result parsing of g-code
//
// struct GCode_resultParse {
//     //     QString FullStr; //
//     QString GoodStr; // for decoded
//     QString BadStr;  // for unknown
// };



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

// class for reading of different formats

class cDataManager : public GData // , public cTranslator
{
    public:
        cDataManager();
        ~cDataManager();
#if 1
        void BresenhamLine(QVector<QVector<quint8> > &p, int x0, int y0, int x1, int y1, typeSpline _Splane);
        void BresenhamCircle(QVector<QVector<quint8> > &p,  int x0, int y0, int radius, quint8 setvalue = 4, bool needFill = false);
#endif
        bool readFile( const QString &fileName);

        //         void loadGCode(const QString &filename);


        //         void loadGCodeFromText(QStringList lines);
        //         bool parserGCodeLine(const QString &value);

        void writeFile(const QString &fileName);

        void fixGCodeList();

    public:
        QList<DataCollections> data;
        QVector<QPair<float, QVector<QVector2D> > > layers;
        //         std::vector<Vec3f> cached_lines;
        //         std::vector<Vec3f> cached_arcs;
        //         std::vector<Vec3f> cached_points;
        //         std::vector<Vec3f> cached_color;

        QVector<QString> *stringList();
        //         QString lastDir;

        //             signals:
        //                 void logMessage(const QString &s);

    private:

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

    private:
        bool addLine(GCodeData* param);
        bool addArc(GCodeData* param);
        void dataChecker();

        void resetSoftLimits();

        //         QVector <GCodeOptim> getRapidPoints();

        void Swap(int &p1, int &p2);

        float determineAngle(const QVector3D &pos, const QVector3D &pos_center, PlaneEnum pl);
        void convertArcToLines(int p);
        float calcAngleOfLines(const QVector3D &c1, const QVector3D &c2, int plane);

        int  calculateMinAngleSteps(int pos);
        void patchSpeedAndAccelCode(int begPos, int endPos);

        void detectMinMax(const QVector3D& v);

        //         bool readGCode( const QByteArray &gcode );
        bool readGBR( const QByteArray &gcode );
        bool readDRL( const QByteArray &gcode );
        bool readDXF( const QByteArray &gcode );
        bool readSVG( const QByteArray &gcode );
        bool readEPS( const QByteArray &gcode );
        bool readPLT( const QByteArray &gcode );

    protected:
        void sortGCode(const QVector<int> &antdata);
        void antColonyOptimization();
        const QVector<int> calculateAntPath(/*const QVector<GCodeOptim> &v*/);

        //         GCode_resultParse parseStringGCode(const QString &value);

    private:
        //         QByteArray arr;

        QVector<QString> goodList; // only decoded G-code

        QVector<int> path;
        QVector<int> occup;
        QVector <QVector <float> > distance;

        QVector<GCodeOptim> g0Points;

        float maxLookaheadAngleRad;
        typeFileLoad TypeFile;// = typeFileLoad.None;
};


#endif // DATAMANAGER_H
