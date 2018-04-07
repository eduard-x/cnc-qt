/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
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


#include <QObject>
// #include <QRegExp>
#include <QDebug>
#include <QTime>
#include <QString>
#include <QDir>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/DataManager.h"
// #include "includes/MainWindow.h"


/******************************************************************************
** cDataManager
*/

//
// for debugging information of arc calculations
// to disable seto to 0
//

// QMutex mutex(QMutex::Recursive);
//
// units of messure, mm or inches
//
GerberData::GerberData()
{
    UnitsType = "";

    // длина всего числа
    countDigitsX = 1;
    // длина всего числа
    countDigitsY = 1;
    // длина дробной части
    countPdigX = 0;
    // длина дробной части
    countPdigY = 0;

    X_min = 100000;
    X_max = -100000;

    Y_min = 100000;
    Y_max = -100000;
}


//
// Вычисление размерности необходимого массива, для анализа
//
// accuracy: Коэфициент уменьшения размеров данных
void GerberData::CalculateGatePoints(int _accuracy)
{
    // немного уменьшим значения
    foreach (grbPoint VARIABLE, points) {
        VARIABLE.X = VARIABLE.X / _accuracy;
        VARIABLE.Y = VARIABLE.Y / _accuracy;
    }

    foreach (grbPoint VARIABLE, points) {
        if (VARIABLE.X > X_max) {
            X_max = VARIABLE.X;
        }

        if (VARIABLE.X < X_min) {
            X_min = VARIABLE.X;
        }

        if (VARIABLE.Y > Y_max) {
            Y_max = VARIABLE.Y;
        }

        if (VARIABLE.Y < Y_min) {
            Y_min = VARIABLE.Y;
        }
    }

    // Немного расширим границу
    X_max += 500;
    Y_max += 500;
}


// constructor
cDataManager::cDataManager()
{
    TypeFile = None;

    //     this->parent = parent;
}

// destructor
cDataManager::~cDataManager()
{
    g0Points.clear();

    goodList.clear();
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



QVector <GCodeOptim> cDataManager::getRapidPoints()
{
    return g0Points;
}

/**
 * @brief
 *
 */
QVector<QString> *cDataManager::stringList()
{
    return &goodList;
}

void cDataManager::gcodeChecker()
{
    // for Ant optimization
    g0Points.clear();

    goodList.clear();

    resetSoftLimits();

    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    QVector3D origin(0, 0, 0);
    QVector3D current_pos(0, 0, 0);

    // TODO home pos
    // goodList has the text for table
    g0Points << GCodeOptim {QVector3D(0, 0, 0), goodList.count(), -1, gCodeVector.count(), -1};

    // the first pos (cur = 0) is 0 or home
    for(int cur = 1; cur < gCodeVector.count(); cur++) {
        GCodeData d = gCodeVector.at(cur);

        if (d.decoded == false) {
            qInfo() << "not decoded line" << d.numberLine;
            continue;
        }

        QString line;

        if (d.gCmd >= 0) {
            detectMinMax(d);

            line = QString().sprintf("G%02d ", d.gCmd);

            if (Settings::filterRepeat == true) { // TODO
                if (gCodeVector.at(cur - 1).baseCoord.x() != d.baseCoord.x()) {
                    line += QString().sprintf("X%g ", d.baseCoord.x());
                }

                if (gCodeVector.at(cur - 1).baseCoord.y() != d.baseCoord.y()) {
                    line += QString().sprintf("Y%g ", d.baseCoord.y());
                }

                if (gCodeVector.at(cur - 1).baseCoord.z() != d.baseCoord.z()) {
                    line += QString().sprintf("Z%g ", d.baseCoord.z());
                }
            } else {
                line += QString().sprintf("X%g Y%g Z%g ", d.baseCoord.x(), d.baseCoord.y(), d.baseCoord.z());
            }

            switch (d.gCmd) {
                case 0: {
                    QVector3D delta_pos;

                    delta_pos = d.baseCoord - gCodeVector.at(cur - 1).baseCoord;

                    d.rapidVelo = 0.0;

                    if (b_absolute) {
                        current_pos = d.baseCoord + origin;
                    } else {
                        current_pos += d.baseCoord;
                    }

                    // TODO move to separate subroutine
                    // for the way optimizing
                    switch (d.plane) {
                        case XY: {
                            // xy moving
                            if (delta_pos.z() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.y() != 0.0)) {
                                if (gCodeVector.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // z moving
                            if (delta_pos.z() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.y() == 0.0)) {
                                if (gCodeVector.at(cur - 1).movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (cur);
                                }
                            }

                            break;
                        }

                        case YZ: {
                            if (delta_pos.x() == 0.0 && (delta_pos.y() != 0.0 || delta_pos.z() != 0.0)) {
                                // yz moving
                                if (gCodeVector.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (cur - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // x moving
                            if (delta_pos.x() != 0.0 && (delta_pos.y() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeVector.at(cur - 1).movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (cur);
                                }
                            }

                            break;
                        }

                        case ZX: {
                            if (delta_pos.y() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.z() != 0.0)) {
                                // zx moving
                                if (gCodeVector.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (cur - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // y moving
                            if (delta_pos.y() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeVector.at(cur - 1).movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (cur);
                                }
                            }

                            break;
                        }

                        default:
                            break;
                    }

                    break;
                }

                case 1: {
                    d.toolChange = false;
                    d.pauseMSec = -1; // no pause

                    if (b_absolute) {
                        current_pos = d.baseCoord + origin;
                    } else {
                        current_pos += d.baseCoord;
                    }

                    calcAngleOfLines(cur - 1);

                    break;
                }

                case 2:
                case 3: {
                    if (b_absolute) {
                        current_pos = d.baseCoord + origin;
                    } else {
                        current_pos += d.baseCoord;
                    }

                    if (d.useExtCoord == IJK) { //
                        line += QString().sprintf("I%g J%g K%g ", d.extCoord.x(), d.extCoord.y(), d.extCoord.z());
                    }

                    if (d.radius > 0) {
                        line += QString().sprintf("R%g ", d.radius);
                    }

                    convertArcToLines(d);

                    break;
                }

                case 4: {
                    break;
                }

                case 17:
                case 18:
                case 19:
                    break;

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
                    origin = gCodeVector.at(cur - 1).baseCoord - d.baseCoord;
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
            line += QString().sprintf("M%d ", d.mCmd);

            switch (d.mCmd) {
                case 0: {
                    d.pauseMSec = 0; // waiting
                    break;
                }

                case 2: {
                    break;
                }

                case 3: {
                    d.spindelOn = true;
                    break;
                }

                case 5: {
                    d.spindelOn = false;
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

            //             if (d.decoded == true) {
            //                 line += QString().sprintf("M%d ", d.mCmd);
            //             }
        }

        if (d.lineComment.length()) {
            line += d.lineComment;
        }

        if (line.length()) {
            goodList << line;
        }
    }
}


/**
 * @brief function patches the data list before sending to mk1
 *
 * the data list will be patched dependend from current user settings:
 * speed, steps per mm and other. we need to patch data in case of settings changing
 */
void cDataManager::fixGCodeList()
{
    if (gCodeVector.count() < 2) {
        return;
    }

    // grad to rad
    maxLookaheadAngleRad = Settings::maxLookaheadAngle * PI / 180.0;

    // calculate the number of steps in one direction, if exists
    for (int idx = 0; idx < gCodeVector.size(); idx++) {
        if (gCodeVector[idx].movingCode == RAPID_LINE_CODE) {
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
    foreach (const GCodeData d, gCodeList) {
        qDebug() << "line:" << d.numberLine << "accel:" << (hex) << d.movingCode << (dec) << "max coeff:" << d.vectorCoeff << "splits:" <<  d.splits
                 << "steps:" << d.stepsCounter << "vector speed:" << d.vectSpeed << "coords:" << d.X << d.Y << "delta angle:" << d.deltaAngle;
    }

    qDebug() << "max delta angle: " << PI - maxLookaheadAngleRad;
#endif
}


/**
 * @brief function set the vector speed and acceleration code before sending data to controller
 *
 * before sending data to microcontroller we need to calculate the vector speed and acceleration code
 * acceleration codes: ACCELERAT_CODE, DECELERAT_CODE, CONSTSPEED_CODE or FEED_LINE_CODE
 *
 * gCodeVector [begPos .. endPos]
 *
 * @param[in] begPos from this position in gcode list
 * @param[in] endPos inclusively end position
 *
 */
void cDataManager::patchSpeedAndAccelCode(int begPos, int endPos)
{
    if (begPos < 1 || begPos >= gCodeVector.count() - 1) {
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

    switch (gCodeVector.at(begPos).plane) {
        case XY: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {

                float dX = qFabs(gCodeVector.at(i - 1).baseCoord.x() - gCodeVector.at(i).baseCoord.x());
                float dY = qFabs(gCodeVector.at(i - 1).baseCoord.y() - gCodeVector.at(i).baseCoord.y());
                float dH = qSqrt(dX * dX + dY * dY);
                float coeff = 1.0;

                if (dX > dY) {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    // calculation of vect speed
                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeVector[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                } else {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeVector[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                }

                sumSteps += gCodeVector[i].stepsCounter;

                gCodeVector[i].vectorCoeff = coeff;
            }

            break;
        }

        case YZ: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dY = qFabs(gCodeVector.at(i - 1).baseCoord.y() - gCodeVector.at(i).baseCoord.y());
                float dZ = qFabs(gCodeVector.at(i - 1).baseCoord.z() - gCodeVector.at(i).baseCoord.z());
                float dH = qSqrt(dZ * dZ + dY * dY);
                float coeff = 1.0;

                if (dY > dZ) {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeVector[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                } else {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeVector[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                }

                sumSteps += gCodeVector[i].stepsCounter;

                gCodeVector[i].vectorCoeff = coeff;
            }

            break;
        }

        case ZX: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dZ = qFabs(gCodeVector.at(i - 1).baseCoord.z() - gCodeVector.at(i).baseCoord.z());
                float dX = qFabs(gCodeVector.at(i - 1).baseCoord.x() - gCodeVector.at(i).baseCoord.x());
                float dH = qSqrt(dX * dX + dZ * dZ);
                float coeff = 1.0;

                if (dZ > dX) {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeVector[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                } else {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    gCodeVector[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeVector[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                }

                sumSteps += gCodeVector[i].stepsCounter;

                gCodeVector[i].vectorCoeff = coeff;
            }

            break;
        }

        default: {
            qDebug() << "no plane information: pos " << begPos << "x" << gCodeVector[begPos].baseCoord.x() << "y" << gCodeVector[begPos].baseCoord.y() << "z" << gCodeVector[begPos].baseCoord.z();
        }
    }

    if (sumSteps > 0) {
        // now for steps
        for (int i = begPos; i < endPos; i++) {
            int tmpStps;
            tmpStps = gCodeVector[i].stepsCounter;
            gCodeVector[i].stepsCounter = sumSteps;
            sumSteps -= tmpStps;
            gCodeVector[i].movingCode = CONSTSPEED_CODE;
        }

        gCodeVector[begPos].movingCode = ACCELERAT_CODE;
        gCodeVector[endPos].movingCode = DECELERAT_CODE;
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

    if (startPos > gCodeVector.count() - 1 || startPos < 1) {
        qDebug() << "steps counter bigger than list";
        return -1;
    }

#if 1

    if (gCodeVector.at(startPos).splits > 0) { // it's arc, splits inforamtion already calculated
        idx += gCodeVector.at(startPos).splits;
        return idx;
    }

#endif

    // or for lines
    for (idx = startPos; idx < gCodeVector.count() - 1; idx++) {
#if 1

        if (gCodeVector.at(idx).movingCode == ACCELERAT_CODE && gCodeVector.at(idx).splits > 0) {
            idx += gCodeVector.at(idx).splits;
            return idx;
        }

#endif

        if (gCodeVector.at(idx + 1).movingCode == RAPID_LINE_CODE) {
            return idx;
        }

#if 0
        qDebug() << "found diff accel code" << startPos << idx << (hex) << gCodeList.at(idx).movingCode << gCodeList[idx + 1].movingCode
                 << "coordinates" << (dec) << gCodeList.at(idx).X << gCodeList.at(idx).Y << gCodeList[idx + 1].X << gCodeList[idx + 1].Y;
#endif

        float a1 = gCodeVector.at(idx).angle;
        float a2 = gCodeVector.at(idx + 1).angle;

        gCodeVector[idx].deltaAngle = (a1 - a2);

        if (qFabs(gCodeVector.at(idx).deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
            break;
        }
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
    int points = g0Points.count();

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
            distance[i][j] = g0Points.at(j).coord.distanceToPoint(g0Points.at(i).coord);
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
    int points = g0Points.count();

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
    QVector<QString> tmpList; // for the display list
    QVector<GCodeData> tmpGCodeList; // for the visualisation


    for(int n = 0; n < citydata.size(); n++) {
        int pos = citydata.at(n);
        int startNum = g0Points.at(pos).lineBeg;
        int endNum = g0Points.at(pos).lineEnd;

        for (int j = startNum; j <= endNum; j++) {
            tmpList << goodList.at(j);
        }

        startNum = g0Points.at(pos).gcodeBeg;
        endNum = g0Points.at(pos).gcodeEnd;

        for (int j = startNum; j <= endNum; j++) {
            tmpGCodeList << gCodeVector.at(j);
        }

        //         qDebug() << "pos" << pos << "lines:" << startNum << ".." << endNum - 1 << goodList.at(endNum) << g0Points.at(citydata.at(n)).coord;
        //         startNum = endNum;
    }

    mut.lock();

    goodList.clear();

    goodList = tmpList;

    gCodeVector.clear();

    gCodeVector = tmpGCodeList;

    mut.unlock();
    //     for  (int n = 0; n < citydata.size(); n++) {
    //         int ln = g0Points.at(citydata.at(n)).line;
    //         endNum =
    //         qDebug() << "line:" << ln << goodList.at(ln) << g0Points.at(citydata.at(n)).coord;
    //     }
}


/**
 * @brief
 *
 */
bool cDataManager::addLine(GCodeData *c)
{
}


/**
 * @brief
 *
 */
bool cDataManager::addArc(GCodeData *c)
{
}

/**
 * @brief detect the min and max ranges
 *
 * @param[in] pos actual index in GCode data list, if pos is 0: init of min/max
 *
 */
void cDataManager::detectMinMax(const GCodeData &d)
{
    if (d.baseCoord.x() > Settings::coord[X].softLimitMax) {
        Settings::coord[X].softLimitMax = d.baseCoord.x();
    }

    if (d.baseCoord.x() < Settings::coord[X].softLimitMin) {
        Settings::coord[X].softLimitMin = d.baseCoord.x();
    }

    if (d.baseCoord.y() > Settings::coord[Y].softLimitMax) {
        Settings::coord[Y].softLimitMax = d.baseCoord.y();
    }

    if (d.baseCoord.y() < Settings::coord[Y].softLimitMin) {
        Settings::coord[Y].softLimitMin = d.baseCoord.y();
    }

    if (d.baseCoord.z() > Settings::coord[Z].softLimitMax) {
        Settings::coord[Z].softLimitMax = d.baseCoord.z();
    }

    if (d.baseCoord.z() < Settings::coord[Z].softLimitMin) {
        Settings::coord[Z].softLimitMin = d.baseCoord.z();
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
void cDataManager::calcAngleOfLines(int pos)
{
    if (pos < 1 || pos > gCodeVector.count() - 1) {
        return;
    }

    switch (gCodeVector.at(pos).plane) {
        case XY: {
            gCodeVector[pos].angle = qAtan2(gCodeVector.at(pos).baseCoord.y() - gCodeVector.at(pos - 1).baseCoord.y(), gCodeVector.at(pos).baseCoord.x() - gCodeVector.at(pos - 1).baseCoord.x());
            break;
        }

        case YZ: {
            gCodeVector[pos].angle = qAtan2(gCodeVector.at(pos).baseCoord.z() - gCodeVector.at(pos - 1).baseCoord.z(), gCodeVector.at(pos).baseCoord.y() - gCodeVector.at(pos - 1).baseCoord.y());
            break;
        }

        case ZX: {
            gCodeVector[pos].angle = qAtan2(gCodeVector.at(pos).baseCoord.x() - gCodeVector.at(pos - 1).baseCoord.x(), gCodeVector.at(pos).baseCoord.z() - gCodeVector.at(pos - 1).baseCoord.z());
            break;
        }

        default: {
            qDebug() << "calcAngleOfLines(): no plane information";
            break;
        }
    }

    if (gCodeVector[pos].angle < 0.0) {
        gCodeVector[pos].angle += 2.0 * PI;
    }
}


/**
 * @brief this function converts the arc to short lines: mk1 do not support the arc commands
 *
 * @param d pointer to the list with decoded coordinates of endpoint
 *
 */
void cDataManager::convertArcToLines(GCodeData &d)
{
    if (gCodeVector.count() == 0) {
        return;
    }

    if (!(d.gCmd == 2 || d.gCmd == 3) ) { // it's not arc
        return;
    }

    GCodeData &begData = gCodeVector.last();
    // arcs
    // translate points to arc
    float r = 0.0; // length of sides
    //     float x2, x1, y2, y1, z2, z1;
    QVector3D beginPos, endPos;

    beginPos = begData.baseCoord;
    endPos = d.baseCoord;

    float i, j, k;
    i = d.extCoord.x(); // IJK
    j = d.extCoord.y();
    k = d.extCoord.z();

    //     QVector3D pos1 = begData; //(x1, y1, z1);
    //     QVector3D pos2(x2, y2, z2);

    float deltaPos = 0.0;
    float begPos = 0.0;

    switch (d.plane) {
        case XY: {
            if (d.radius == 0.0) {
                r = qSqrt(qPow(beginPos.x() - i, 2) + qPow(beginPos.y() - j, 2));
            } else {
                r = d.radius;
                // compute i, j
                //                 float a = determineAngle (pos1, pos2, d.plane) + PI;
                //                 qDebug() << "radius " << r << "alpha" << a << "xy point 1" << x1 << y1 << "xy point 2" << x2 << y2;
            }

            deltaPos = endPos.z() - beginPos.z();
            begPos = beginPos.z();
        }
        break;

        case YZ: {
            if (d.radius == 0.0) {
                r = qSqrt(qPow(beginPos.y() - j, 2) + qPow(beginPos.z() - k, 2));
            } else {
                r = d.radius;
                // compute j, k
            }

            deltaPos = endPos.x() - beginPos.x();
            begPos = beginPos.x();
        }
        break;

        case ZX: {
            if (d.radius == 0.0) {
                r = qSqrt(qPow(beginPos.z() - k, 2) + qPow(beginPos.x() - i, 2));
            } else {
                r = d.radius;
                // compute k, i
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

    QVector3D posC(i, j, k);

    alpha_beg = determineAngle (beginPos, posC, d.plane);
    alpha_end = determineAngle (endPos, posC, d.plane);

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

    float n = (bLength * Settings::splitsPerMm)/* - 1*/; // num segments of arc per mm
    float splitLen = 1.0 / (float)Settings::splitsPerMm;

    if ( n <= 1.0) {
        n++;
        qDebug() << "warning, n = " << n << alpha_beg << alpha_end << bLength << r << alpha << beginPos.x() << beginPos.y() << endPos.x() << endPos.y();
        //         return;
    }

    float dAlpha = alpha / n;

    deltaPos = deltaPos / n;

    if (d.gCmd == 3) {
        dAlpha = -dAlpha;
    }

#if DEBUG_ARC
    QString dbg;
#endif
    float angle = alpha_beg;
    float loopPos = begPos;

    // copy of parsed endpoint
    GCodeData *ncommand = new GCodeData(d);

#if DEBUG_ARC
    qDebug() << "arc from " << begData.X << begData.Y << begData.Z  << "to" << d.X << d.Y << d.Z << "splits: " << n;
#endif

    QVector<GCodeData> tmpList;

    ncommand->baseCoord = begData.baseCoord;
    ncommand->extCoord = begData.extCoord; // ABC
    //     ncommand.Z = begData.Z;
    //     ncommand.A = begData.A;

    detectMinMax(ncommand);

    ncommand->splits = n;
    ncommand->movingCode = ACCELERAT_CODE;

    // now split
    switch (d.plane) {
        case XY: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                angle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(angle);
                float s = qSin(angle);

                float x_new = i + r * c;
                float y_new = j + r * s;

                float angle = qAtan2(y_new - ncommand->baseCoord.y(), x_new - ncommand->baseCoord.x());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->baseCoord = {x_new, y_new, loopPos};

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d x=%f y=%f angle=%f qSin=%f qCos=%f\n", step, x_new, y_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - d.baseCoord.x()) * (x_new - d.baseCoord.x()) + (y_new - d.baseCoord.y()) * (y_new - d.baseCoord.y())) <= splitLen) {
                    float t_angle = qAtan2(y_new - d.baseCoord.y(), x_new - d.baseCoord.x());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->baseCoord = d.baseCoord;
                    //                     ncommand.Y = d.Y;
                    //                     ncommand.Z = d.Z;

                    detectMinMax(ncommand);

                    n = step;

                    tmpList << *ncommand;

                    break;
                }

                tmpList << *ncommand;
                ncommand = new GCodeData(*ncommand);

                ncommand->movingCode = CONSTSPEED_CODE;
                ncommand->splits = 0;
            }
        }
        break;

        case YZ: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                angle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(angle);
                float s = qSin(angle);

                float y_new = j + r * c;
                float z_new = k + r * s;

                float angle = qAtan2(z_new - ncommand->baseCoord.z(), y_new - ncommand->baseCoord.y());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->baseCoord = {loopPos, y_new, z_new};
                //                 ncommand->X = loopPos;

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d y=%f z=%f angle=%f qSin=%f qCos=%f\n", step, y_new, z_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((y_new - d.baseCoord.y()) * (y_new - d.baseCoord.y()) + (z_new - d.baseCoord.z()) * (z_new - d.baseCoord.z())) <= splitLen) {
                    float t_angle = qAtan2(z_new - d.baseCoord.z(), y_new - d.baseCoord.y());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->baseCoord = d.baseCoord;

                    detectMinMax(ncommand);

                    n = step;

                    tmpList << *ncommand;

                    break;
                }

                tmpList << *ncommand;
                ncommand = new GCodeData(*ncommand);

                ncommand->movingCode = CONSTSPEED_CODE;
                ncommand->splits = 0;
            }
        }
        break;

        case ZX: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                angle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(angle);
                float s = qSin(angle);

                float z_new = k + r * c;
                float x_new = i + r * s;

                float angle = qAtan2(x_new - ncommand->baseCoord.x(), z_new - ncommand->baseCoord.z());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->baseCoord = {x_new, loopPos, z_new};
                //                 ncommand->X = x_new;
                //                 ncommand->Y = loopPos;

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d z=%f x=%f angle=%f qSin=%f qCos=%f\n", step, z_new, x_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - d.baseCoord.x()) * (x_new - d.baseCoord.x()) + (z_new - d.baseCoord.z()) * (z_new - d.baseCoord.z())) <= splitLen) {
                    float t_angle = qAtan2(x_new - d.baseCoord.x(), z_new - d.baseCoord.z());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->baseCoord = d.baseCoord;

                    detectMinMax(ncommand);

                    n = step;

                    tmpList << *ncommand;

                    break;
                }

                tmpList << *ncommand;

                ncommand = new GCodeData(*ncommand);
                ncommand->movingCode = CONSTSPEED_CODE;
                ncommand->splits = 0;
            }
        }
        break;

        default:
            qDebug() << "no plane info!";
            break;
    }


    if (tmpList.length() > 0) {
        tmpList[tmpList.length() - 1].movingCode = DECELERAT_CODE;
        tmpList[0].splits = n;
    }

    gCodeVector += (tmpList);

    tmpList.clear();

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
        qInfo() << "cannot open " << fileName;
        return false;
    }

    qint64 sz = file.size();

    if (sz > MAX_FILE_SIZE) {
        return false;
    }

    QByteArray arr = file.readAll();
    QByteArray detectArray = arr.left(1024); // first 1024 bytes for format detection

    file.close();

    TypeFile = None;

    if ((detectArray.indexOf("G0") >= 0) || (detectArray.indexOf("G1") >= 0)) { // G-Code program detect
        TypeFile = GCODE;
        QTime tMess;
        tMess.start();

        bool res = readGCode(arr.data());

        emit logMessage(QString().sprintf("Parse gcode, flex/bison. Time elapsed: %d ms", tMess.elapsed()));
        // the parsed data is in gCodeList

        tMess.restart();

        gcodeChecker();

        emit logMessage(QString().sprintf("Data post processed. Time elapsed: %d ms, lines parsed: %d", tMess.elapsed(), goodList.count()));

        if (Settings::optimizeRapidWays == true) {
            tMess.restart();
            //             g0points = getRapidPoints();
            QVector<int> ant = calculateAntPath();

            if (ant.count() > 2) {
                qDebug() << ant;

                sortGCode(ant);
            }

            emit logMessage(QString().sprintf("Read gcode, Ant optimization. Time elapsed: %d ms, cities: %d", tMess.elapsed(), ant.count()));
        }

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

    if ((detectArray.indexOf("G04 ") >= 0) && (detectArray.indexOf("%MOMM*%") > 0 || detectArray.indexOf("%MOIN*%") > 0) ) { // extended gerber
        TypeFile = GBR;
        bool res = readGBR(arr);


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
    GerberData grb;

    //
    int numberSplineNow = -1;
    int X = 0;
    int Y = 0;

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

    //System.Diagnostics.Process proc = System.Diagnostics.Process.Start("mspaint.exe", "d:\sample.bmp"); //Запускаем блокнот
    //proc.WaitForExit();//и ждем, когда он завершит свою работу
}

#endif
