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
#include <QDebug>
#include <QTime>
#include <QString>
#include <QDir>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/DataManager.h"



#define DEBUG_ARC 0


// constructor
cDataManager::cDataManager()
{
    TypeFile = None;

    //     this->parent = parent;
}

// destructor
cDataManager::~cDataManager()
{
    gCities.clear();

    dataVector.clear();
    filteredList.clear();
    originalList.clear();
}

// void cDataManager::lock() const
// {
//  mutex.lock();
// }
//
//
// void cDataManager::unlock() const
// {
//  mutex.unlock();
// }



// QVector <GCodeOptim> cDataManager::getRapidPoints()
// {
//     return gCities;
// }

/**
 * @brief
 *
 */
QVector<QString> *cDataManager::getFilteredList()
{
    return &filteredList;
}

/**
 * @brief
 *
 */
QVector<QString> *cDataManager::getOriginalList()
{
    return &originalList;
}


QVector<SerialData*> *cDataManager::getSerialVector()
{
    return &serialDataVector;
}

void cDataManager::checkCity(const QVector3D &current_pos, int pos)
{
    ParserData d = dataVector.at(pos);
    QVector3D delta_pos;
    delta_pos = d.coord - dataVector.at(pos - 1).coord;

    switch (currentMCmd->plane) {
        case XY: {
            // xy moving
            if (delta_pos.z() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.y() != 0.0)) {
                if (dataVector.at(pos - 1).gCmd == 0) {
                    gCities.last().lineEndFilter = (filteredList.count() - 1 );
                    gCities.last().lineEndOrig = (d.numberLine - 1 );
                    gCities.last().serialEnd = (pos - 1);
                    gCities << GCodeOptim {current_pos, d.numberLine, -1, filteredList.count(), -1, pos, -1};
                }

                break;
            }

            // z moving
            if (delta_pos.z() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.y() == 0.0)) {
                if (dataVector.at(pos - 1).gCmd != 0) {
                    gCities.last().serialEnd = (pos);
                }
            }

            break;
        }

        case YZ: {
            if (delta_pos.x() == 0.0 && (delta_pos.y() != 0.0 || delta_pos.z() != 0.0)) {
                // yz moving
                if (dataVector.at(pos - 1).gCmd == 0) {
                    gCities.last().lineEndFilter = (filteredList.count() - 1 );
                    gCities.last().lineEndOrig = (d.numberLine - 1 );
                    gCities.last().serialEnd = (pos - 1);
                    gCities << GCodeOptim {current_pos, d.numberLine, -1, filteredList.count(), -1, pos, -1};
                }

                break;
            }

            // x moving
            if (delta_pos.x() != 0.0 && (delta_pos.y() == 0.0 && delta_pos.z() == 0.0)) {
                if (dataVector.at(pos - 1).gCmd != 0) {
                    gCities.last().serialEnd = (pos);
                }
            }

            break;
        }

        case ZX: {
            if (delta_pos.y() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.z() != 0.0)) {
                // zx moving
                if (dataVector.at(pos - 1).gCmd == 0) {
                    gCities.last().lineEndFilter = (filteredList.count() - 1 );
                    gCities.last().lineEndOrig = (d.numberLine - 1 );
                    gCities.last().serialEnd = (pos - 1);
                    gCities << GCodeOptim {current_pos, d.numberLine, -1, filteredList.count(), -1, pos, -1};
                }

                break;
            }

            // y moving
            if (delta_pos.y() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.z() == 0.0)) {
                if (dataVector.at(pos - 1).gCmd != 0) {
                    gCities.last().serialEnd = (pos);
                }
            }

            break;
        }

        default:
            break;
    }
}

/**
 * @brief check the parsed data and create the vector of SerialData
 * with calculated data for sending to mk1
 *
 */
void cDataManager::dataChecker()
{
    // for Ant optimization
    gCities.clear();

    filteredList.clear();
    serialDataVector.clear();

    resetSoftLimits();

    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    QVector3D origin(0, 0, 0);
    QVector3D current_pos(0, 0, 0);

    // TODO home pos
    // goodList has the text for table
    gCities << GCodeOptim {QVector3D(0, 0, 0), 0/*originalList.count()*/, -1, filteredList.count(), -1, dataVector.count(), -1};

    currentMCmd = new MData();

    currentMCmd->plane = XY;

    // the first pos (cur = 0) is 0 or home
    for(int cur = 1; cur < dataVector.count(); cur++) {
        ParserData d = dataVector.at(cur);

        if (d.decoded == false) {
            qInfo() << "not decoded line" << d.numberLine;
            continue;
        }

        QString filteredLine;

        SerialData *sTmp = 0;

        if (d.gCmd >= 0) {
            sTmp = new SerialData(d.coord);

            detectMinMax(sTmp->coord);

            filteredLine = QString().sprintf("G%02d ", d.gCmd);

            // filter duplicates
            if (Settings::filterRepeat == true) {
                if (dataVector.at(cur - 1).coord.x() != sTmp->coord.x()) {
                    filteredLine += QString().sprintf("X%g ", sTmp->coord.x());
                }

                if (dataVector.at(cur - 1).coord.y() != sTmp->coord.y()) {
                    filteredLine += QString().sprintf("Y%g ", sTmp->coord.y());
                }

                if (dataVector.at(cur - 1).coord.z() != sTmp->coord.z()) {
                    filteredLine += QString().sprintf("Z%g ", sTmp->coord.z());
                }
            } else {
                filteredLine += QString().sprintf("X%g Y%g Z%g ", sTmp->coord.x(), sTmp->coord.y(), sTmp->coord.z());
            }

            if (d.vParams.count() > 0) {
                foreach(addParam p, d.vParams) {
                    if (p.hasValue == false) {
                        filteredLine += QString().sprintf("%c ", (p.name & ~0x20));
                    } else {
                        filteredLine += QString().sprintf("%c%g ", (p.name & ~0x20), p.value);
                    }
                }
            }

            QVector3D delta_pos;
            delta_pos = d.coord - dataVector.at(cur - 1).coord;

            if (b_absolute) {
                current_pos = d.coord + origin;
            } else {
                current_pos += d.coord;
            }

            switch (d.gCmd) {
                case 0: {
                    sTmp->movingCode = RAPID_LINE_CODE;

                    // for the way optimizing
                    checkCity(current_pos, cur);

                    break;
                }

                case 1: {
                    sTmp->movingCode = FEED_LINE_CODE;

                    break;
                }

                case 2:
                case 3: {
                    sTmp->movingCode = FEED_LINE_CODE;

                    if (d.useExtCoord == IJK) { //
                        if (d.extCoord.x() != 0.0) {
                            filteredLine += QString().sprintf("I%g ", d.extCoord.x());
                        }

                        if (d.extCoord.y() != 0.0) {
                            filteredLine += QString().sprintf("J%g ", d.extCoord.y());
                        }

                        if (d.extCoord.z() != 0.0) {
                            filteredLine += QString().sprintf("K%g ", d.extCoord.z());
                        }
                    }

                    foreach(addParam p, d.vParams) {
                        if (p.name == 'r') {
                            filteredLine += QString().sprintf("R%g ", p.value);
                            break;
                        }
                    }

                    convertArcToLines(cur);

                    break;
                }

                case 4: { //
                    break;
                }

                case 17: {
                    MData *m = new MData();
                    m->plane = XY;
                    currentMCmd->plane = XY;
                    sTmp->pMCommand = m;
                    break;
                }

                case 18: {
                    MData *m = new MData();
                    m->plane = YZ;
                    currentMCmd->plane = YZ;
                    sTmp->pMCommand = m;
                    break;
                }

                case 19: {
                    MData *m = new MData();
                    m->plane = ZX;
                    currentMCmd->plane = ZX;
                    sTmp->pMCommand = m;
                    break;
                }

                case 20: {
                    coef = 25.4;
                    break;
                }

                case 21: {
                    coef = 1.0;
                    break;
                }

                case 28: {
                    break;
                }

                case 90: {
                    b_absolute = true;
                    break;
                }

                case 91: {
                    b_absolute = false;
                    break;
                }

                case 92: {
                    origin = dataVector.at(cur - 1).coord - d.coord;
                    break;
                }

                // Home axes to minimum
                case 161: {
                    break;
                }

                // Home axes to maximum
                case 162: {
                    break;
                }

                default: {
                    emit logMessage(QString().sprintf("Not decoded line %d G command %d", d.numberLine, d.gCmd));
                    d.decoded = false;
                    break;
                }
            }
        } // end of g decoding

        // the m part
        if (d.mCmd >= 0 ) {
            filteredLine += QString().sprintf("M%d ", d.mCmd);

            if (!sTmp) {
                sTmp = new SerialData(d.coord);
            }

            sTmp->pMCommand = new MData();

            switch (d.mCmd) {
                case 0: {
                    //                     sTmp->pauseMSec = 0; // waiting
                    break;
                }

                case 2: {
                    break;
                }

                case 3: {
                    sTmp->pMCommand->spindelOn = 1;
                    break;
                }

                case 5: {
                    sTmp->pMCommand->spindelOn = 0;
                    break;
                }

                case 6: { // TODO with two params
                    break;
                }

                // Disable all stepper motors
                case 18: {
                    break;
                }

                // Turn extruder 1 on (Forward), Undo Retraction
                case 101: {
                    break;
                }

                // Turn extruder 1 on (Reverse)
                case 102: {
                    break;
                }

                // Turn all extruders off, Extruder Retraction
                case 103: {
                    break;
                }

                // Set Extruder Temperature
                case 104: {
                    break;
                }

                // Get Extruder Temperature
                case 105: {
                    break;
                }

                // Set Extruder Speed (BFB)
                case 108: {
                    break;
                }

                // Set Extruder Temperature and Wait
                case 109: {
                    break;
                }

                // Set Extruder PWM
                case 113: {
                    break;
                }

                //
                case 132: {
                    break;
                }

                case 206: {
                    break;
                }

                default: {
                    emit logMessage(QString().sprintf("Not decoded line %d, M command %d", d.numberLine, d.mCmd));
                    d.decoded = false;
                    break;
                }
            }
        }

        if (d.decoded == true) {
            if (sTmp) {
                serialDataVector << sTmp;
            }

            if (d.lineComment.length()) {
                if (Settings::filterRepeat == false) {
                    filteredLine += d.lineComment;
                }
            }

            if (filteredLine.length()) {
                filteredList << filteredLine;
                filteredLine.clear();
            }
        }
    }
}


/**
 * @brief function patches the data list before sending to mk1
 *
 * the data list will be patched dependend from current user settings:
 * speed, steps per mm and other. we need to patch data in case of settings changing
 */
void cDataManager::fixSerialList()
{
    if (serialDataVector.count() < 2) {
        return;
    }

    // grad to rad
    maxLookaheadAngleRad = Settings::maxLookaheadAngle * PI / 180.0;

    qInfo() << "fixSerialList, list size" << serialDataVector.size();

    // calculate the number of steps in one direction, if exists
    for (int idx = 0; idx < serialDataVector.size(); idx++) {
        if (serialDataVector.at(idx)->movingCode == RAPID_LINE_CODE || serialDataVector.at(idx)->movingCode == NO_CODE) {
            continue;
        }

        int endPos = calculateMinAngleSteps(idx); // and update the pos

        if (endPos >= 1) {
            patchSpeedAndAccelCode(idx, endPos);
            idx = endPos;
            continue;
        }
    }

#if 0

    // now debug
    foreach (const ParserData d, gCodeList) {
        qDebug() << "line:" << d.numberLine << "accel:" << (hex) << d.movingCode << (dec) << "max coeff:" << d.vectorCoeff << "splits:" <<  d.arcSplits
                 << "steps:" << d.stepsCounter << "vector speed:" << d.vectSpeed << "coords:" << d.X << d.Y << "delta angle:" << d.deltaAngle;
    }

    qDebug() << "max delta angle: " << PI - maxLookaheadAngleRad;
#endif
}

/**
 * @brief check pointer to MCommand struct
 *        and check the changes in device management
 */
void cDataManager::checkMCommand(const SerialData &s)
{
    if (s.pMCommand == 0) {
        return;
    }

    // plane to change?
    if(s.pMCommand->plane != PlaneEnum::None) {
        currentMCmd->plane = s.pMCommand->plane;
    }

    if (s.pMCommand->coolantOn != -1) {
        currentMCmd->coolantOn = s.pMCommand->coolantOn;
    }

    if (s.pMCommand->mistOn != -1) {
        currentMCmd->mistOn = s.pMCommand->mistOn;
    }

    if (s.pMCommand->toolChange != -1) {
        currentMCmd->toolChange = s.pMCommand->toolChange;
    }

    if (s.pMCommand->spindelOn != -1) {
        currentMCmd->spindelOn = s.pMCommand->spindelOn;
    }

    if (s.pMCommand->toolNumber != currentMCmd->toolNumber) {
        currentMCmd->toolNumber = s.pMCommand->toolNumber;
    }

    if (s.pMCommand->toolDiameter != currentMCmd->toolDiameter) {
        currentMCmd->toolDiameter = s.pMCommand->toolDiameter;
    }

    if (s.pMCommand->rapidVelo != currentMCmd->rapidVelo) {
        currentMCmd->rapidVelo = s.pMCommand->rapidVelo;
    }

    if (s.pMCommand->pauseMSec >= 0) {
        currentMCmd->pauseMSec = s.pMCommand->pauseMSec;
    }

}

/**
 * @brief function set the vector speed and acceleration code before sending data to controller
 *
 * before sending data to microcontroller we need to calculate the vector speed and acceleration code
 * acceleration codes: ACCELERAT_CODE, DECELERAT_CODE, CONSTSPEED_CODE or FEED_LINE_CODE
 *
 * dataVector [begPos .. endPos]
 *
 * @param[in] begPos from this position in gcode list
 * @param[in] endPos inclusively end position
 *
 */
void cDataManager::patchSpeedAndAccelCode(int begPos, int endPos)
{
    if (begPos < 1 || begPos >= dataVector.count() - 1) {
        qDebug() << "wrong position number patchSpeedAndAccelCode()" << begPos;
        return;
    }

    if (begPos == endPos) {
        return;
    }

    int sumSteps = 0;

    float dnewSpdX  = 3600; // 3584?
    float dnewSpdY  = 3600; // 3584?
    float dnewSpdZ  = 3600; // 3584?

    // TODO to calculate this only after settings changing
    if ((Settings::coord[X].maxVeloLimit != 0.0) && (Settings::coord[X].pulsePerMm != 0.0)) {
        dnewSpdX = 7.2e8 / ((float)Settings::coord[X].maxVeloLimit * Settings::coord[X].pulsePerMm);
    }

    if ((Settings::coord[Y].maxVeloLimit != 0.0) && (Settings::coord[Y].pulsePerMm != 0.0)) {
        dnewSpdY = 7.2e8 / ((float)Settings::coord[Y].maxVeloLimit * Settings::coord[Y].pulsePerMm);
    }

    if ((Settings::coord[Z].maxVeloLimit != 0.0) && (Settings::coord[Z].pulsePerMm != 0.0)) {
        dnewSpdZ = 7.2e8 / ((float)Settings::coord[Z].maxVeloLimit * Settings::coord[Z].pulsePerMm);
    }

    checkMCommand(*serialDataVector[begPos]);

    switch (currentMCmd->plane) {
        case XY: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dX = qFabs(serialDataVector.at(i - 1)->coord.x() - serialDataVector.at(i)->coord.x());
                float dY = qFabs(serialDataVector.at(i - 1)->coord.y() - serialDataVector.at(i)->coord.y());
                float dH = qSqrt(dX * dX + dY * dY);
                float coeff = 1.0;

                if (dX > dY) {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    // calculation of vect speed
                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdX); //
                    serialDataVector[i]->stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                } else {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdY); //
                    serialDataVector[i]->stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                }

                sumSteps += serialDataVector[i]->stepsCounter;

                serialDataVector[i]->vectorCoeff = coeff;
            }

            break;
        }

        case YZ: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dY = qFabs(serialDataVector.at(i - 1)->coord.y() - serialDataVector.at(i)->coord.y());
                float dZ = qFabs(serialDataVector.at(i - 1)->coord.z() - serialDataVector.at(i)->coord.z());
                float dH = qSqrt(dZ * dZ + dY * dY);
                float coeff = 1.0;

                if (dY > dZ) {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdY); //
                    serialDataVector[i]->stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                } else {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdZ); //
                    serialDataVector[i]->stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                }

                sumSteps += serialDataVector[i]->stepsCounter;

                serialDataVector[i]->vectorCoeff = coeff;
            }

            break;

        }

        case ZX: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dZ = qFabs(serialDataVector.at(i - 1)->coord.z() - serialDataVector.at(i)->coord.z());
                float dX = qFabs(serialDataVector.at(i - 1)->coord.x() - serialDataVector.at(i)->coord.x());
                float dH = qSqrt(dX * dX + dZ * dZ);
                float coeff = 1.0;

                if (dZ > dX) {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdZ); //
                    serialDataVector[i]->stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                } else {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    serialDataVector[i]->vectSpeed = (int)(coeff * dnewSpdX); //
                    serialDataVector[i]->stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                }

                sumSteps += serialDataVector[i]->stepsCounter;

                serialDataVector[i]->vectorCoeff = coeff;
            }

            break;
        }

        default: {
            qDebug() << "no plane information: pos " << begPos << "x" << serialDataVector[begPos]->coord.x() << "y" << serialDataVector[begPos]->coord.y() << "z" << serialDataVector[begPos]->coord.z();
        }
    }

    if (sumSteps > 0) {
        // now for steps
        for (int i = begPos; i < endPos; i++) {
            int tmpStps;
            tmpStps = serialDataVector[i]->stepsCounter;
            serialDataVector[i]->stepsCounter = sumSteps;
            sumSteps -= tmpStps;
            serialDataVector[i]->movingCode = CONSTSPEED_CODE;
        }

        serialDataVector[begPos]->movingCode = ACCELERAT_CODE;
        serialDataVector[endPos]->movingCode = DECELERAT_CODE;
    }
}


/**
 * @brief function determines, how many steps from actual position the g-code object has to the last point with angle up to maxLookaheadAngleRad
 *
 * the angle maxLookaheadAngle is recommended from 150 to 179 grad. it well be converted to radians
 *
 * @param[in] startPos begin pos of searching
 *
 * @return the end position of polygon with angle difference less than maxLookaheadAngle
 */
int cDataManager::calculateMinAngleSteps(int startPos)
{
    int idx = startPos;

    if (startPos > serialDataVector.count() - 1 || startPos < 1) {
        qDebug() << "steps counter bigger than list: " << startPos << serialDataVector.count();
        return -1;
    }

#if 0

    if (serialDataVector.at(startPos).arcData.count() > 0) { // it's arc, splits inforamtion already calculated
        //         qInfo() << "arc , pos" << startPos;
        //         idx += serialDataVector.at(startPos).arcSplits;
        return idx + 1;
    }

#endif

    // or for lines
    for (idx = startPos; idx < serialDataVector.count() - 1; idx++) {
#if 0

        // if ARC
        if (serialDataVector.at(idx).arcData.count() > 0) {
            //             qInfo() << "arc found , pos" << idx;
            //             idx += serialDataVector.at(idx).arcSplits;
            // TODO check the angles of enter and end points
            return idx + 1;
        }

#endif

        if (serialDataVector.at(idx + 1)->movingCode == RAPID_LINE_CODE) {// RAPID_LINE_CODE) {
            return idx;
        }

#if 0
        qDebug() << "found diff accel code" << startPos << idx << (hex) << gCodeList.at(idx).movingCode << gCodeList[idx + 1].movingCode
                 << "coordinates" << (dec) << gCodeList.at(idx).X << gCodeList.at(idx).Y << gCodeList[idx + 1].X << gCodeList[idx + 1].Y;
#endif

        checkMCommand(*serialDataVector[idx]);

        // line to line
        if ((serialDataVector.at(idx)->movingCode == RAPID_LINE_CODE) && (serialDataVector.at(idx + 1)->movingCode == RAPID_LINE_CODE)) {
            qDebug() << "line to line" << idx << serialDataVector.at(idx - 1)->coord << serialDataVector.at(idx)->coord << serialDataVector.at(idx + 1)->coord;
            float a1 = calcAngleOfLines(serialDataVector.at(idx - 1)->coord, serialDataVector.at(idx)->coord, currentMCmd->plane);
            float a2 = calcAngleOfLines(serialDataVector.at(idx)->coord, serialDataVector.at(idx + 1)->coord, currentMCmd->plane);

            float deltaAngle = (a1 - a2);

            if (qFabs(deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
                break;
            }

            continue;
        }

#if 0

        // arc to line
        if ((serialDataVector.at(idx).arcData.count() > 0) && (serialDataVector.at(idx + 1).gCmd == 1)) {
            int lastPos = serialDataVector.at(idx).arcData.count() - 1;
            qDebug() << "arc to line, zeile" << serialDataVector.at(idx).numberLine << serialDataVector.at(idx).arcData.at(lastPos - 1) << serialDataVector.at(idx).arcData.at(lastPos) << serialDataVector.at(idx + 1)->coord;
            float a1 = calcAngleOfLines(serialDataVector.at(idx).arcData.at(lastPos - 1).coord, serialDataVector.at(idx).arcData.at(lastPos).coord, serialDataVector.at(idx).plane);
            float a2 = calcAngleOfLines(serialDataVector.at(idx).arcData.at(lastPos).coord, serialDataVector.at(idx + 1)->coord, serialDataVector.at(idx).plane);

            float deltaAngle = (a1 - a2);

            if (qFabs(deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
                break;
            }

            continue;
        }

        // arc to arc
        if ((serialDataVector.at(idx).arcData.count() > 0) && (serialDataVector.at(idx + 1).arcData.count() > 0)) {
            int lastPos = serialDataVector.at(idx).arcData.count() - 1;
            qDebug() << "arc to arc, zeile" << serialDataVector.at(idx).numberLine << serialDataVector.at(idx).arcData.at(lastPos - 1) << serialDataVector.at(idx).arcData.at(lastPos) << serialDataVector.at(idx + 1).arcData.at(0);
            float a1 = calcAngleOfLines(serialDataVector.at(idx).arcData.at(lastPos - 1).coord, serialDataVector.at(idx).arcData.at(lastPos).coord, serialDataVector.at(idx).plane);
            float a2 = calcAngleOfLines(serialDataVector.at(idx + 1).arcData.at(0).coord, serialDataVector.at(idx + 1).arcData.at(1).coord, serialDataVector.at(idx).plane);

            float deltaAngle = (a1 - a2);

            if (qFabs(deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
                break;
            }

            continue;
        }

        // line to arc
        if ((serialDataVector.at(idx).gCmd == 1) && (serialDataVector.at(idx + 1).arcData.count() > 0)) {
            int lastPos = serialDataVector.at(idx).arcData.count() - 1;
            qDebug() << "line to arc, zeile" << serialDataVector.at(idx).numberLine << serialDataVector.at(idx - 1)->coord << serialDataVector.at(idx)->coord << serialDataVector.at(idx + 1).arcData.at(0);
            float a1 = calcAngleOfLines(serialDataVector.at(idx - 1)->coord, serialDataVector.at(idx)->coord, serialDataVector.at(idx).plane);
            float a2 = calcAngleOfLines(serialDataVector.at(idx + 1).arcData.at(0).coord, serialDataVector.at(idx + 1).arcData.at(1).coord, serialDataVector.at(idx).plane);

            float deltaAngle = (a1 - a2);

            if (qFabs(deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
                break;
            }

            continue;
        }

#endif
    }

#if 0

    if ((idx - startPos) != 0) {
        gCodeList[startPos].splits = idx - startPos;
        qDebug() << "found in pos:" << startPos << ", steps: " << idx - startPos << " from" << gCodeList[startPos].X << gCodeList[startPos].Y  << "to" << gCodeList[idx].X << gCodeList[idx].Y;// << dbg;
    }

#endif

    return idx;
}


const QVector<int> cDataManager::calculateAntPath(/*const QVector<GCodeOptim> &v*/)
{
    int points = gCities.count();

    path.clear();

    if (points <= 2) {
        return path;
    }

    path.resize(points);

    if (distance.size() > 0) {
        for (int i = 0; i < distance.size(); ++i) {
            distance[i].clear();
        }

        distance.clear();
    }

    distance.resize(points);

    for (int i = 0; i < distance.size(); ++i) {
        distance[i].resize(points);
    }

    // two dimensional array for distances between the points
    for (int i = 0; i < points; i++) {
        path[i] = i;

        for (int j = 0; j < points; j++) {
            distance[i][j] = gCities.at(j).coord.distanceToPoint(gCities.at(i).coord);
        }
    }

    antColonyOptimization();

    return path;
}

/**
 * @brief
 *
 * @see Ant Colony Optimization algorithm
 * @link https://hackaday.io/project/4955-g-code-optimization
 */
void cDataManager::antColonyOptimization()
{
    int points = gCities.count();

    int maxDepth = points;

    if (maxDepth > Settings::maxAntSearchDepth) {
        maxDepth = Settings::maxAntSearchDepth;
    }

    for (int i = 0; i < maxDepth - 2; i++) {
        for (int j = i + 2; j < points - 1; j++) {
            float swap_length = distance[path[i]][path[j]] + distance[path[i + 1]][path[j + 1]];
            float old_length = distance[path[i]][path[i + 1]] + distance[path[j]][path[j + 1]];

            if (swap_length < old_length) {
                // Make the new and shorter path.
                for (int x = 0; x < (j - i) / 2; x++) {
                    // swap
                    int temp = path[i + 1 + x];
                    path[i + 1 + x] = path[j - x];
                    path[j - x] = temp;
                }

                // recursively
                antColonyOptimization();
                //  say no to goto! ;) // goto START;
            }
        }
    }
}


/**
 *
 *
 */
void cDataManager::sortGCode(const QVector<int> &citydata)
{
    QVector<QString> tmpOrigList; // for the display list
    QVector<QString> tmpFilteredList; // for the display list
    QVector<ParserData> tmpGCodeList; // serial data
    QVector<VertexData> tmpVertexVector; // for the visualisation

    for(int n = 0; n < citydata.size(); n++) {
        int pos = citydata.at(n);
        int startNum = gCities.at(pos).lineBegOrig;
        int endNum = gCities.at(pos).lineEndOrig;

        for (int j = startNum; j <= endNum; j++) {
            tmpOrigList << originalList.at(j);
        }

        startNum = gCities.at(pos).lineBegFilter;
        endNum = gCities.at(pos).lineEndFilter;

        for (int j = startNum; j <= endNum; j++) {
            tmpFilteredList << filteredList.at(j);
        }

        startNum = gCities.at(pos).serialBeg;
        endNum = gCities.at(pos).serialEnd;

        for (int j = startNum; j <= endNum; j++) {
            tmpGCodeList << dataVector.at(j);
        }

        startNum = gCities.at(pos).vertexBeg;
        endNum = gCities.at(pos).vertexEnd;

        //         for (int j = startNum; j <= endNum; j++) {
        //             tmpVertexVector << vertexVector.at(j);
        //         }

        //         qDebug() << "pos" << pos << "lines:" << startNum << ".." << endNum - 1 << goodList.at(endNum) << gCities.at(citydata.at(n)).coord;
        //         startNum = endNum;
    }

    mut.lock();

    filteredList.clear();
    filteredList = tmpFilteredList;

    originalList.clear();
    originalList = tmpOrigList;

    dataVector.clear();
    dataVector = tmpGCodeList;

    mut.unlock();
    //     for  (int n = 0; n < citydata.size(); n++) {
    //         int ln = gCities.at(citydata.at(n)).line;
    //         endNum =
    //         qDebug() << "line:" << ln << goodList.at(ln) << gCities.at(citydata.at(n)).coord;
    //     }
}

#if 0
/**
 * @brief
 *
 */
bool cDataManager::addLine(ParserData *c)
{
}


/**
 * @brief
 *
 */
bool cDataManager::addArc(ParserData *c)
{
}
#endif

/**
 * @brief detect the min and max ranges
 *
 * @param[in] pos actual index in GCode data list, if pos is 0: init of min/max
 *
 */
void cDataManager::detectMinMax(const QVector3D &v)
{
    if (v.x() > Settings::coord[X].softLimitMax) {
        Settings::coord[X].softLimitMax = v.x();
    }

    if (v.x() < Settings::coord[X].softLimitMin) {
        Settings::coord[X].softLimitMin = v.x();
    }

    if (v.y() > Settings::coord[Y].softLimitMax) {
        Settings::coord[Y].softLimitMax = v.y();
    }

    if (v.y() < Settings::coord[Y].softLimitMin) {
        Settings::coord[Y].softLimitMin = v.y();
    }

    if (v.z() > Settings::coord[Z].softLimitMax) {
        Settings::coord[Z].softLimitMax = v.z();
    }

    if (v.z() < Settings::coord[Z].softLimitMin) {
        Settings::coord[Z].softLimitMin = v.z();
    }
}


/**
 * @brief calculate angle between two points
 *
 * @param[in] pos1 first point
 * @param[in] pos2 second point
 *
 */
float cDataManager::determineAngle(const QVector3D &pos1, const QVector3D &pos2, PlaneEnum pl)
{
    float radians = 0.0;

    switch (pl) {
        case XY: {
            if (pos1[X] == pos2[X] && pos1[Y] == pos2[Y]) { // if diff is 0
                return 0.0;
            }

            radians = qAtan2(pos1[Y] - pos2[Y], pos1[X] - pos2[X]);

            break;
        }

        case YZ: {
            if (pos1[Y] == pos2[Y] && pos1[Z] == pos2[Z]) {
                return 0.0;
            }

            radians = qAtan2(pos1[Z] - pos2[Z], pos1[Y] - pos2[Y]);

            break;
        }

        case ZX: {
            if (pos1[Z] == pos2[Z] && pos1[X] == pos2[X]) {
                return 0.0;
            }

            radians = qAtan2(pos1[X] - pos2[X], pos1[Z] - pos2[Z]);

            break;
        }

        default:
            qDebug() << "not defined plane of arc";
            break;
    }

    if (radians < 0.0) {
        radians += 2.0 * PI;
    }

    return radians;
}


/**
 * @brief calculates the angle diffenerce between two points
 *
 * @param[in] pos the actual position
 *
 */
float cDataManager::calcAngleOfLines(const QVector3D &c1, const QVector3D &c2, int plane)
{
    //     if (pos < 1 || pos > dataVector.count() - 1) {
    //         return 0.0;
    //     }

    float angle;

    switch (plane) {
        case XY: {
            angle = qAtan2(c2.y() - c1.y(), c2.x() - c1.x());

            break;
        }

        case YZ: {
            angle = qAtan2(c2.z() - c1.z(), c2.y() - c1.y());
            break;
        }

        case ZX: {
            angle = qAtan2(c2.x() - c1.x(), c2.z() - c1.z());
            break;
        }

        default: {
            qDebug() << "calcAngleOfLines(): no plane information";
            break;
        }
    }

    if (angle < 0.0) {
        angle += 2.0 * PI;
    }

    return angle;
}


/**
 * @brief this function converts the arc to short lines: mk1 do not support the arc commands
 *
 * @param p is the current position in data vector
 *
 */
void cDataManager::convertArcToLines(int p)
{
    if (dataVector.count() == 0) {
        return;
    }

    ParserData &d = dataVector[p];
    ParserData &begData = dataVector[p - 1];

    if (!(d.gCmd == 2 || d.gCmd == 3) ) { // it's not arc
        return;
    }

    // arcs
    // translate points to arc
    float r = 0.0; // length of sides

    QVector3D beginPos, endPos;

    beginPos = begData.coord;
    endPos = d.coord;

    float i, j, k;

    float radius = 0.0;

    foreach(addParam p, d.vParams) {
        if (p.name == 'r' && p.value > 0) {
            radius = p.value;
            break;
        }
    }

    //     if (d.paramName == 'r' && d.paramValue > 0) {
    //         radius = d.paramValue;
    //     }

    if (radius == 0) {
        i = beginPos.x() + d.extCoord.x(); // IJK
        j = beginPos.y() + d.extCoord.y();
        k = beginPos.z() + d.extCoord.z();
    } else {
        float abs_radius;            /* absolute value of given radius */
        float half_length;           /* distance from M to end point   */
        float mid_x;                 /* first coordinate of M          */
        float mid_y;                 /* second coordinate of M         */
        float mid_z;                 /* second coordinate of M         */
        float offset;                /* distance from M to center      */
        float theta;                 /* angle of line from M to center */
        float turn2;                 /* absolute value of half of turn */

        abs_radius = fabs(radius);

        switch (currentMCmd->plane) {
            case XY: {
                mid_x = (endPos.x() + beginPos.x()) / 2.0;
                mid_y = (endPos.y() + beginPos.y()) / 2.0;
                half_length = hypot((mid_x - endPos.x()), (mid_y - endPos.y()));
#if 0

                if ((half_length / abs_radius) > (1 - TINY)) {
                    half_length = abs_radius;    /* allow a small error for semicircle */
                }

#endif

                /* check needed before calling asin   */
                if (((d.gCmd == 2) && (radius > 0)) || ((d.gCmd == 3) && (radius < 0))) {
                    theta = qAtan2((endPos.y() - beginPos.y()), (endPos.x() - beginPos.x())) - M_PI_2l;
                } else {
                    theta = qAtan2((endPos.y() - beginPos.y()), (endPos.x() - beginPos.x())) + M_PI_2l;
                }

                turn2 = qAsin(half_length / abs_radius);
                offset = abs_radius * qCos(turn2);
                i = mid_x + (offset * qCos(theta));
                j = mid_y + (offset * qSin(theta));

                k = 0.0;
                break;
            }

            case YZ: {
                mid_y = (endPos.y() + beginPos.y()) / 2.0;
                mid_z = (endPos.z() + beginPos.z()) / 2.0;
                half_length = hypot((mid_y - endPos.y()), (mid_z - endPos.z()));
#if 0

                if ((half_length / abs_radius) > (1 - TINY)) {
                    half_length = abs_radius;    /* allow a small error for semicircle */
                }

#endif

                /* check needed before calling asin   */
                if (((d.gCmd == 2) && (radius > 0)) || ((d.gCmd == 3) && (radius < 0))) {
                    theta = qAtan2((endPos.z() - beginPos.z()), (endPos.y() - beginPos.y())) - M_PI_2l;
                } else {
                    theta = qAtan2((endPos.z() - beginPos.z()), (endPos.y() - beginPos.y())) + M_PI_2l;
                }

                turn2 = qAsin(half_length / abs_radius);
                offset = abs_radius * qCos(turn2);
                j = mid_y + (offset * qCos(theta));
                k = mid_z + (offset * qSin(theta));

                i = 0.0;
                break;
            }

            case ZX: {
                mid_z = (endPos.z() + beginPos.z()) / 2.0;
                mid_x = (endPos.x() + beginPos.x()) / 2.0;
                half_length = hypot((mid_z - endPos.z()), (mid_x - endPos.x()));
#if 0

                if ((half_length / abs_radius) > (1 - TINY)) {
                    half_length = abs_radius;    /* allow a small error for semicircle */
                }

#endif

                /* check needed before calling asin   */
                if (((d.gCmd == 2) && (radius > 0)) || ((d.gCmd == 3) && (radius < 0))) {
                    theta = qAtan2((endPos.x() - beginPos.x()), (endPos.z() - beginPos.z())) - M_PI_2l;
                } else {
                    theta = qAtan2((endPos.x() - beginPos.x()), (endPos.z() - beginPos.z())) + M_PI_2l;
                }

                turn2 = qAsin(half_length / abs_radius);
                offset = abs_radius * qCos(turn2);
                k = mid_z + (offset * qCos(theta));
                i = mid_x + (offset * qSin(theta));

                j = 0.0;
                break;
            }
        }
    }

    QVector3D posCenter(i, j, k);

    float deltaPos = 0.0;
    float begPos = 0.0;

    switch (currentMCmd->plane) {
        case XY: {
            if (radius == 0.0) {
                r = qSqrt(qPow(beginPos.x() - posCenter.x(), 2) + qPow(beginPos.y() - posCenter.y(), 2));
            } else {
                r = radius;
            }

            deltaPos = endPos.z() - beginPos.z();
            begPos = beginPos.z();
        }
        break;

        case YZ: {
            if (radius == 0.0) {
                r = qSqrt(qPow(beginPos.y() - posCenter.y(), 2) + qPow(beginPos.z() - posCenter.z(), 2));
            } else {
                r = radius;
            }

            deltaPos = endPos.x() - beginPos.x();
            begPos = beginPos.x();
        }
        break;

        case ZX: {
            if (radius == 0.0) {
                r = qSqrt(qPow(beginPos.z() - posCenter.z(), 2) + qPow(beginPos.x() - posCenter.x(), 2));
            } else {
                r = radius;
            }

            deltaPos = endPos.y() - beginPos.y();
            begPos = beginPos.y();
        }
        break;

        default:
            break;
    }

    float alpha = 0.0;
    float alpha_beg, alpha_end;

    if (r == 0.0) {
        qDebug() << "wrong, r = 0";
        return;
    }


    alpha_beg = determineAngle (beginPos, posCenter, currentMCmd->plane);
    alpha_end = determineAngle (endPos, posCenter, currentMCmd->plane);

    if (d.gCmd == 2) {
        if (alpha_beg == alpha_end) {
            alpha_beg += 2.0 * PI;
        }

        alpha = alpha_beg - alpha_end;

        if (alpha_beg < alpha_end) {
            alpha = qFabs(alpha_beg + (2.0 * PI - alpha_end));
        }
    } else {
        if (alpha_beg == alpha_end) {
            alpha_end += 2.0 * PI;
        }

        alpha = alpha_end - alpha_beg;

        if (alpha_beg > alpha_end) {
            alpha = qFabs(alpha_end + (2.0 * PI - alpha_beg));
        }
    }

    float bLength = r * alpha;

    float n = (bLength * Settings::splitsPerMm + 0.5); // num steps for arc
    float splitLen = 1.0 / (float)Settings::splitsPerMm;

    if ( n < 2.0) { // arc is too short
        n = 2.0;
        qDebug() << "warning, n = " << n << alpha_beg << alpha_end << bLength << r << alpha << beginPos.x() << beginPos.y() << endPos.x() << endPos.y();
    }

    float dAlpha = alpha / n;

    deltaPos = deltaPos / n;

    if (d.gCmd == 2) {
        dAlpha = -dAlpha;
    }

#if DEBUG_ARC
    QString dbg;
#endif
    float runAngle = alpha_beg;
    float loopPos = begPos;


#if DEBUG_ARC
    qDebug() << "arc from " << begData.X << begData.Y << begData.Z  << "to" << d.X << d.Y << d.Z << "splits: " << n;
#endif

    SerialData *runSerial = new SerialData(d.coord);

    detectMinMax(runSerial->coord);

    //     d.arcSplits = n;
    runSerial->movingCode = ACCELERAT_CODE;

    // now split
    switch (currentMCmd->plane) {
        case XY: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                runAngle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(runAngle);
                float s = qSin(runAngle);

                float x_new = posCenter.x() + r * c;
                float y_new = posCenter.y() + r * s;

                float angle = qAtan2(y_new - runSerial->coord.y(), x_new - runSerial->coord.x());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                runSerial = new SerialData(QVector3D(x_new, y_new, loopPos));

                detectMinMax(runSerial->coord);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d x=%f y=%f angle=%f qSin=%f qCos=%f\n", step, x_new, y_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - endPos.x()) * (x_new - endPos.x()) + (y_new - endPos.y()) * (y_new - endPos.y())) <= splitLen) {
                    runSerial = new SerialData(endPos);

                    detectMinMax(runSerial->coord);

                    n = step;

                    serialDataVector << runSerial;

                    break;
                }

                serialDataVector << runSerial;
            }
        }
        break;

        case YZ: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                runAngle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(runAngle);
                float s = qSin(runAngle);

                float y_new = posCenter.y() + r * c;
                float z_new = posCenter.z() + r * s;

                float angle = qAtan2(z_new - runSerial->coord.z(), y_new - runSerial->coord.y());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                runSerial = new SerialData(QVector3D(loopPos, y_new, z_new));

                detectMinMax(runSerial->coord);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d y=%f z=%f angle=%f qSin=%f qCos=%f\n", step, y_new, z_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((y_new - endPos.y()) * (y_new - endPos.y()) + (z_new - endPos.z()) * (z_new - endPos.z())) <= splitLen) {
                    runSerial = new SerialData( endPos);

                    detectMinMax(runSerial->coord);

                    n = step;

                    serialDataVector << runSerial;

                    break;
                }

                serialDataVector << runSerial;
            }
        }
        break;

        case ZX: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                runAngle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(runAngle);
                float s = qSin(runAngle);

                float z_new = posCenter.z() + r * c;
                float x_new = posCenter.x() + r * s;

                float angle = qAtan2(x_new - runSerial->coord.x(), z_new - runSerial->coord.z());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                runSerial = new SerialData(QVector3D(x_new, loopPos, z_new));

                detectMinMax(runSerial->coord);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d z=%f x=%f angle=%f qSin=%f qCos=%f\n", step, z_new, x_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - endPos.x()) * (x_new - endPos.x()) + (z_new - endPos.z()) * (z_new - endPos.z())) <= splitLen) {
                    runSerial = new SerialData( endPos);

                    detectMinMax(runSerial->coord);

                    n = step;

                    serialDataVector << runSerial;

                    break;
                }

                serialDataVector << runSerial;
            }
        }
        break;

        default:
            qDebug() << "no plane info!";
            break;
    }


#if DEBUG_ARC

    if ((qFabs (x2 - gCodeList.last().X) > (bLength / splitsPerMm)) || (qFabs (y2 - gCodeList.last().Y) > (bLength / splitsPerMm))) { // wenn zu weit vom ziel...
        qDebug() << "begin: " << x1 << y1 << "end" << x2 << y2 << "center" << i << j;
        qDebug() << "bogen " << bLength << "mm" << "r" << r << "a" << a << "triangle alpha" << alpha;
        qDebug() << "alpha:" << alpha_beg << "->" << alpha_end << "d alpha: " << dAlpha; // rad
        qDebug() << dbg;
    }

#endif
}



void cDataManager::writeFile(const QString &fileName)
{
}


bool cDataManager::readFile(const QString &fileName)
{
    int pointPos = fileName.lastIndexOf(".");

    if (pointPos == -1) { // error
        return false;
    }

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
        //         qInfo() << "cannot open " << fileName;
        return false;
    }

    qint64 sz = file.size();

    if (sz > MAX_FILE_SIZE) {
        return false;
    }

    QByteArray arr = file.readAll();
    QByteArray detectArray = arr.left(4096); // first 4096 bytes for format detection

    file.close();

    TypeFile = None;

    if ((detectArray.indexOf("G0") >= 0) || (detectArray.indexOf("G1") >= 0)) { // G-Code program detect
        TypeFile = GCODE;
        QTime tMess;
        tMess.start();

        dataVector.clear();

        bool res = readGCode(arr.data());

        originalList.clear();
        originalList = QString(arr).split("\n").toVector();

        emit logMessage(QString().sprintf("Parse gcode, flex/bison. Time elapsed: %d ms", tMess.elapsed()));
        // the parsed data is in gCodeList

        tMess.restart();

        dataChecker();

        emit logMessage(QString().sprintf("Data post processed. Time elapsed: %d ms, lines parsed: %d", tMess.elapsed(), originalList.count()));

        if (Settings::optimizeRapidWays == true) {
            tMess.restart();
            //             g0points = geRapidPoints();
            QVector<int> ant = calculateAntPath();

            if (ant.count() > 2) {
                qDebug() << ant;

                sortGCode(ant);
            }

            emit logMessage(QString().sprintf("Read gcode, Ant optimization. Time elapsed: %d ms, cities: %d", tMess.elapsed(), ant.count()));
        }

        fixSerialList();

        arr.clear();

        return res;
    }

    if ((detectArray.indexOf("G04 ") >= 0) && (detectArray.indexOf("%MOMM*%") > 0 || detectArray.indexOf("%MOIN*%") > 0) ) { // extended gerber
        TypeFile = GBR;
        bool res = readGBR(arr);


        return res;
    }

    if ( detectArray.indexOf("IN1;") >= 0 ) { // plotter format
        TypeFile = PLT;
        bool res = readPLT(arr);

        return res;
    }

    if ( detectArray.indexOf("<svg") >= 0 ) { // svg
        TypeFile = SVG;
        bool res = readSVG(arr);

        return res;
    }

    if ( detectArray.indexOf("") >= 0 ) { // eps
        TypeFile = EPS;
        bool res = readEPS(arr);

        return res;
    }

    if ( detectArray.indexOf("") >= 0 ) { // polylines
        TypeFile = DXF;
        bool res = readDXF(arr);

        return res;
    }

    if ( detectArray.indexOf("") >= 0 ) { // excellon
        TypeFile = DRL;
        bool res = readDRL(arr);


        return res;
    }

    if (TypeFile == None) { // error
        // qmessagebox

    }

    return false;
}



void cDataManager::resetSoftLimits()
{
    Settings::coord[X].softLimitMax = 0;
    Settings::coord[X].softLimitMin = 0;
    Settings::coord[Y].softLimitMax = 0;
    Settings::coord[Y].softLimitMin = 0;
    Settings::coord[Z].softLimitMax = 0;
    Settings::coord[Z].softLimitMin = 0;
}


#if 1

bool cDataManager::readPLT( const QByteArray &arr )
{
    QList<Point> points;// = new QList<Point>();

    data.clear();
    //checkedListBox1.Items.clear();

    //  parent->treeView1.Nodes.clear();

    //  TreeNode trc;// = new TreeNode("");


    //  qDebug() << "анализ файла";

    int index = 0;

    QTextStream stream(arr);
    stream.setLocale(QLocale("C"));

    while (!stream.atEnd()) {
        QString s = stream.readLine();

        //      qDebug() << "анализ файла строка " + QString::number(index);
        //
        // begin point
        if (s.trimmed().mid(0, 2) == "PU") {
            int pos1 = s.indexOf('U');
            int pos2 = s.indexOf(' ');
            int pos3 = s.indexOf(';');

            float posX = s.mid(pos1 + 1, pos2 - pos1 - 1).toFloat();
            float posY = s.mid(pos2 + 1, pos3 - pos2 - 1).toFloat();

            // Пересчет в милиметры
            posX = posX / 40.0;
            posY = posY / 40.0;

            if (data.count() > 0) {
                //первый раз
                //   indexList++;
                //  } else {
                //   indexList++;
                //checkedListBox1.Items.Add("линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек");
                //   trc.Text = "линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек";

                //   data.last().Points <<  points;
                data << DataCollections(points);

                points.clear();
                //   points = new QList<Point>();

                //   treeView1.Nodes.Add(trc);
                //   trc = new TreeNode("");
            }

            points <<  (Point) {
                posX, posY
            };
        }

        //продолжение
        if (s.trimmed().mid(0, 2) == "PD") {
            int pos1 = s.indexOf('D');
            int pos2 = s.indexOf(' ');
            int pos3 = s.indexOf(';');

            float posX = s.mid(pos1 + 1, pos2 - pos1 - 1).toFloat();
            float posY = s.mid(pos2 + 1, pos3 - pos2 - 1).toFloat();

            // convert to mm
            posX = posX / 40.0;
            posY = posY / 40.0;

            points <<  (Point) {
                posX, posY
            };
            //  trc.Nodes.Add("Точка - X: " + QString::number(posX) + "  Y: " + QString::number(posY));

        }

        //      s = fs.ReadLine();
        index++;
    }

    //  fl.close();

    //  indexList++;
    //  Instument instr = {0, 0.0}; // number, diameter
    data <<  DataCollections(points);
    //checkedListBox1.Items.Add("линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек");
    //  trc.Text = "линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек";
    //  data <<  points;
    //  points = new QList<Point>();

    //  points.clear();

    //  treeView1.Nodes.Add(trc);
    //  trc = new TreeNode("");


    //  qDebug() << "загружено!!!!!!!!";
    //  fs = null;

    return true;

}


bool cDataManager::readSVG( const QByteArray &arr)
{
    return true;
}


bool cDataManager::readEPS( const QByteArray &arr)
{
    return true;
}


bool cDataManager::readDXF( const QByteArray &arr)
{
    return true;
}


bool cDataManager::readDRL( const QByteArray &arr)
{
    data.clear();

    QList<Point> points;

    //  StreamcDataManager fs = new StreamcDataManager(tbFile.Text);
    //  QString s = fs.ReadLine();

    bool isDataDrill = false; //определение того какие сейчас данные, всё что до строки с % параметры инструментов, после - дырки для сверлений

    DataCollections *dc = NULL;

    QTextStream stream(arr);
    stream.setLocale(QLocale("C"));

    while (!stream.atEnd()) {
        QString s = stream.readLine();

        if (s.trimmed().mid(0, 1) == "%") {
            isDataDrill = true;
        }

        if (!isDataDrill && s.trimmed().mid(0, 1) == "T") { //описание инструмента
            // На данном этапе в список добавляем интрумент, без точек сверловки
            int numInstrument = s.trimmed().mid(1, 2).toInt();

            int pos1 = s.indexOf('C');
            float diametr = s.mid(pos1 + 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toFloat();

            data << DataCollections(QList<Point>(), (Instrument) {
                numInstrument, diametr
            });
        }

        if (isDataDrill && s.trimmed().mid(0, 1) == "T") {
            //начало сверловки данным инструментом
            int numInstrument = s.trimmed().mid(1, 2).toInt();

            foreach (DataCollections VARIABLE, data) {
                if (VARIABLE.intrument.Number == numInstrument) {
                    dc = &VARIABLE;
                }
            }
        }

        if (isDataDrill && s.trimmed().mid(0, 1) == "X") {
            int pos1 = s.indexOf('X');
            int pos2 = s.indexOf('Y');

            float posX = s.mid(pos1 + 1, 7).toFloat() / 100.0;
            float posY = s.mid(pos2 + 1, 7).toFloat() / 100.0;

            dc->points <<  (Point) {
                posX, posY
            };
        }

        //      s = fs.ReadLine();
    }

    //  fs = null;
    //  fl.close();

    //  treeView1.Nodes.clear();
    //
    //  foreach (DataCollections VARIABLE, data) {
    //      TreeNode trc = new TreeNode("Сверловка - " + QString::number(VARIABLE.intrument.Diametr));
    //
    //      foreach (Point VARIABLE2, VARIABLE.Points) {
    //  trc.Nodes.Add("Точка - X: " + QString::number(VARIABLE2.X) + "  Y: " + QString::number(VARIABLE2.Y));
    //      }
    //      treeView1.Nodes.Add(trc);
    //  }

    //TreeNode trc = new TreeNode("");
    //qDebug() << "анализ файла";

    //int index = 0;
    //int indexList = -1;

    return true;

}

void cDataManager::Swap(int &p1, int &p2)
{
    int p = p1;
    p1 = p2;
    p2 = p;
}


//
// Заполнение в матрицы окружностью
//
// парам: arrayPoint Массив в котором делать
// парам: x0 центр круга по оси Х
// парам: y0 центр круга по оси Y
// парам: radius радиус круга
// парам: setvalue какое значение записывать в матрицу, если необходимо нарисовать точку окружности
// парам: needFill необходимость заполнить внутренность круга
//
void cDataManager::BresenhamCircle(QVector<QVector< quint8 > > &arrayPoint,  int x0, int y0, int radius, quint8 setvalue, bool needFill)
{
    int tmpradius = radius;

    while (tmpradius > 0) {

        int x = tmpradius;
        //int x = radius;
        int y = 0;
        int radiusError = 1 - x;

        while (x >= y) {
            arrayPoint[x + x0][y + y0] = setvalue;
            arrayPoint[y + x0][x + y0] = setvalue;
            arrayPoint[-x + x0][y + y0] = setvalue;
            arrayPoint[-y + x0][x + y0] = setvalue;
            arrayPoint[-x + x0][-y + y0] = setvalue;
            arrayPoint[-y + x0][-x + y0] = setvalue;
            arrayPoint[x + x0][-y + y0] = setvalue;
            arrayPoint[y + x0][-x + y0] = setvalue;
            y++;

            if (radiusError < 0) {
                radiusError += 2 * y + 1;
            } else {
                x--;
                radiusError += 2 * (y - x + 1);
            }
        }

        tmpradius--;
    }
}


void cDataManager::BresenhamLine(QVector<QVector<quint8> > &arrayPoint, int x0, int y0, int x1, int y1, typeSpline _Splane)
{
    //матрицу сплайна
    //  byte[,] spArray = new byte[1, 1];
    //  spArray[0, 0] = 1; //просто обычная точка
    QVector<QVector<quint8> > spArray;

    int sizeMatrixX = 0;
    int sizeMatrixY = 0;

    //но если это круг
    if (_Splane.aperture == C_circle) {
        sizeMatrixX = (int)(_Splane.size1 * 100);
        sizeMatrixY = sizeMatrixX;
    }


    // init of two dimensional array
    for (int y = 0; y <= sizeMatrixY; y++) {
        //matrixYline matrixline = new matrixYline();

        //matrixline.Y = numPosY->value() + (y* numStep->value());
        spArray.push_back(QVector< quint8 > ());

        for (int x = 0; x <= sizeMatrixX; x++) {
            //  parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
            quint8 v = 0;//{0.0, 0.0, 0.0, 0.0 };
            spArray[y].push_back(v);
            //matrixline.X.Add(new matrixPoint(numPosX->value() + (x * numStep->value()), numPosZ->value(), true));
        }

        //dataCode.Matrix.Add(matrixline);
    }


    if (_Splane.aperture == C_circle) {
        BresenhamCircle(spArray, sizeMatrixX / 2, sizeMatrixY / 2, sizeMatrixX / 2, 1, true);
    }

    //TODO: отладка с выводом в txt формат

    //string debugstr = "";
    //for (int dyy = 0; dyy < sizeMatrixY; dyy++)
    //{
    //    for (int dxx = 0; dxx < sizeMatrixX; dxx++)
    //    {
    //     debugstr += QString::number(spArray[dxx, dyy]);
    //    }
    //    debugstr += "\n";
    //}




    //if (x0 == x1 && y0 == y1) return;

    bool steep = qAbs(y1 - y0) > qAbs(x1 - x0); // Проверяем рост отрезка по оси икс и по оси игрек

    // Отражаем линию по диагонали, если угол наклона слишком большой
    if (steep) {
        Swap( x0,  y0); // Перетасовка координат вынесена в отдельную функцию для красоты
        Swap( x1,  y1);
    }

    // Если линия растёт не слева направо, то меняем начало и конец отрезка местами
    if (x0 > x1) {
        Swap( x0,  x1);
        Swap( y0,  y1);
    }

    int dx = x1 - x0;
    int dy = qAbs(y1 - y0);
    int error = dx / 2; // Здесь используется оптимизация с умножением на dx, чтобы избавиться от лишних дробей
    int ystep = (y0 < y1) ? 1 : -1; // Выбираем направление роста координаты y
    int y = y0;

    int sizeMatrixdX = 1;
    int sizeMatrixdY = 1;


    for (int x = x0; x <= x1; x++) {
        int possX = (steep ? y : x);
        int possY = (steep ? x : y);
        arrayPoint[possX][possY] = 2; //TODO: это нужно УБРАТЬ!!! Не забываем вернуть координаты на место

        //а тут нужно наложить матрицу на массив данных
        for (int xxx = 0; xxx < sizeMatrixX; xxx++) {
            for (int yyy = 0; yyy < sizeMatrixY; yyy++) {
                if (spArray[xxx][yyy] != 0) {
                    int pointX = possX + xxx - (sizeMatrixX / 2);
                    int pointY = possY + yyy - (sizeMatrixY / 2);

                    arrayPoint[pointX][pointY] = 2; // Не забываем вернуть координаты на место
                    //arrayPoint[possX + xxx - (sizeMatrixdX/2), possY + yyy - (sizeMatrixdY/2)] = 2; // Не забываем вернуть координаты на место
                }
            }
        }

        error -= dy;

        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}


//
// gerber reader
//
bool cDataManager::readGBR( const QByteArray &arr)
{
    //     GerberData grb;

    //
    int numberSplineNow = -1;
    int X = 0;
    int Y = 0;
#if 0
    QTextStream stream(arr);
    stream.setLocale(QLocale("C"));

    while (!stream.atEnd()) {
        QString s = stream.readLine();
        s = s.trimmed();

        if (s == "") {
            continue;    // пропутим пустую строку
        }

        // Извлечем тип единицы измерения
        if (s.mid(0, 3) == "%MO") {
            grb.UnitsType = s.mid(3, 2);
        }

        //извлечем параметры сплайна
        if (s.mid(0, 3) == "%AD") {
            int numb = s.mid(4, 2).toInt();
            QString letterAperture = s.mid(6, 1);

            //т.к. сплайны бывают разные, то и метод парсинга разный

            if (letterAperture == "C") { //если окружность
                int sstart = s.indexOf(",");
                int sEnd = s.indexOf("*");

                float sizeRound = s.mid(sstart + 1, sEnd - sstart - 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble();

                grb.typeSplines <<  (typeSpline) {
                    numb, C_circle, sizeRound
                };
            }

            if (letterAperture == "R") { //если прямоугольник
                int sstart1 = s.indexOf(",");
                int sstart2 = s.indexOf("X");
                int sEnd = s.indexOf("*");

                float sizeX = s.mid(sstart1 + 1, sstart2 - sstart1 - 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble();
                float sizeY = s.mid(sstart2 + 1, sEnd - sstart2 - 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble();

                grb.typeSplines << (typeSpline) {
                    numb, R_rectangle, sizeX, sizeY
                };
            }

            if (letterAperture == "O") { //если овал
                int sstart1 = s.indexOf(",");
                int sstart2 = s.indexOf("X");
                int sEnd = s.indexOf("*");

                float sizeX = s.mid(sstart1 + 1, sstart2 - sstart1 - 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble();
                float sizeY = s.mid(sstart2 + 1, sEnd - sstart2 - 1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble();

                grb.typeSplines <<  (typeSpline) {
                    numb, O_obround, sizeX, sizeY
                };
            }
        }

        //извлечение номера сплайна, которым будет рисовать
        if (s.mid(0, 1) == "D") {
            int posSYMBOL = s.indexOf("*");
            numberSplineNow = s.mid(1, posSYMBOL - 1).toInt();
        }

        //извлечение движения
        if (s.mid(0, 1) == "X" || s.mid(0, 1) == "Y") {
            int posX = s.indexOf("X");
            int posY = s.indexOf("Y");
            int posD = s.indexOf("D");

            QString sType = s.mid(posD, 2);

            if (posX != -1) {
                if (posY != -1) {
                    X = s.mid(posX + 1, posY - posX - 1).toInt();
                } else {
                    X = s.mid(posX + 1, posD - posX - 1).toInt();
                }
            }

            if (posY != -1) {
                Y = s.mid(posY + 1, posD - posY - 1).toInt();
            }

            grb.points <<  (grbPoint) {
                X, Y, sType, numberSplineNow
            };
        }

#if 0

        if (s.length() > 4 && s.trimmed().toUpper().mid(0, 5) == "%FSLA") {
            //параметры разбора чисел
            int pos1 = s.indexOf('X');
            int pos2 = s.indexOf('Y');
            int pos3 = s.indexOf('*');

            grb.countDigitsX = s.mid(pos1 + 1, 1).toInt();
            grb.countPdigX = s.mid(pos1 + 2, 1).toInt();
            grb.countDigitsY = s.mid(pos2 + 1, 1).toInt();
            grb.countPdigY = s.mid(pos2 + 2, 1).toInt();
        }

#endif
        //      s = fs.ReadLine();
    }


    // Вычислим границы данных, и уменьшим размерчик
    grb.CalculateGatePoints(10);

    //     qDebug() << "Наполнение массива";

    QVector<QVector<quint8> > arrayPoint;// = new byte[grb.X_max + 1, grb.Y_max + 1];

    for (int y = 0; y <= grb.Y_max; y++) {
        //matrixYline matrixline = new matrixYline();

        //matrixline.Y = numPosY->value() + (y* numStep->value());
        arrayPoint.push_back(QVector< quint8 > ());

        for (int x = 0; x <= grb.X_max; x++) {
            //  parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
            quint8 v = 0;//{0.0, 0.0, 0.0, 0.0 };
            arrayPoint[y].push_back(v);
            //matrixline.X.Add(new matrixPoint(numPosX->value() + (x * numStep->value()), numPosZ->value(), true));
        }

        //dataCode.Matrix.Add(matrixline);
    }

    int newX = 0;
    int newY = 0;

    int oldX = 0;
    int oldY = 0;

    typeSpline splaynNow = (typeSpline) {
        1, C_circle, 1, 1
    }; //по умолчанию просто точка

    foreach (grbPoint VARIABLE, grb.points) {
        //дополнительно получим характеристики сплайна
        foreach (typeSpline spl, grb.typeSplines) {
            if (spl.number == VARIABLE.numberSplane) {
                splaynNow = spl;
                break;
            }
        }

        newX = VARIABLE.X;
        newY = VARIABLE.Y;

        if (VARIABLE.typePoint == "D1") {
            BresenhamLine(arrayPoint, oldX, oldY, newX, newY, splaynNow);

            oldX = newX;
            oldY = newY;

        }

        if (VARIABLE.typePoint == "D2") {
            oldX = newX;
            oldY = newY;
        }

        if (VARIABLE.typePoint == "D3") {
            if (splaynNow.aperture == C_circle) {
                int sized = (int)(splaynNow.size1 * 100);

                BresenhamCircle( arrayPoint, newX, newY, sized / 2, 1, true);
            }

            if (splaynNow.aperture == R_rectangle) {
                int sized1 = (int)(splaynNow.size1 * 100) / 2;
                int sized2 = (int)(splaynNow.size2 * 100) / 2;

                for (int ix = -sized1; ix < sized1; ix++) {
                    for (int iy = -sized2; iy < sized2; iy++) {
                        arrayPoint[newX + ix][newY + iy] = 1;
                    }
                }
            }

            if (splaynNow.aperture == O_obround) {
                //TODO: для овала....
                //овал это линия из круглых сплейнов

                int sized1 = (int)(splaynNow.size1 * 100.0);
                int sized2 = (int)(splaynNow.size2 * 100.0);

                if (sized1 > sized2) {
                    //овал горизонтальный
                    int oX1 = newX - (sized1 / 2) + (sized2 / 2);
                    int oY1 = newY;
                    int oX2 = newX + (sized1 / 2) - (sized2 / 2);
                    int oY2 = newY;

                    typeSpline tps = (typeSpline) {
                        0, C_circle, (float)(sized2 / 100.0)
                    };
                    BresenhamLine(arrayPoint, oX1, oY1, oX2, oY2, tps);
                } else {
                    //овал вертикальный
                    int oX1 = newX;
                    int oY1 = newY - (sized2 / 2) + (sized1 / 2);
                    int oX2 = newX;
                    int oY2 = newY + (sized2 / 2) - (sized1 / 2);
                    typeSpline tps = (typeSpline) {
                        0, C_circle, (float)(sized1 / 100.0)
                    };
                    BresenhamLine(arrayPoint, oX1, oY1, oX2, oY2, tps);
                }
            }

        }
    }

#if 0
    qDebug() << "Помещение в BMP";

    Bitmap bmp = new Bitmap(grb.X_max + 1, grb.Y_max + 1, PixelFormat.Format16bppArgb1555);

    for (int x = 0; x < grb.X_max; x++) {
        for (int y = 0; y < grb.Y_max; y++) {
            if (arrayPoint[x][y] != 0) {
                bmp.SetPixel(x, y, Color.Black);
            }
        }
    }

    bmp.RotateFlip(RotateFlipType.RotateNoneFlipY);

    bmp.Save("d:\sample.png", ImageFormat.Png);
#endif
    //     qDebug() << "Готово!";

    return true;
    //  arrayPoint = null;
#endif
    //System.Diagnostics.Process proc = System.Diagnostics.Process.Start("mspaint.exe", "d:\sample.bmp"); //Запускаем блокнот
    //proc.WaitForExit();//и ждем, когда он завершит свою работу
}

#endif
