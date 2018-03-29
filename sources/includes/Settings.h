/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QStringList>
#include <QString>

#include <QSettings>
#include <QtGui>

#include "MainWindow.h"


enum AxisNames { X = 0, Y, Z, A, B, C, U, V, W };

enum ParamNames { PARAM_CMD = 0, PARAM_X, PARAM_Y, PARAM_Z, PARAM_A, PARAM_B, PARAM_C, PARAM_I, PARAM_J, PARAM_K, PARAM_F };

class axis
{
    public:
        axis(); // constructor
        float posMm();
        int posPulse(float posMm);

    public:
        float minVeloLimit;
        float maxVeloLimit;
        float acceleration;
        int   pulsePerMm;
        float actualPosmm;
        int   actualPosPulses;
        bool  invertDirection;
        bool  invertPulses;

        bool  useLimitMin; // set HW
        bool  useLimitMax; // set HW
        bool  invLimitMin; // set HW
        bool  invLimitMax; // set HW

        bool  actualLimitMax; // get from HW
        bool  actualLimitMin; // get from HW

        int connector; // physical connector number
        float startPos;
        bool  checkSoftLimits;

        float softLimitMax;
        float softLimitMin;
        float backlash;
        float workAreaMin;
        float workAreaMax;
        bool  enabled;
        float home;
        bool  wrong;
};

enum {
    COLOR_X = 0,
    COLOR_Y,
    COLOR_Z,
    COLOR_BGROUND,
    COLOR_TOOL,
    COLOR_WORKBENCH, // 5
    COLOR_TRAVERSE,
    COLOR_RAPID,
    COLOR_WORK,
    COLOR_GRID,
    COLOR_SURFACE, // 10
    COLOR_CONNECTION,
    COLOR_BORDER,
    COLOR_ENTRIES
};



class Settings : public QSettings
{
    public:
        static bool saveSettings();
        static bool readSettings();

    public:
        static axis coord[9]; // array of 4 axes for mk1, 9 axes for mk2
        static int splitsPerMm;
        static float maxLookaheadAngle;
        static int pointSize;
        static int lineWidth;
        static bool smoothMoving;
        static bool showTraverse;
        static bool showWorkbench;

        static QString currentLang;
        static QPoint progPos;
        static QSize progSize;

        static quint8 bb14;
        static quint8 bb16;
        static quint8 bb19;

        static char fromDecimalPoint;
        static char toDecimalPoint;

        static QColor colorSettings[COLOR_ENTRIES];
        //         QVector<axis> mk2[9]; // array of 9 axis for mk2

        static int depthSum;
        static int repeatTimes;
        static bool optimizeRapidWays;
        static bool repeatProcessing;
        //
        //         static bool Estop;
        static bool filterRepeat;
        // for virtual controller
        static bool DEMO_DEVICE;
        static    bool unitMm;
        static int veloCutting;
        // 3d Settings
        static bool ShowBorder;

        static MATERIAL cuttedMaterial;
        static float toolDiameter;
        static int toolFlutes;
        static int toolRPM;

        static short maxAntSearchDepth;

        static bool disableOpenGL;
        static QString remoteName;
        static int remotePort;
        static bool enableRemote;

        static bool carbideTool;
        static int  cuttingDeep;

        static int PosX, PosY, PosZ;
        static int PosAngleX, PosAngleY, PosAngleZ;

        static float PosZoom;

        static bool ShowInstrument;
        static bool ShowGrid;
        static bool ShowLines;
        static bool ShowPoints;
        static bool ShowSurface;
        static bool ShowAxes;
        static bool ShowMessure;
        static bool ShowInstrumentCone; // cone or cylinder
        static float ShowIntrumentHight;
        static float ShowIntrumentDiameter;

        //         bool disableIfSSH;

        static int GridXstart;
        static int GridXend;
        static int GridYstart;
        static int GridYend;
        static int GrigStep;
        // end of 3d

        // user defined control keys
        static QMap<QString, Qt::Key> userKeys;
        //
        static int veloManual;

        static int currentKeyPad;

        static int accelerationCutting;
        static int minVelo;
        static int maxVelo;
        static int veloMoving;
        static QString langDir;
        static QString lastDir;
        static QStringList lastFiles;
        static QString helpDir;
        static QString currentAppDir;
        static QFont sysFont;
        static short fontSize;
        static QString axisNames;

    private:
        static QString getLocaleString();
};


#endif // SETTINGS_H
