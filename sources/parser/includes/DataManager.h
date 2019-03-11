/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2019 by Eduard Kalinowski                             *
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
#include <QMutex>
#include <QPair>


// #define COORD_TOO_BIG 10e6
#define MAX_FILE_SIZE 20e6

#include "GData.h"

// class for reading of different formats

class cDataManager //: private QObject //: public Parser // , public cTranslator
{
    public:
        explicit cDataManager();
        ~cDataManager();
#if 0
        void BresenhamLine(QVector<QVector<quint8> > &p, int x0, int y0, int x1, int y1, typeSpline _Splane);
        void BresenhamCircle(QVector<QVector<quint8> > &p,  int x0, int y0, int radius, quint8 setvalue = 4, bool needFill = false);
#endif
        bool readFile( const QString &fileName);

        void writeFile(const QString &fileName);


    public:
        QStringList logBuffer; // for sending of log data

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

    public:
        //         bool addLine(ParserData* param);
        //         bool addArc(ParserData* param);
        //         void dataChecker();

        void resetSoftLimits();
        void checkCity(const QVector3D &current_pos, int pos);

        //         QVector <GCodeOptim> getRapidPoints();

        void Swap(int &p1, int &p2);

        float determineAngle(const QVector3D &pos, const QVector3D &pos_center, PlaneEnum pl);
        void convertArcToLines(int p);
        float calcAngleOfLines(const QVector3D &c1, const QVector3D &c2, int plane);

        void detectMinMax(const QVector3D& v);

    protected:
        void sortGCode(const QVector<int> &antdata);
        void antColonyOptimization();
        const QVector<int> calculateAntPath(/*const QVector<GCodeOptim> &v*/);
        void checkMCommand(const SerialData &s);

    public:
        QVector<VertexData> vertexVector;
        QVector<SerialData*> serialDataVector;

        // for the control data
        MData *currentMCmd;

        QVector<GCodeOptim> gCities;
        QVector<GData> dataVector;
        QVector<QString> filteredList; // only decoded G-code
        QMap<QString, float> dataVaris;

    private:
        QVector<int> path;
        QVector<int> occup;
        QVector <QVector <float> > distance;

        typeFileLoad TypeFile;// = typeFileLoad.None;

        QMutex mut;
};


#endif // DATAMANAGER_H
