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

#include "includes/GData.h"

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

//     angle = 0.0;
//     deltaAngle = 0.0;

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

    radius = 0.0; // got G02, G03

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

//     angle = 0.0; //d->angleVectors;

//     deltaAngle = 0.0;

    toolChange = d->toolChange;
    toolNumber = d->toolNumber;
    pauseMSec = d->pauseMSec;
    toolDiameter = d->toolDiameter;
};

