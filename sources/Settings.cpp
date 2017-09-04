/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
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

#include <QLocale>
#include <QVector>


#include "includes/Settings.h"



axis::axis()
{
    acceleration = 50.0;
    actualLimitMax = false;
    actualLimitMin = false;
    enabled = true;
    backlash = 0.0;
    connector = 0;
    invertDirection = false;
    invertPulses = false;
    invLimitMax = false;
    invLimitMin = false;
    workAreaMax = 100.0;
    workAreaMin = -100.0;
    pulsePerMm = 200;
    actualPosPulses = 0;
    wrong = false;
}


float axis::posMm()
{
    if (pulsePerMm != 0) {
        return (float)(actualPosPulses / (float) pulsePerMm);
    } else {
        return 0.0;
    }
}


int axis::posPulse(float posMm)
{
    return (int)(posMm * (float)pulsePerMm);
}


QString Settings::axisNames = "XYZA";
QString Settings::currentLang = "English";
QString Settings::helpDir = "";
QString Settings::langDir = "";
QPoint Settings::progPos = QPoint();
QSize Settings::progSize = QSize();
QFont Settings::sysFont = QFont();
short Settings::fontSize = 10;

QStringList Settings::lastFiles = QStringList("");
QChar Settings::toDecimalPoint = '.';
QChar Settings::fromDecimalPoint = ';';
bool Settings::DEMO_DEVICE = false;
int  Settings::splitsPerMm = 10;
float Settings::maxLookaheadAngle = 170.0;

int Settings::pointSize = 1;
int Settings::lineWidth = 3;
bool Settings::smoothMoving = false;
bool Settings::showTraverse = false;
bool Settings::showWorkbench = false;
bool Settings::filterRepeat = true;
float Settings::PosZoom = 1.0;
int Settings::PosAngleX = 1;
int Settings::PosAngleY = 1;
int Settings::PosAngleZ = 1;
int Settings::PosX = 1;
int Settings::PosY = 1;
int Settings::PosZ = 1;
int Settings::veloCutting = 200;
bool Settings::unitMm = true;

bool Settings::disableOpenGL = false;
QString Settings::remoteName = "";
QString Settings::lastDir = "";
int Settings::remotePort = 0;
bool Settings::enableRemote = false;

byte Settings::bb14 = 0x0;
byte Settings::bb16 = 0x0;
byte Settings::bb19 = 0x0;

bool Settings::ShowBorder = true;
QString Settings::currentAppDir = "";
MATERIAL Settings::cuttedMaterial = HARDWOOD;

float Settings::toolDiameter = 0.1;
int Settings::toolFlutes = 2;
int Settings::toolRPM = 10000;

int Settings::GridXstart = 0;
int Settings::GridXend = 0;
int Settings::GridYstart = 0;
int Settings::GridYend = 0;
int Settings::GrigStep = 0;

bool Settings::ShowInstrument = true;
bool Settings::ShowGrid = true;
bool Settings::ShowLines = false;
bool Settings::ShowPoints = true;
bool Settings::ShowSurface = false;
bool Settings::ShowAxes = true;


// end of 3d

QMap<QString, Qt::Key> Settings::userKeys = {
    { "UserAplus", Qt::Key_multiply },
    { "UserAminus", Qt::Key_division },
    { "UserZplus", Qt::Key_Home },
    { "UserZminus", Qt::Key_End },
    { "UserYplus", Qt::Key_Up },
    { "UserYminus", Qt::Key_Down },
    { "UserXplus", Qt::Key_Right },
    { "UserXminus", Qt::Key_Left }
};
//

int Settings::veloManual = 400;

int Settings::currentKeyPad = -1;

int Settings::accelerationCutting = 15;
int Settings::minVelo = 20;
int Settings::maxVelo = 400;
int Settings::veloMoving = 500;


bool Settings::carbideTool = false;
int  Settings::cuttingDeep = 1; // from diameter

int  Settings::depthSum = 6;
int  Settings::repeatTimes = 0;
bool Settings::optimizeRapidWays = false;
bool Settings::repeatProcessing = false;


QColor Settings::colorSettings[COLOR_ENTRIES];


axis Settings::coord[] = { axis(), axis(), axis(), axis() };


bool Settings::saveSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );


    s->setValue("pos", progPos);
    s->setValue("size", progSize);
    s->setValue("LANGUAGE", currentLang);
    s->setValue("LASTDIR", lastDir);

    s->setValue("VelocityCutting", veloCutting);
    s->setValue("VelocityMoving", veloMoving);
    s->setValue("VelocityManual", veloManual);

    s->setValue("SplitArcPerMM", splitsPerMm);
    s->setValue("LookaheadAngle", maxLookaheadAngle);

    s->setValue("UnitMM", unitMm);
    s->setValue("ToolDiameter", toolDiameter);
    s->setValue("ToolFlutes", toolFlutes);
    s->setValue("ToolRPM", toolRPM);

    s->setValue("FilterRepeatData", filterRepeat);

    s->setValue("CuttedMaterial", cuttedMaterial);

    QMapIterator<QString, Qt::Key> imap(userKeys);

    while (imap.hasNext()) {
        imap.next();
        s->setValue( imap.key(),  (quint32)imap.value());
    }

    s->setValue("KeyControl", (int) currentKeyPad);

    lastFiles.removeDuplicates();

    int i = 0;

    foreach (QString l, lastFiles) {
        if (i > 9) { // max last dirs
            break;
        }

        s->setValue("LASTFILE" + QString::number(i), l);
        i++;
    }

    // opengl settings
    s->beginGroup("OpenGL");

    s->setValue("ShowLines", ShowLines);
    s->setValue("ShowPoints", ShowPoints);

    s->setValue("ShowInstrument", ShowInstrument);
    s->setValue("ShowGrid", ShowGrid);
    s->setValue("ShowSurface", ShowSurface);
    s->setValue("ShowAxes", ShowAxes);

    s->setValue("DisableOpenGL", disableOpenGL);
    s->setValue("RemoteName", remoteName);
    s->setValue("RemotePort", remotePort);
    s->setValue("RemoteEnable", enableRemote);
    s->setValue("DepthSum", depthSum);
    s->setValue("RepeatTimes", repeatTimes);
    s->setValue("OptimizeRapid", optimizeRapidWays);
    s->setValue("RepeatProcessing", repeatProcessing);


    s->setValue("GrigStep", (int)GrigStep);

    s->setValue("GridXstart", (int)GridXstart);
    s->setValue("GridXend", (int)GridXend);
    s->setValue("GridYstart", (int)GridYstart);
    s->setValue("GridYend", (int)GridYend);

    s->setValue("ShowGrate", (bool)ShowBorder); // grenzen

    s->setValue("PosX", (int)PosX); //
    s->setValue("PosY", (int)PosY); //
    s->setValue("PosZ", (int)PosZ); //

    s->setValue("AngleX", (int)PosAngleX); //
    s->setValue("AngleY", (int)PosAngleY); //
    s->setValue("AngleZ", (int)PosAngleZ); //

    s->setValue("Zoom", (int)PosZoom); //


    s->setValue("Color_X", (QColor)colorSettings[COLOR_X]);
    s->setValue("Color_Y", (QColor)colorSettings[COLOR_Y]);
    s->setValue("Color_Z", (QColor)colorSettings[COLOR_Z]);
    s->setValue("Color_BG", (QColor)colorSettings[COLOR_BACKGROUND]);
    s->setValue("Color_Tool", (QColor)colorSettings[COLOR_TOOL]);
    s->setValue("Color_WB", (QColor)colorSettings[COLOR_WORKBENCH]);
    s->setValue("Color_Traverse", (QColor)colorSettings[COLOR_TRAVERSE]);
    s->setValue("Color_Rapid", (QColor)colorSettings[COLOR_RAPID]);
    s->setValue("Color_Work", (QColor)colorSettings[COLOR_WORK]);
    s->setValue("Color_Grid", (QColor)colorSettings[COLOR_GRID]);
    s->setValue("Color_Border", (QColor)colorSettings[COLOR_BORDER]);
    s->setValue("Color_Surface", (QColor)colorSettings[COLOR_SURFACE]);
    s->setValue("Color_Connect", (QColor)colorSettings[COLOR_CONNECTION]);

    s->setValue("LineWidth", (int)lineWidth);
    s->setValue("PointSize", (int)pointSize);
    s->setValue("SmoothMoving", (bool)smoothMoving);
    s->setValue("ShowTraverse", (bool)showTraverse);
    s->setValue("ShowWorkbench", (bool)showWorkbench);

    s->endGroup();

    s->beginGroup("mk1");

    for (int c = 0; c < axisNames.length(); c++) {
        s->setValue("Connector" + QString( axisNames.at(c)), coord[c].connector);
        s->setValue("Pulse" + QString( axisNames.at(c)), coord[c].pulsePerMm);
        s->setValue("Accel" + QString( axisNames.at(c)), (double)coord[c].acceleration);
        s->setValue("StartVelo" + QString( axisNames.at(c)), (double)coord[c].minVeloLimit);
        s->setValue("EndVelo" + QString( axisNames.at(c)), (double)coord[c].maxVeloLimit);

        //
        s->setValue("Backlash" + QString( axisNames.at(c)), (double)coord[c].backlash);
        s->setValue("InvDirection" + QString( axisNames.at(c)), (bool)coord[c].invertDirection);
        s->setValue("InvPulses" + QString( axisNames.at(c)), (bool)coord[c].invertPulses);
        s->setValue("InvLimitMax" + QString( axisNames.at(c)), (bool)coord[c].invLimitMax);
        s->setValue("InvLimitMin" + QString( axisNames.at(c)), (bool)coord[c].invLimitMin);
        s->setValue("WorkAreaMin" + QString( axisNames.at(c)), (double)coord[c].workAreaMin);
        s->setValue("WorkAreaMax" + QString( axisNames.at(c)), (double)coord[c].workAreaMax);
        s->setValue("Enabled" + QString( axisNames.at(c)), (bool)coord[c].enabled);
        //

        s->setValue("HardLimitMin" + QString( axisNames.at(c)), (bool)coord[c].useLimitMin);
        s->setValue("HardLimitMax" + QString( axisNames.at(c)), (bool)coord[c].useLimitMax);

        s->setValue("SoftLimit" + QString( axisNames.at(c)), (bool)coord[c].checkSoftLimits);
        s->setValue("SoftMin" + QString( axisNames.at(c)), (double)coord[c].softLimitMin);
        s->setValue("SoftMax" + QString( axisNames.at(c)), (double)coord[c].softLimitMax);

        s->setValue("Home" + QString( axisNames.at(c)), (double)coord[c].home);
    }

    s->endGroup();

    s->sync();

    delete s;
}



/**
 * @brief get the system locale for selection of language, if exists
 *
 */
QString Settings::getLocaleString()
{
    QString res;
    QLocale lSys = QLocale::system();

    switch (lSys.language()) {
        case QLocale::C:
            res = "English";
            break;

        case QLocale::German:
            res = "Deutsch";
            break;

        case QLocale::Russian:
            res = "Russian";
            break;

        default:
            res = "English";
            break;
    }

    return res;
}



bool Settings::readSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );

    //     s->beginGroup("General");

    progPos = s->value("pos", QPoint(200, 200)).toPoint();
    progSize = s->value("size", QSize(840, 640)).toSize();

    accelerationCutting = s->value("AccelerationCutting", 15).toInt();
    minVelo = s->value("MinVelocity", 20).toInt();
    maxVelo = s->value("MaxVelocity", 400).toInt();

    veloCutting = s->value("VelocityCutting", 200).toInt();
    veloMoving = s->value("VelocityMoving", 500).toInt();
    veloManual = s->value("VelocityManual", 400).toInt();
    currentKeyPad = s->value("KeyControl", -1).toInt();

    unitMm = s->value("UnitMM", 1).toBool();
    splitsPerMm =   s->value("SplitArcPerMM", 10).toInt();
    maxLookaheadAngle = s->value("LookaheadAngle", 170.0).toFloat();
    cuttedMaterial = (MATERIAL)s->value("CuttedMaterial", 0).toInt();

    toolDiameter = s->value("ToolDiameter", 3.0).toFloat();
    toolFlutes = s->value("ToolFlutes", 2).toInt();
    toolRPM = s->value("ToolRPM", 10000).toInt();

    filterRepeat = s->value("FilterRepeatData", true).toBool();


    QMapIterator<QString, Qt::Key> imap(userKeys);

    while (imap.hasNext()) {
        imap.next();
        userKeys[imap.key()] = (Qt::Key)s->value(imap.key(), (quint32)imap.value()).toUInt();
    }

    QString l;
    l = getLocaleString();

    currentLang = s->value("LANGUAGE", l).toString();

    lastDir = s->value("LASTDIR", "").toString();

    sysFont = sysFont.toString();

    int sz = sysFont.pointSize();

    if ( sz == -1) {
        sz = sysFont.pixelSize();
    }

    fontSize = sz;
    lastFiles.clear();

    for (int i = 0; i < 10; i++) {
        QString d = s->value("LASTFILE" + QString::number(i)).toString();
        QFile fl;

        if (d.length() == 0) {
            break;
        }

        if (fl.exists(d) == true) {
            lastFiles << d;
        }
    }

    lastFiles.removeDuplicates();



    QDir dir;
    QStringList dirsLang;
    dirsLang << "/usr/share/cnc-qt/" << "/usr/local/share/cnc-qt/" << currentAppDir;

    foreach(QString entry, dirsLang) {
        helpDir = entry + "/help/";

        dir = QDir(helpDir);

        if (dir.exists() == true) {
            break;
        } else {
            helpDir = "";
        }
    }

    foreach(QString entry, dirsLang) {
        langDir = entry + "/lang/";

        dir = QDir(langDir);

        if (dir.exists() == true) {
            break;
        } else {
            langDir = "";
        }
    }

    //       s->endGroup();

    //     if (enableOpenGL == true) {
    // opengl settings
    s->beginGroup("OpenGL");

    ShowLines = s->value("ShowLines", false).toBool();
    ShowPoints = s->value("ShowPoints", true).toBool();

    ShowInstrument = s->value("ShowInstrument", true).toBool();
    ShowGrid = s->value("ShowGrid", true).toBool();
    ShowSurface = s->value("ShowSurface", false).toBool();
    ShowAxes = s->value("ShowAxes", true).toBool();

    disableOpenGL = s->value("DisableOpenGL", false).toBool();
    remoteName = s->value("RemoteName", "").toString();
    remotePort = s->value("RemotePort", 0).toInt();
    enableRemote = s->value("RemoteEnable", false).toBool();
    depthSum = s->value("DepthSum", 6).toInt();
    repeatTimes = s->value("RepeatTimes", 0).toInt();
    optimizeRapidWays = s->value("OptimizeRapid", false).toBool();
    repeatProcessing = s->value("RepeatProcessing", false).toBool();

    GrigStep = s->value("GrigStep", 10).toInt();

    GridXstart = s->value("GridXstart", -100).toInt();
    GridXend = s->value("GridXend", 100).toInt();
    GridYstart = s->value("GridYstart", -100).toInt();
    GridYend = s->value("GridYend", 100).toInt();

    ShowBorder = s->value("ShowGrate", true).toBool(); // grenzen

    PosX = s->value("PosX", -96 ).toInt(); //
    PosY = s->value("PosY", -64 ).toInt(); //
    PosZ = s->value("PosZ", -300 ).toInt(); //

    PosAngleX = s->value("AngleX", 180 ).toInt(); //
    PosAngleY = s->value("AngleY", 180 ).toInt(); //
    PosAngleZ = s->value("AngleZ", 180 ).toInt(); //

    PosZoom = s->value("Zoom", 20 ).toInt(); //

    colorSettings[COLOR_X] = s->value("Color_X", QColor {
        0, 255, 0, 255
    }).value<QColor>();
    colorSettings[COLOR_Y] = s->value("Color_Y", QColor {
        255, 0, 0, 255
    }).value<QColor>();
    colorSettings[COLOR_Z] = s->value("Color_Z", QColor {
        0, 255, 255, 255
    }).value<QColor>();
    colorSettings[COLOR_BACKGROUND] = s->value("Color_BG", QColor {
        100, 100, 100, 255
    }).value<QColor>();
    colorSettings[COLOR_TOOL] = s->value("Color_Tool", QColor {
        255, 255, 0, 255
    }).value<QColor>();
    colorSettings[COLOR_WORKBENCH] = s->value("Color_WB", QColor {
        0, 0, 255, 255
    }).value<QColor>();
    colorSettings[COLOR_TRAVERSE] = s->value("Color_Traverse", QColor {
        255, 255, 255, 255
    }).value<QColor>();
    colorSettings[COLOR_RAPID] = s->value("Color_Rapid", QColor {
        255, 0, 0, 255
    }).value<QColor>();
    colorSettings[COLOR_WORK] = s->value("Color_Work", QColor {
        0, 255, 0, 255
    }).value<QColor>();
    colorSettings[COLOR_GRID] = s->value("Color_Grid", QColor {
        200, 200, 200, 255
    }).value<QColor>();
    colorSettings[COLOR_BORDER] = s->value("Color_Border", QColor {
        200, 200, 200, 255
    }).value<QColor>();
    colorSettings[COLOR_SURFACE] = s->value("Color_Surface", QColor {
        255, 255, 255, 255
    }).value<QColor>();
    colorSettings[COLOR_CONNECTION] = s->value("Color_Connect", QColor {
        150, 255, 100, 255
    }).value<QColor>();

    pointSize = s->value("PointSize", 1).toInt();
    lineWidth = s->value("LineWidth", 3).toInt();
    smoothMoving = s->value("SmoothMoving", false).toBool();
    showTraverse = s->value("ShowTraverse", false).toBool();
    showWorkbench = s->value("ShowWorkbench", false).toBool();

    s->endGroup();
    //     }

    bool res;

    s->beginGroup("mk1");

    for (int c = 0; c < axisNames.length(); c++) {
        int i = s->value("Connector" + QString( axisNames.at(c)), c).toInt( &res);
        coord[c].connector = (res == true) ? i : c;

        i = s->value("Pulse" + QString( axisNames.at(c)), 200).toInt( &res);
        coord[c].pulsePerMm = (res == true) ? i : 200;

        float f = s->value("Accel" + QString( axisNames.at(c)), 15).toFloat( &res);
        coord[c].acceleration = (res == true) ? f : 15;

        f = s->value("StartVelo" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].minVeloLimit = (res == true) ? f : 0;

        f = s->value("EndVelo" + QString( axisNames.at(c)), 400).toFloat( &res);
        coord[c].maxVeloLimit = (res == true) ? f : 400;

        coord[c].checkSoftLimits = s->value("SoftLimit" + QString( axisNames.at(c)), false).toBool( );

        f = s->value("SoftMin" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].softLimitMin = (res == true) ? f : 0;

        f = s->value("SoftMax" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].softLimitMax = (res == true) ? f : 0;

        f = s->value("Home" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].home = (res == true) ? f : 0;

        coord[c].useLimitMin = s->value("HardLimitMin" + QString( axisNames.at(c)), true).toBool();
        coord[c].useLimitMax = s->value("HardLimitMax" + QString( axisNames.at(c)), true).toBool();

        //
        coord[c].invertDirection = s->value("InvDirection" + QString( axisNames.at(c)), false).toBool();
        coord[c].invertPulses = s->value("InvPulses" + QString( axisNames.at(c)), false).toBool();
        coord[c].invLimitMax = s->value("InvLimitMax" + QString( axisNames.at(c)), false).toBool();
        coord[c].invLimitMin = s->value("InvLimitMin" + QString( axisNames.at(c)), false).toBool();
        coord[c].enabled = s->value("Enabled" + QString( axisNames.at(c)), true).toBool();

        f = s->value("Backlash" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].backlash = (res == true) ? f : 0;

        f = s->value("WorkAreaMin" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].workAreaMin = (res == true) ? f : 0;

        f = s->value("WorkAreaMax" + QString( axisNames.at(c)), 0).toFloat( &res);
        coord[c].workAreaMax = (res == true) ? f : 0;
        //
    }

    s->endGroup();

    //     updateSettingsOnGUI();

    delete s;
}

