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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QGraphicsScene>
#include <QVector>
#include <QPixmap>

#include "MainWindow.h"
#include "ui_Settings.h"

class cTranslator;
class MainWindow;


#define COLOR_LINES 16

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
    COLOR_BACKGROUND,
    COLOR_TOOL,
    COLOR_WORKBENCH, // 5
    COLOR_TRAVERSE,
    COLOR_RAPID,
    COLOR_WORK,
    COLOR_GRID,
    COLOR_SURFACE, // 10
    COLOR_CONNECTION,
    COLOR_BORDER
};


class Settings
{
    public:
        static axis coord[9]; // array of 4 axes for mk1, 9 axes for mk2
        static int splitsPerMm;
        static float maxLookaheadAngle;
        static int pointSize;
        static int lineWidth;
        static bool smoothMoving;
        static bool showTraverse;
        static bool showWorkbench;

        static byte bb14;
        // static byte bb15; limits
        static byte bb19;

        static QChar fromDecimalPoint;
        static QChar toDecimalPoint;

        static QColor colorSettings[COLOR_LINES];
        //         QVector<axis> mk2[9]; // array of 9 axis for mk2

        //         static bool setSettings;
        //         static int  spindleMoveSpeed;
        //         static bool spindleEnabled;
        //         static bool mistEnabled;
        //         static bool fluidEnabled;
        //
        //         static bool Estop;
        static bool filterRepeat;
        // for virtual controller
        static bool DEMO_DEVICE;
};



class SettingsDialog : public QDialog, public Ui::SettingsDialog,  public cTranslator
{
        Q_OBJECT
    public:
        SettingsDialog(QWidget *parent = 0);

    private slots:
        void onSave();
        void onSelection(QTreeWidgetItem* it, QTreeWidgetItem * ip);
        void onChangeColor(int i);
        void onChangeTool(int i);
        void changeColor();
        void onChangeConnector(int i);

    private:
        void translateDialog();

    private:
        MainWindow* parent;
        mk1Controller *cnc;
        QVector< QVector<QString> > toolArray;
        QVector<QPixmap> frz_png;
        QGraphicsScene *grph;
        QVector< QVector<QString> > menuArr;
        QVector< QVector<QGroupBox*> > grpArr;
};


#endif // SETTINGS_H
