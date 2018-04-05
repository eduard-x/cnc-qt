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


#include <QObject>
#include <QRegExp>
#include <QDebug>
#include <QString>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/MainWindow.h"

#include "scan_gcode.h"


#define DEBUG_ARC 0

/**
 * @brief consructor
 *
 */
GCodeData::GCodeData()
{
    toolChange = false;
    toolNumber = 0;
    commandNum = 0; // is the command number for sending to mk1
    numberLine = 0;
    decoded = true;

    gCmd = -1;
    mCmd = -1;

    /** @var pauseMSec
     * no pause: -1
     * waiting: 0
     * pause > 0 in milliseconds
     */
    pauseMSec = -1; // no pause

    baseCoord = { 0.0, 0.0, 0.0 }; // X, Y, Z

    useExtCoord = NoEXT;

    extCoord = { 0.0, 0.0, 0.0 };

    plane = XY;

    lineComment = "";

    labelNum = -1;

    radius = 0.0;

    vectorCoeff = 0.0;
    // end of arc

    rapidVelo = 0.0;

    movingCode =  NO_CODE;

    vectSpeed = 0.0;

    stepsCounter = 0;

    angle = 0.0;
    deltaAngle = 0.0;

    spindelOn = false;
    mistOn = false;
    coolantOn = false;

    splits = 0; // init

    toolDiameter = 0.0;
};


/**
 * @brief constructor based on command
 *
 */
GCodeData::GCodeData(GCodeData *d)
{
    baseCoord = d->baseCoord;
    useExtCoord = NoEXT;

    extCoord = { 0.0, 0.0, 0.0 }; // for ABC, IJK, UVW

    decoded = true;

    gCmd = d->gCmd;
    mCmd = -1;

    radius = d->radius; // got G02, G03

    plane = d->plane;

    lineComment = "";

    rapidVelo = d->rapidVelo; // ???

    labelNum = -1;

    vectorCoeff = 0.0;

    spindelOn = d->spindelOn;
    mistOn = d->mistOn;
    coolantOn = d->coolantOn;

    vectSpeed = d->vectSpeed;

    splits = 0; // if arc, will be splitted, debug information only

    stepsCounter = 0; // should be calculated

    movingCode = d->movingCode;

    numberLine = d->numberLine;
    commandNum = 0;

    angle = 0.0; //d->angleVectors;

    deltaAngle = 0.0;

    toolChange = d->toolChange;
    toolNumber = d->toolNumber;
    pauseMSec = d->pauseMSec;
    toolDiameter = d->toolDiameter;
};

// is static
QVector<GCodeData> GData::gCodeVector;

/**
 * @brief constructor
 *
 */
GData::GData()
{
}

/**
 * @brief destructor
 *
 */
GData::~GData()
{
    gCodeVector.clear();
}


void GData::gcodeInit()
{
    gcode_lineno = 0;
    //      gcode_result = NULL;
    //   gcode_vector = NULL;
    //   gcode_header = NULL;
}



void GData::gcodeDestroy()
{
    //      if (csv_result != NULL) {
    //     // delete associated dataset
    //     delete csv_result;
    //     csv_result = NULL;
    //   }
    //   if (csv_vector != NULL) {
    //     csv_finalize ();
    //     csv_vector = NULL;
    //   }
}


void GData::resetSoftLimits()
{
    Settings::coord[X].softLimitMax = 0;
    Settings::coord[X].softLimitMin = 0;
    Settings::coord[Y].softLimitMax = 0;
    Settings::coord[Y].softLimitMin = 0;
    Settings::coord[Z].softLimitMax = 0;
    Settings::coord[Z].softLimitMin = 0;
}


/**
 * @brief read and parse into GCodeData list and OpenGL list
 * @see for the optimizations see https://blog.qt.io/blog/2014/06/13/qt-weekly-13-qstringliteral/
 * TODO convert QString to QStringLiteral
 *
 */
bool GData::readGCode(char *indata)
{
    int ret = true;

    gCodeVector.clear();

    mut.lock();

    gcodeInit();

    /* because the data in already in buffer 'indata' */

    YY_BUFFER_STATE bs = gcode__scan_string(indata);
    gcode__switch_to_buffer(bs);

    if ( gcode_parse() != 0) {
        ret = false;
    }

    gcode_lex_destroy();

    mut.unlock();

    if (!ret) {
        gcodeDestroy();
        return false;
    }

    gcodeDestroy();




#if 0

    if (Settings::optimizeRapidWays == true) {
        QTime t;
        t.start();
        //             g0points = getRapidPoints();
        QVector<int> ant = calculateAntPath();

        if (ant.count() > 2) {
            qDebug() << ant;

            sortGCode(ant);
        }

        emit logMessage(QString().sprintf("Read gcode, Ant optimization. Time elapsed: %d ms, cities: %d", t.elapsed(), ant.count()));
        //     qDebug() << "read gcode end";
        t.restart();
    }

#endif
    return true;
}



/**
 * @brief
 *
 */
QVector<GCodeData> *GData::dataVector()
{
    //     qDebug() << "return gcode data" << gCodeList.count();
    return &gCodeVector;
}

