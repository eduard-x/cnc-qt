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
#include <QRegExp>
#include <QDebug>
// #include <QTime>
#include <QString>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/MainWindow.h"


#define DEBUG_ARC 0

/**
 * @brief consructor
 *
 */
GCodeData::GCodeData()
{
    changeInstrument = false;
    numberInstrument = 0;
    numberInstruction = 0;
    numberLine = 0;
    /** @var pauseMSeconds
     * no pause: -1
     * waiting: 0
     * pause > 0 in milliseconds
     */
    pauseMSeconds = -1;

    xyz = { 0.0, 0.0, 0.0 };
    ijk = { 0.0, 0.0, 0.0 };
    abc = { 0.0, 0.0, 0.0 };
    uvw = { 0.0, 0.0, 0.0 };
    //     for(int i=0; i< 16; i++){
    //         axis[i] = 0.0;
    //     }
    //     X = 0.0;
    //     Y = 0.0;
    //     Z = 0.0;
    //     A = 0.0;
    //     B = 0.0;
    //     C = 0.0;
    //
    //     // arc parameters
    //     I = 0.0;
    //     J = 0.0;
    //     K = 0.0;

    plane = None;

    Radius = 0.0;
    vectorCoeff = 0.0;
    // end of arc

    typeMoving = NoType;

    movingCode =  NO_CODE;

    vectSpeed = 0.0;

    stepsCounter = 0;

    angle = 0.0;
    deltaAngle = 0.0;

    spindelON = false;
    splits = 0; // init

    diametr = 0.0;
};


/**
 * @brief constructor based on command
 *
 */
GCodeData::GCodeData(GCodeData *d)
{
    xyz = d->xyz;
    ijk = { 0.0, 0.0, 0.0 };
    abc = { 0.0, 0.0, 0.0 };
    uvw = { 0.0, 0.0, 0.0 };
    //     for(int i=A; i< 16; i++){
    //         axis[i] = 0.0;
    //     }
    //     axis[X] = d->axis[X];
    //     axis[Y] = d->axis[Y];
    //     axis[Z] = d->axis[Z];
    //     A = d->A;
    //     B = d->B;
    //     C = d->C;
    //
    //     I = d->I;
    //     J = d->J;
    //     K = d->K;

    Radius = d->Radius;

    plane = d->plane;

    vectorCoeff = 0.0;

    typeMoving = d->typeMoving;

    spindelON = d->spindelON;
    vectSpeed = d->vectSpeed;

    splits = 0; // if arc, will be splitted, debug information only
    stepsCounter = 0; // should be calculated

    movingCode = NO_CODE;

    numberLine = d->numberLine;
    numberInstruction = 0;

    angle = 0.0;//d->angleVectors;

    deltaAngle = 0.0;

    changeInstrument = d->changeInstrument;
    numberInstrument = d->numberInstrument;
    pauseMSeconds = d->pauseMSeconds;
    diametr = d->diametr;
};


/**
 * @brief constructor
 *
 */
GCodeParser::GCodeParser()
{

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


/**
 * @brief read and parse into GCodeData list and OpenGL list
 * @see for the optimizations see https://blog.qt.io/blog/2014/06/13/qt-weekly-13-qstringliteral/
 * TODO convert QString to QStringLiteral
 *
 */
bool GCodeParser::readGCode(const QByteArray &gcode)
{
    mut.lock();

    goodList.clear();

    QTextStream stream(gcode);
    stream.setLocale(QLocale("C"));
    // or this ? QString.split(QRegExp("\n|\r\n|\r"));
    // if we switch the input methode from QTextStream to QStringList, performance is about 15% higher

    QVector3D origin(0, 0, 0);
    QVector3D current_pos(0, 0, 0);
    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    QTime t;
    t.start();

    bool decoded;

    QVector<QString> gCodeLines;
    QString lastCmd;
    int lineNr = 0;
    //     QString lastCommand;
    QString param[16];//X, paramY, paramZ, paramA, paramF;

    while(!stream.atEnd()) { // restruct lines
        QString lineStream = stream.readLine().toUpper();
        lineNr++;

        // ignore commentars
        if (lineStream.isEmpty()/* || lineStream.at(0) == ';' || lineStream.at(0) == '(' */ || lineStream.at(0) == '%') {
            continue;
        }

        int posComment = lineStream.indexOf(";");

        if (posComment >= 0) {
            lineStream = lineStream.mid(posComment);

            if (lineStream.isEmpty()) {
                continue;
            }
        }

        // this is commentar too : ( ... )
        int commentBeg = lineStream.indexOf('(');
        int commentEnd = -1;

        if (commentBeg >= 0) {
            commentEnd = lineStream.lastIndexOf(')');

            if (commentEnd > commentBeg) {
                lineStream = lineStream.remove(commentBeg, commentEnd - commentBeg + 1);
            }
        }

        if (lineStream.length() == 0) {
            continue;
        }

        QRegExp rx("([A-Z])((\\-)?(\\+)?\\d+(\\.\\d+)?)");
        int pos = 0;
        QString tmpStr;

        if (Settings::filterRepeat == true) {
            while ((pos = rx.indexIn(lineStream, pos)) != -1) {
                QString currentText = rx.cap(0);
                QChar c = currentText.at(0);

                if (c == 'N') { // ignore line number
                    pos += rx.matchedLength();
                    continue;
                }

                if (pos == 0) {
                    if (currentText == "G1" || currentText == "G01") {
                        param[PARAM_CMD] = currentText;
                    } else {
                        param[PARAM_CMD].clear();
                        param[PARAM_X].clear();
                        param[PARAM_Y].clear();
                        param[PARAM_Z].clear();
                        param[PARAM_I].clear();
                        param[PARAM_J].clear();
                        param[PARAM_K].clear();
                        param[PARAM_A].clear();
                        param[PARAM_B].clear();
                        param[PARAM_C].clear();
                        param[PARAM_F].clear();
                    }

                    pos += rx.matchedLength();
                } else {
                    // when last command exists
                    pos += rx.matchedLength();

                    if (param[PARAM_CMD].length() > 0) {
                        if (c == 'X') {
                            if (currentText == param[PARAM_X]) {
                                continue;
                            } else {
                                param[PARAM_X] = currentText;
                            }
                        }

                        if (c == 'Y') {
                            if (currentText == param[PARAM_Y]) {
                                continue;
                            } else {
                                param[PARAM_Y] = currentText;
                            }
                        }

                        if (c == 'Z') {
                            if (currentText == param[PARAM_Z]) {
                                continue;
                            } else {
                                param[PARAM_Z] = currentText;
                            }
                        }

                        if (c == 'A') {
                            if (currentText == param[PARAM_A]) {
                                continue;
                            } else {
                                param[PARAM_A] = currentText;
                            }
                        }

                        if (c == 'F') {
                            if (currentText == param[PARAM_F]) {
                                continue;
                            } else {
                                param[PARAM_F] = currentText;
                            }
                        }
                    }
                }

                tmpStr += currentText;
                tmpStr += " ";
            }
        } else {
            while ((pos = rx.indexIn(lineStream, pos)) != -1) {
                QChar c = rx.cap(0).at(0);

                pos += rx.matchedLength();

                if (c == 'N') { // ignore line number
                    continue;
                }

                tmpStr += rx.cap(0);
                tmpStr += " ";
            }
        }

        if (tmpStr.length() == 0) {
            emit logMessage(QString("gcode parqSing error: " + lineStream));
            //             badList << lineStream;
            continue;
        }

        QChar c = tmpStr.at(0);

        if (!(c == 'G' || c == 'M' || c == 'F')) {
            if (lastCmd.length() > 0) {
                tmpStr = QString(lastCmd + " " + tmpStr);
            } else {
                emit logMessage(QString("gcode parqSing error: " + lineStream));
                //                 badList << QString::number(lineNr - 1) + ": " + lineStream;
            }
        } else {
            int posSpace = tmpStr.indexOf(" ");

            if (posSpace > 0) { // command with parameter
                if (posSpace == 2) { // insert '0' if two characters
                    tmpStr.insert(1, "0");
                    posSpace++;
                }

                lastCmd = tmpStr.left(posSpace);

            } else { // command without parameter
                if (tmpStr.length() == 2) { // insert '0' if two characters
                    tmpStr.insert(1, "0");
                    posSpace++;
                }

                lastCmd = tmpStr;
            }
        }

        gCodeLines << tmpStr;
    }

    emit logMessage(QString().sprintf("Read gcode, loaded. Time elapsed: %d ms", t.elapsed()));
    //     qDebug() << "read gcode end";
    t.restart();


    PlaneEnum currentPlane;
    currentPlane = XY;

    gCodeList.clear();
    g0Points.clear();


    Settings::coord[X].softLimitMax = 0;
    Settings::coord[X].softLimitMin = 0;
    Settings::coord[Y].softLimitMax = 0;
    Settings::coord[Y].softLimitMin = 0;
    Settings::coord[Z].softLimitMax = 0;
    Settings::coord[Z].softLimitMin = 0;

    GCodeData *tmpCommand = new GCodeData();

    // TODO home pos
    g0Points << GCodeOptim {QVector3D(0, 0, 0), goodList.count(), -1, gCodeList.count(), -1};
    //     int preCount = 0;

    foreach(QString line, gCodeLines) {
        decoded = true;
        QString correctLine = line;
        QStringList vct_ref = line.simplified().split(" ", QString::SkipEmptyParts);
        const QString cmd = vct_ref.at(0);

        if (cmd.isEmpty()) {
            continue;
        }

        // qDebug() << cmd << line;
        switch(cmd.at(0).toLatin1()) {
            case 'G': {
                if (cmd == "G00") { // eilgang
                    QVector3D delta_pos;
                    QVector3D next_pos(b_absolute ? current_pos - origin : QVector3D(0, 0, 0));
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->xyz = next_pos;

                    if (gCodeList.count() > 0) {
                        delta_pos = next_pos - gCodeList.last().xyz;

                        if (Settings::filterRepeat == true) {
                            if (delta_pos == QVector3D(0, 0, 0) && gCodeList.last().movingCode == RAPID_LINE_CODE) {
                                correctLine = "";
                                break;
                            }
                        }
                    } else {
                        delta_pos = QVector3D(0, 0, 0);
                    }


                    detectMinMax(tmpCommand);

                    tmpCommand->splits = 0;

                    tmpCommand->typeMoving = GCodeData::Line;

                    tmpCommand->movingCode = RAPID_LINE_CODE;
                    tmpCommand->plane = currentPlane;


                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    // for the way optimizing
                    switch (currentPlane) {
                        case XY: {
                            // xy moving
                            if (delta_pos.z() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.y() != 0.0)) {
                                if (gCodeList.last().movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, gCodeList.count(), -1};
                                }

                                break;
                            }

                            // z moving
                            if (delta_pos.z() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.y() == 0.0)) {
                                if (gCodeList.last().movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (gCodeList.count());
                                }
                            }

                            break;
                        }

                        case YZ: {
                            if (delta_pos.x() == 0.0 && (delta_pos.y() != 0.0 || delta_pos.z() != 0.0)) {
                                // yz moving
                                if (gCodeList.last().movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (gCodeList.count() - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, gCodeList.count(), -1};
                                }

                                break;
                            }

                            // x moving
                            if (delta_pos.x() != 0.0 && (delta_pos.y() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeList.last().movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (gCodeList.count());
                                }
                            }

                            break;
                        }

                        case ZX: {
                            if (delta_pos.y() == 0.0 && (delta_pos.x() != 0.0 || delta_pos.z() != 0.0)) {
                                // zx moving
                                if (gCodeList.last().movingCode == RAPID_LINE_CODE) {
                                    g0Points.last().lineEnd = (goodList.count() - 1 );
                                    g0Points.last().gcodeEnd = (gCodeList.count() - 1);
                                    g0Points << GCodeOptim {current_pos, goodList.count(), -1, gCodeList.count(), -1};
                                }

                                break;
                            }

                            // y moving
                            if (delta_pos.y() != 0.0 && (delta_pos.x() == 0.0 && delta_pos.z() == 0.0)) {
                                if (gCodeList.last().movingCode != RAPID_LINE_CODE) {
                                    g0Points.last().gcodeEnd = (gCodeList.count());
                                }
                            }

                            break;
                        }

                        default:
                            break;
                    }

                    gCodeList << *tmpCommand;
                    // init of next instuction
                    tmpCommand = new GCodeData(tmpCommand);

                    tmpCommand->numberLine = goodList.count();

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    break;
                }

                if (cmd == "G01") { // feed
                    QVector3D next_pos(b_absolute ? current_pos - origin : QVector3D(0, 0, 0));
                    float E(-1.0);

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->xyz = next_pos;

                    tmpCommand->splits = 0;

                    detectMinMax(tmpCommand);

                    tmpCommand->typeMoving = GCodeData::Line;

                    tmpCommand->movingCode = FEED_LINE_CODE;

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    tmpCommand->plane = currentPlane;

                    gCodeList << *tmpCommand;

                    calcAngleOfLines(gCodeList.count() - 1);

                    // init of next instuction
                    tmpCommand = new GCodeData(tmpCommand);

                    tmpCommand->numberLine = goodList.count();

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    break;
                }

                // http://www.manufacturinget.org/2011/12/cnc-g-code-g02-and-g03/
                if (cmd == "G02" || cmd == "G03") { // arc
                    QVector3D next_pos(b_absolute ? current_pos - origin : QVector3D(0, 0, 0));
                    float E(-1.0);

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->xyz = next_pos;

                    detectMinMax(tmpCommand);

                    QVector3D arc_center(current_pos);
                    // float E_arc(-1.0);
                    float radius = 0.0;

                    if (parseArc(line, arc_center, radius, coef ) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->plane = currentPlane;

                    if (radius == 0.0) {
                        // the arc center coordinateds
                        tmpCommand->ijk = arc_center;
                        //                         tmpCommand->J = arc_center.y();
                        //                         tmpCommand->K = arc_center.z();
                    }

                    tmpCommand->Radius = radius;

                    if (cmd == "G02") {
                        tmpCommand->typeMoving = GCodeData::ArcCW;
                    } else {
                        tmpCommand->typeMoving = GCodeData::ArcCCW;
                    }

                    tmpCommand->movingCode = FEED_LINE_CODE;

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    // qDebug() << "line " << tmpCommand->numberLine << "before convertArcToLines()" << gCodeList.count() << "splits" << tmpCommand->splits;
                    convertArcToLines(tmpCommand); // tmpCommand has data of last point

                    tmpCommand->numberLine = goodList.count();

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    // qDebug() << "after " << gCodeList.count() << "splits" << tmpCommand->splits;
                    break;
                }

                if (cmd == "G04") {
                    // need next parameter
                    QString property1 = vct_ref.at(1).mid(0, 1);
                    QString value1 = vct_ref.at(1).mid(1);

                    if (property1 == "P") {
                        bool res;
                        tmpCommand->pauseMSeconds = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                            break;
                        }
                    }

                    if (property1 == "X") {
                        bool res;
                        tmpCommand->pauseMSeconds = value1.toFloat(&res) * 1000;

                        if (res == false) {
                            decoded = false;
                            break;
                        }
                    }

                    break;
                }

                if (cmd == "G17") {
                    currentPlane = XY;
                    break;
                }

                if (cmd == "G18") {
                    currentPlane = YZ;
                    break;
                }

                if (cmd == "G19") {
                    currentPlane = ZX;
                    break;
                }

                if (cmd == "G20") {
                    coef = 25.4;
                    break;
                }

                if (cmd == "G21") {
                    coef = 1.0;
                    break;
                }

                if (cmd == "G28") {
                    QVector3D next_pos(qInf(),
                                       qInf(),
                                       qInf());
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    if (qIsInf(next_pos[0])
                            && qIsInf(next_pos[1])
                            && qIsInf(next_pos[2])) {
                        current_pos = origin = QVector3D(0, 0, 0);
                    } else {
                        for(size_t i = 0 ; i < 3 ; ++i) {
                            if (qIsInf(next_pos[i])) {
                                current_pos[i] = 0;
                                origin[i] = 0;
                            }
                        }
                    }

                    break;
                }

                if (cmd == "G90") {
                    b_absolute = true;
                    break;
                }

                if (cmd == "G91") {
                    b_absolute = false;
                    break;
                }

                if (cmd == "G92") {
                    QVector3D next_pos(current_pos);
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    origin = current_pos - next_pos;

                    break;
                }

                // Home axes to minimum
                if (cmd == "G161") {
                    break;
                }

                // Home axes to maximum
                if (cmd == "G162") {
                    break;
                }

                decoded = false;
                break;
            }

            case 'M': {
                if (cmd == "M00") {
                    tmpCommand->pauseMSeconds = 0; // waiting
                    break;
                }

                if (cmd == "M03") {
                    tmpCommand->spindelON = true;
                    break;
                }

                if (cmd == "M05") {
                    tmpCommand->spindelON = false;
                    break;
                }

                if (cmd == "M06") {
                    // need next parameter
                    QString property1 = vct_ref.at(1).mid(0, 1);
                    QString value1 = vct_ref.at(1).mid(1);

                    if (property1 == "T") {
                        tmpCommand->changeInstrument = true;
                        bool res;
                        tmpCommand->numberInstrument = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                        }

                        tmpCommand->pauseMSeconds = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                        }

                        if (vct_ref.count() > 2) {
                            QString property2 = vct_ref.at(2).mid(0, 1);

                            if ( property2 == "D" ) {
                                QString value2 = vct_ref.at(2).mid(1).replace(Settings::fromDecimalPoint, Settings::toDecimalPoint);

                                tmpCommand->diametr = value2.toDouble(&res);

                                if (res == false) {
                                    decoded = false;
                                    break;
                                }
                            }
                        }
                    }

                    break;
                }

                // Disable all stepper motors
                if (cmd == "M18") {
                    break;
                }

                // Turn extruder 1 on (Forward), Undo Retraction
                if (cmd == "M101") {
                    break;
                }

                // Turn extruder 1 on (Reverse)
                if (cmd == "M102") {
                    break;
                }

                // Turn all extruders off, Extruder Retraction
                if (cmd == "M103") {
                    break;
                }

                // Set Extruder Temperature
                if (cmd == "M104") {
                    break;
                }

                // Get Extruder Temperature
                if (cmd == "M105") {
                    break;
                }

                // Set Extruder Speed (BFB)
                if (cmd == "M108") {
                    break;
                }

                // Set Extruder Temperature and Wait
                if (cmd == "M109") {
                    break;
                }

                // Set Extruder PWM
                if (cmd == "M113") {
                    break;
                }

                //
                if (cmd == "M132") {
                    break;
                }

                //
                if (cmd == "M206") {
                    float E;

                    if (parseCoord(line, origin, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    break;
                }

                decoded = false;
                break;
            }

            case 'F': {
                decoded = false;
                break;
            }

            default:
                decoded = false;
                break;
        }

        if (decoded == false) {
            emit logMessage(QString("gcode parsing error: " + line));
        } else {
            if (correctLine.length() > 0) {
                goodList << correctLine;
            }
        }
    }

    if (g0Points.count() > 2 && goodList.count() > 1) {
        g0Points.last().lineEnd = (goodList.count() - 1 );
        g0Points.last().gcodeEnd = (gCodeList.count() - 1);
    }

    //     QString log = "Read gcode, parsed. Time elapsed: " + QString::number(t.elapsed()) + " ms";

    //     qDebug("read gcode, parsed. Time elapsed: %d ms", t.elapsed());

    gCodeLines.clear();

    mut.unlock();


    emit logMessage(QString().sprintf("Read gcode, parsed. Time elapsed: %d ms, lines parsed: %d", t.elapsed(), goodList.count()));


    //     qDebug() << "readGCode" << goodList.count();

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
    if (d.xyz.x() > Settings::coord[X].softLimitMax) {
        Settings::coord[X].softLimitMax = d.xyz.x();
    }

    if (d.xyz.x() < Settings::coord[X].softLimitMin) {
        Settings::coord[X].softLimitMin = d.xyz.x();
    }

    if (d.xyz.y() > Settings::coord[Y].softLimitMax) {
        Settings::coord[Y].softLimitMax = d.xyz.y();
    }

    if (d.xyz.y() < Settings::coord[Y].softLimitMin) {
        Settings::coord[Y].softLimitMin = d.xyz.y();
    }

    if (d.xyz.z() > Settings::coord[Z].softLimitMax) {
        Settings::coord[Z].softLimitMax = d.xyz.z();
    }

    if (d.xyz.z() < Settings::coord[Z].softLimitMin) {
        Settings::coord[Z].softLimitMin = d.xyz.z();
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
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).xyz.y() - gCodeList.at(pos - 1).xyz.y(), gCodeList.at(pos).xyz.x() - gCodeList.at(pos - 1).xyz.x());
            break;
        }

        case YZ: {
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).xyz.z() - gCodeList.at(pos - 1).xyz.z(), gCodeList.at(pos).xyz.y() - gCodeList.at(pos - 1).xyz.y());
            break;
        }

        case ZX: {
            gCodeList[pos].angle = qAtan2(gCodeList.at(pos).xyz.x() - gCodeList.at(pos - 1).xyz.x(), gCodeList.at(pos).xyz.z() - gCodeList.at(pos - 1).xyz.z());
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
 * @param endData pointer to the list with decoded coordinates of endpoint
 *
 */
void GCodeParser::convertArcToLines(GCodeData *endData)
{
    if (gCodeList.count() == 0) {
        return;
    }

    if (endData == 0) {
        return;
    }

    if (!(endData->typeMoving == GCodeData::ArcCW || endData->typeMoving == GCodeData::ArcCCW) ) { // it's not arc
        return;
    }

    GCodeData &begData = gCodeList.last();
    // arcs
    // translate points to arc
    float r = 0.0; // length of sides
    //     float x2, x1, y2, y1, z2, z1;
    QVector3D beginPos, endPos;

    beginPos = begData.xyz;
    endPos = endData->xyz;
    //     x1 = begData.X;;
    //     x2 = endData->X;
    //
    //     y1 = begData.Y;
    //     y2 = endData->Y;
    //
    //     z1 = begData.Z;
    //     z2 = endData->Z;

    float i, j, k;
    i = endData->ijk.x();
    j = endData->ijk.y();
    k = endData->ijk.z();

    //     QVector3D pos1 = begData; //(x1, y1, z1);
    //     QVector3D pos2(x2, y2, z2);

    float deltaPos = 0.0;
    float begPos = 0.0;

    switch (endData->plane) {
        case XY: {
            if (endData->Radius == 0.0) {
                r = qSqrt(qPow(beginPos.x() - i, 2) + qPow(beginPos.y() - j, 2));
            } else {
                r = endData->Radius;
                // compute i, j
                //                 float a = determineAngle (pos1, pos2, endData->plane) + PI;
                //                 qDebug() << "radius " << r << "alpha" << a << "xy point 1" << x1 << y1 << "xy point 2" << x2 << y2;
            }

            deltaPos = endPos.z() - beginPos.z();
            begPos = beginPos.z();
        }
        break;

        case YZ: {
            if (endData->Radius == 0.0) {
                r = qSqrt(qPow(beginPos.y() - j, 2) + qPow(beginPos.z() - k, 2));
            } else {
                r = endData->Radius;
                // compute j, k
            }

            deltaPos = endPos.x() - beginPos.x();
            begPos = beginPos.x();
        }
        break;

        case ZX: {
            if (endData->Radius == 0.0) {
                r = qSqrt(qPow(beginPos.z() - k, 2) + qPow(beginPos.x() - i, 2));
            } else {
                r = endData->Radius;
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

    alpha_beg = determineAngle (beginPos, posC, endData->plane);
    alpha_end = determineAngle (endPos, posC, endData->plane);

    if (endData->typeMoving == GCodeData::ArcCW) {
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

    int n = (int)(bLength * Settings::splitsPerMm) - 1; // num segments of arc per mm
    float splitLen = 1.0 / (float)Settings::splitsPerMm;

    if ( n == 0) {
        qDebug() << "wrong, n = 0" << alpha_beg << alpha_end;
        return;
    }

    float dAlpha = alpha / n;

    deltaPos = deltaPos / n;

    if (endData->typeMoving == GCodeData::ArcCW) {
        dAlpha = -dAlpha;
    }

#if DEBUG_ARC
    QString dbg;
#endif
    float angle = alpha_beg;
    float loopPos = begPos;

    // copy of parsed endpoint
    GCodeData *ncommand = new GCodeData(endData);

#if DEBUG_ARC
    qDebug() << "arc from " << begData.X << begData.Y << begData.Z  << "to" << endData->X << endData->Y << endData->Z << "splits: " << n;
#endif

    QVector<GCodeData> tmpList;

    ncommand->xyz = begData.xyz;
    ncommand->abc = begData.abc;
    //     ncommand->Z = begData.Z;
    //     ncommand->A = begData.A;

    detectMinMax(ncommand);

    ncommand->splits = n;
    ncommand->movingCode = ACCELERAT_CODE;

    // now split
    switch (endData->plane) {
        case XY: {
            for (int step = 0; step < n; ++step) {
                //coordinates of next arc point
                angle += dAlpha;
                loopPos += deltaPos;

                float c = qCos(angle);
                float s = qSin(angle);

                float x_new = i + r * c;
                float y_new = j + r * s;

                float angle = qAtan2(y_new - ncommand->xyz.y(), x_new - ncommand->xyz.x());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->xyz = {x_new, y_new, loopPos};

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d x=%f y=%f angle=%f qSin=%f qCos=%f\n", step, x_new, y_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - endData->xyz.x()) * (x_new - endData->xyz.x()) + (y_new - endData->xyz.y()) * (y_new - endData->xyz.y())) <= splitLen) {
                    float t_angle = qAtan2(y_new - endData->xyz.y(), x_new - endData->xyz.x());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->xyz = endData->xyz;
                    //                     ncommand->Y = endData->Y;
                    //                     ncommand->Z = endData->Z;

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

                float angle = qAtan2(z_new - ncommand->xyz.z(), y_new - ncommand->xyz.y());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->xyz = {loopPos, y_new, z_new};
                //                 ncommand->X = loopPos;

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d y=%f z=%f angle=%f qSin=%f qCos=%f\n", step, y_new, z_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((y_new - endData->xyz.y()) * (y_new - endData->xyz.y()) + (z_new - endData->xyz.z()) * (z_new - endData->xyz.z())) <= splitLen) {
                    float t_angle = qAtan2(z_new - endData->xyz.z(), y_new - endData->xyz.y());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->xyz = endData->xyz;
                    //                     ncommand->Y = endData->Y;
                    //                     ncommand->Z = endData->Z;

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

                float angle = qAtan2(x_new - ncommand->xyz.x(), z_new - ncommand->xyz.z());

                if (angle < 0.0) {
                    angle += 2.0 * PI;
                }

                ncommand->angle = angle;
                ncommand->xyz = {x_new, loopPos, z_new};
                //                 ncommand->X = x_new;
                //                 ncommand->Y = loopPos;

                detectMinMax(ncommand);
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d z=%f x=%f angle=%f qSin=%f qCos=%f\n", step, z_new, x_new, angle, s, c);
#endif

                /** detection of end because of rounding */
                if (qSqrt((x_new - endData->xyz.x()) * (x_new - endData->xyz.x()) + (z_new - endData->xyz.z()) * (z_new - endData->xyz.z())) <= splitLen) {
                    float t_angle = qAtan2(x_new - endData->xyz.x(), z_new - endData->xyz.z());

                    if (t_angle < 0.0) {
                        t_angle += 2.0 * PI;
                    }

                    ncommand->angle = t_angle;

                    ncommand->xyz = endData->xyz;
                    //                     ncommand->Y = endData->Y;
                    //                     ncommand->Z = endData->Z;

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
 * @brief parsing of I, J, K, R parameters of G-Code
 *
 * @param
 * @return if anything is detected, return true
 */
bool GCodeParser::parseArc(const QString &line, QVector3D &pos, float &R, const float coef)
{
    if (line.isEmpty() == true) {
        return false;
    }

    QVector<QStringRef> chunks_ref = line.splitRef(" ", QString::SkipEmptyParts);

    QVector3D arc(qInf(), qInf(), qInf()); // too big coordinates

    if (chunks_ref.count() <= 1) {
        return false;
    }

    bool res = false;

    R = 0.0;

    for (int i = 1; i < chunks_ref.count(); i++) {
        QStringRef s = chunks_ref.at(i);
        bool conv;

        switch(s.at(0).toLatin1()) {
            case 'I': {
                arc.setX(pos.x() + coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'J': {
                arc.setY( pos.y() + coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'K': {
                arc.setZ( pos.z() + coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'R': {
                R = coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            default:
                break;
        }
    }

    pos = arc;

    return res;
}


/**
 * @brief parqSing X, Y, Z, E, F of G-Code parameters
 * rotation parameters A, B, C are not implemented
 *
 * @param
 * @return if anything is detected, return true
 */
bool GCodeParser::parseCoord(const QString &line, QVector3D &pos, float &E, const float coef, float *F)
{
    if (line.isEmpty() == true) {
        return false;
    }

    QVector<QStringRef> chunks_ref = line.splitRef(" ", QString::SkipEmptyParts);

    if (chunks_ref.count() <= 1) {
        return false;
    }

    bool res = false;

    for (int i = 1; i < chunks_ref.count(); i++) {
        QStringRef s = chunks_ref.at(i);
        bool conv;

        switch(s.at(0).toLatin1()) {
            case 'X': {
                pos.setX( coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'Y': {
                pos.setY( coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'Z': {
                pos.setZ( coef * (s.right(s.size() - 1).toDouble(&conv)));

                if (conv == true) {
                    res = true;
                } else {
                    qDebug () << "z" << s.right(s.size() - 1);
                }

                break;
            }

            case 'A': // rotation X
            case 'B': // rotation Y
            case 'C': { // rotation Z are not supported
                break;
            }

            case 'E': {
                E = coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'F': {
                if (F) {
                    *F = s.right(s.size() - 1).toDouble(&conv);
                }

                if (conv == true) {
                    res = true;
                }

                break;
            }

            default:
                break;
        }
    }

    return res;
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

