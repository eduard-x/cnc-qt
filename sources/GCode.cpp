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
    commandNum = 0;
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

    plane = None;

    radius = 0.0;
    vectorCoeff = 0.0;
    // end of arc

    rapidVelo = 0.0;

    typeMoving = NoType;

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

    rapidVelo = d->rapidVelo; // ???

    vectorCoeff = 0.0;

    typeMoving = d->typeMoving;

    spindelOn = d->spindelOn;
    mistOn = d->mistOn;
    coolantOn = d->coolantOn;

    vectSpeed = d->vectSpeed;

    splits = 0; // if arc, will be splitted, debug information only

    stepsCounter = 0; // should be calculated

    movingCode = NO_CODE;

    numberLine = d->numberLine;
    commandNum = 0;

    angle = 0.0;//d->angleVectors;

    deltaAngle = 0.0;

    toolChange = d->toolChange;
    toolNumber = d->toolNumber;
    pauseMSec = d->pauseMSec;
    toolDiameter = d->toolDiameter;
};

// is static
QVector<GCodeData> GCodeParser::gCodeList;

/**
 * @brief constructor
 *
 */
GCodeParser::GCodeParser()
{

}

/**
 * @brief destructor
 *
 */
GCodeParser::~GCodeParser()
{
    gCodeList.clear();
    g0Points.clear();
}

/**
 * @brief
 *
 */
bool GCodeParser::addLine(GCodeData *c)
{
}


/**
 * @brief
 *
 */
bool GCodeParser::addArc(GCodeData *c)
{
}


void GCodeParser::gcode_init()
{
    gcode_lineno = 0;
    //      gcode_result = NULL;
    //   gcode_vector = NULL;
    //   gcode_header = NULL;
}


bool GCodeParser::gcode_checker()
{
}


void GCodeParser::gcode_destroy()
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


void GCodeParser::resetSoftLimits()
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
bool GCodeParser::readGCode(char *indata)
{
    int ret = true;

    gCodeList.clear();
    goodList.clear();

    mut.lock();

    QTime tMess;
    tMess.start();

    gcode_init ();

    /* because the data in already in buffer 'indata' */

    YY_BUFFER_STATE bs = gcode__scan_string(indata);
    gcode__switch_to_buffer(bs);

    if ( gcode_parse () != 0) {
        ret = false;
    } else if ( gcode_checker () != 0) {
        ret = false;
    }

    gcode_lex_destroy ();

    mut.unlock();

    if (!ret) {
        gcode_destroy ();
        return false;
    }

    gcode_destroy ();

    emit logMessage(QString().sprintf("Parse gcode, flex/bison. Time elapsed: %d ms", tMess.elapsed()));

    tMess.restart();

    // the parsed data is in gCodeList
    qInfo() << "file parsed";

    g0Points.clear();

    resetSoftLimits();

    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    PlaneEnum currentPlane;
    currentPlane = XY;

    QVector3D origin(0, 0, 0);
    QVector3D current_pos(0, 0, 0);

    for(int cur = 0; cur < gCodeList.count(); cur++) {
        GCodeData d = gCodeList.at(cur);

        if (d.decoded == false) {
            emit logMessage(QString().sprintf("Not decoded line %d", d.numberLine));
            continue;
        }

        qInfo() << "line " << d.numberLine << d.gCmd;

        if (d.gCmd >= 0) {
            detectMinMax(d);

            switch (d.gCmd) {
                case 0: {
                    QVector3D delta_pos;
                    d.movingCode = RAPID_LINE_CODE;

                    if (cur >= 1) {
                        delta_pos = d.baseCoord - gCodeList.at(cur - 1).baseCoord;
                    } else {
                        delta_pos = QVector3D(0, 0, 0);
                    }

                    if (Settings::filterRepeat == true) { // TODO
                    }

                    d.typeMoving = GCodeData::Line;

                    d.movingCode = RAPID_LINE_CODE;
                    d.plane = currentPlane;
                    d.rapidVelo = 0.0;

                    if (b_absolute) {
                        current_pos = d.baseCoord + origin;
                    } else {
                        current_pos += d.baseCoord;
                    }

                    // TODO move to separate subroutine
                    // for the way optimizing
                    switch (currentPlane) {
                        case XY: {
                            // xy moving
                            if (delta_pos.z() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.y() != 0.0)) {
                                if (gCodeList.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // z moving
                            if (delta_pos.z() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.y() == 0.0)) {
                                if (gCodeList.at(cur - 1).movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (cur);
                                }
                            }

                            break;
                        }

                        case YZ: {
                            if (delta_pos.x() == 0.0 && (delta_pos.y() != 0.0 || delta_pos.z() != 0.0)) {
                                // yz moving
                                if (gCodeList.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (cur - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // x moving
                            if (delta_pos.x() != 0.0 && (delta_pos.y() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeList.at(cur - 1).movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (cur);
                                }
                            }

                            break;
                        }

                        case ZX: {
                            if (delta_pos.y() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.z() != 0.0)) {
                                // zx moving
                                if (gCodeList.at(cur - 1).movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (cur - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, cur, -1};
                                }

                                break;
                            }

                            // y moving
                            if (delta_pos.y() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeList.at(cur - 1).movingCode != RAPID_LINE_CODE) {
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
                    d.movingCode = FEED_LINE_CODE;
                    d.typeMoving = GCodeData::Line;

                    d.plane = currentPlane;
                    d.rapidVelo = 0.0;

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
                    if (d.gCmd == 2) {
                        d.typeMoving = GCodeData::ArcCW;
                    } else {
                        d.typeMoving = GCodeData::ArcCCW;
                    }

                    d.movingCode = FEED_LINE_CODE;

                    if (b_absolute) {
                        current_pos = d.baseCoord + origin;
                    } else {
                        current_pos += d.baseCoord;
                    }

                    convertArcToLines(d);

                    break;
                }

                case 4: {
                    break;
                }

                case 17: {
                    currentPlane = XY;
                    break;
                }

                case 18: {
                    currentPlane = YZ;
                    break;
                }

                case 19: {
                    currentPlane = ZX;
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
                    origin = gCodeList.at(cur - 1).baseCoord - d.baseCoord;
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
                    qInfo() << "g is not decoded" <<  d.gCmd << d.numberLine;
                    emit logMessage(QString().sprintf("Not decoded line %d G command %d", d.numberLine, d.gCmd));
                    d.decoded = false;
                    break;
                }
            }
        } else {
            switch (d.mCmd) {
                case 0: {
                    d.pauseMSec = 0; // waiting
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
                    qInfo() << "m is not decoded" <<  d.gCmd << d.numberLine;
                    emit logMessage(QString().sprintf("Not decoded line %d, M command %d", d.numberLine, d.mCmd));
                    d.decoded = false;
                    break;
                }
            }
        }
    }

    emit logMessage(QString().sprintf("Data was converted. Time elapsed: %d ms, lines parsed: %d", tMess.elapsed(), goodList.count()));

    return true;
}


/**
 *
 *
 */
void GCodeParser::sortGCode(const QVector<int> &citydata)
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
            tmpGCodeList << gCodeList.at(j);
        }

        //         qDebug() << "pos" << pos << "lines:" << startNum << ".." << endNum - 1 << goodList.at(endNum) << g0Points.at(citydata.at(n)).coord;
        //         startNum = endNum;
    }

    mut.lock();

    goodList.clear();

    goodList = tmpList;

    gCodeList.clear();

    gCodeList = tmpGCodeList;

    mut.unlock();
    //     for  (int n = 0; n < citydata.size(); n++) {
    //         int ln = g0Points.at(citydata.at(n)).line;
    //         endNum =
    //         qDebug() << "line:" << ln << goodList.at(ln) << g0Points.at(citydata.at(n)).coord;
    //     }
}

/**
 * @brief detect the min and max ranges
 *
 * @param[in] pos actual index in GCode data list, if pos is 0: init of min/max
 *
 */
void GCodeParser::detectMinMax(const GCodeData &d)
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
float GCodeParser::determineAngle(const QVector3D &pos1, const QVector3D &pos2, PlaneEnum pl)
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
void GCodeParser::calcAngleOfLines(int pos)
{
    if (pos < 1 || pos > gCodeList.count() - 1) {
        return;
    }

    switch (gCodeList.at(pos).plane) {
        case XY: {
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).baseCoord.y() - gCodeList.at(pos - 1).baseCoord.y(), gCodeList.at(pos).baseCoord.x() - gCodeList.at(pos - 1).baseCoord.x());
            break;
        }

        case YZ: {
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).baseCoord.z() - gCodeList.at(pos - 1).baseCoord.z(), gCodeList.at(pos).baseCoord.y() - gCodeList.at(pos - 1).baseCoord.y());
            break;
        }

        case ZX: {
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).baseCoord.x() - gCodeList.at(pos - 1).baseCoord.x(), gCodeList.at(pos).baseCoord.z() - gCodeList.at(pos - 1).baseCoord.z());
            break;
        }

        default: {
            qDebug() << "calcAngleOfLines(): no plane information";
            break;
        }
    }

    if (gCodeList[pos].angle < 0.0) {
        gCodeList[pos].angle += 2.0 * PI;
    }
}


/**
 * @brief this function converts the arc to short lines: mk1 do not support the arc commands
 *
 * @param d pointer to the list with decoded coordinates of endpoint
 *
 */
void GCodeParser::convertArcToLines(GCodeData &d)
{
    if (gCodeList.count() == 0) {
        return;
    }

    if (!(d.typeMoving == GCodeData::ArcCW || d.typeMoving == GCodeData::ArcCCW) ) { // it's not arc
        return;
    }

    GCodeData &begData = gCodeList.last();
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

    if (d.typeMoving == GCodeData::ArcCW) {
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

    if (d.typeMoving == GCodeData::ArcCW) {
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

    gCodeList += (tmpList);

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


/**
 * @brief
 *
 */
QVector<QString> GCodeParser::getGoodList()
{
    return goodList;
}

QVector <GCodeOptim> GCodeParser::getRapidPoints()
{
    return g0Points;
}

/**
 * @brief
 *
 */
QVector<GCodeData> GCodeParser::getGCodeData()
{
    //     qDebug() << "return gcode data" << gCodeList.count();
    return gCodeList;
}

