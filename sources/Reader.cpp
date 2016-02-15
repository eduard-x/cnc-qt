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


#include <QObject>
#include <QRegExp>
#include <QDebug>
#include <QTime>
#include <QFileDialog>
#include <QString>
#include <QDir>

#include <cmath>
#include <limits>

#include "includes/Reader.h"
#include "includes/MainWindow.h"


/******************************************************************************
** Reader
*/

//
// for debugging information of arc calculations
// to disable seto to 0
//
#define DEBUG_ARC 0

#if 1
GCodeData::GCodeData()
{
    changeInstrument = false;
    numberInstrument = 0;
    //  needPause        = false;
    pauseMSeconds      = -1;

    X = 0.0;
    Y = 0.0;
    Z = 0.0;
    A = 0.0;

    // arc parameters
    I = 0.0;
    J = 0.0;
    K = 0.0;

    plane = NonePlane;
    changeDirection = false;

    Radius = 0.0;
    // end of arc

    typeMoving = NoType;

    accelCode =  NO_CODE;

    vectSpeed = 0.0;

    stepsCounter = 0;

    angle = 0.0;

    spindelON = false;
    splits = 0; // init
    numberLine = 0;
    feed      = false;
    diametr = 0.0;
};


// constructor based on existing command
GCodeData::GCodeData(GCodeData *_cmd)
{
    X = _cmd->X;
    Y = _cmd->Y;
    Z = _cmd->Z;
    A = _cmd->A;

    I = _cmd->I;
    J = _cmd->J;
    K = _cmd->K;

    Radius = _cmd->Radius;

    plane = _cmd->plane;

    changeDirection = false;

    typeMoving = _cmd->typeMoving;

    spindelON = _cmd->spindelON;
    vectSpeed = _cmd->vectSpeed;

    splits = 0; // if arc, will be splitted
    stepsCounter = 0; // should calculated

    accelCode = _cmd->accelCode;
    //  numberInstruct = _cmd->numberInstruct;
    numberLine = _cmd->numberLine;
    feed = _cmd->feed;

    angle = 0.0;//_cmd->angleVectors;

    changeInstrument = _cmd->changeInstrument;
    numberInstrument = _cmd->numberInstrument;
    pauseMSeconds = _cmd->pauseMSeconds;
    diametr = _cmd->diametr;
};
#endif

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



Reader::Reader()
//  : mutex(QMutex::Recursive)
{
    TypeFile = None;
}


// void Reader::lock() const
// {
//  mutex.lock();
// }
//
//
// void Reader::unlock() const
// {
//  mutex.unlock();
// }


void Reader::SaveFile()
{
}


bool Reader::readFile(const QString &fileName)
{
    int pointPos = fileName.lastIndexOf(".");

    if (pointPos == -1) { // error
        return false;
    }

    QString n = QString::number(1.01);
    toDecimalPoint = (n.indexOf(",") > 0) ? ',' : '.';
    fromDecimalPoint = (toDecimalPoint == ',') ? '.' : ',';

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
        return false;
    }

    qint64 sz = file.size();

    if (sz > MAX_FILE_SIZE) {
        return false;
    }

    const QByteArray arr = file.readAll();
    QByteArray detectArray = arr.left(1024); // first 1024 bytes for format detection

    file.close();

    TypeFile = None;

    if ((detectArray.indexOf("G0") >= 0) || (detectArray.indexOf("G1") >= 0)) { // G-Code program detect
        TypeFile == GCODE;
        return readGCode(arr);
    }

    if ( detectArray.indexOf("IN1;") >= 0 ) { // plotter format
        TypeFile = PLT;
        return readPLT(arr);
    }

    if ( detectArray.indexOf("<svg") >= 0 ) { // svg
        TypeFile = SVG;
        return readSVG(arr);
    }

    if ( detectArray.indexOf("") >= 0 ) { // eps
        TypeFile = EPS;
        return readEPS(arr);
    }

    if ( detectArray.indexOf("") >= 0 ) { // polylines
        TypeFile = DXF;
        return readDXF(arr);
    }

    if ( detectArray.indexOf("") >= 0 ) { // excellon
        TypeFile = DRL;
        return readDRL(arr);
    }

    if ((detectArray.indexOf("G04 ") >= 0) && (detectArray.indexOf("%MOMM*%") > 0 || detectArray.indexOf("%MOIN*%") > 0) ) { // extended gerber
        TypeFile = GBR;
        return readGBR(arr);
    }

    if (TypeFile == None) { // error
        // qmessagebox
    }

    return false;
}


//
// dialog for opening of file
//
bool Reader::OpenFile(QString &fileName)
{
    QString name;

    if (fileName == "") {
        name = QFileDialog::getOpenFileName ( 0, translate(_LOAD_FROM_FILE), QDir::homePath() );

        if (name.length() == 0) {
            return false;
        }
    } else {
        name = fileName;
    }

    if (name.length() > 0) {
        bool f = readFile(name);

        if (f == true) {
            QFileInfo fi(name);
            fileName = fi.absoluteFilePath();
        }

        return f;
    }

    return false;
}


QStringList Reader::getGoodList()
{
    return goodList;
}


QStringList Reader::getBadList()
{
    return badList;
}


bool Reader::addLine(GCodeData *c)
{
}


bool Reader::addArc(GCodeData *c)
{
}


// read and parse into GCodeData list and OpenGL list
bool Reader::readGCode(const QByteArray &gcode)
{
    //  QMutexLocker mLock(&mutex);
    gCodeList.clear();

    //  lock();

    cached_lines.clear();
    cached_points.clear();
    cached_color.clear();

    goodList.clear();
    badList.clear();

    //  unlock();

    QTextStream stream(gcode);
    stream.setLocale(QLocale("C"));
    // or this ? QString.split(QRegExp("\n|\r\n|\r"));
    // if we switch the input methode from QTextStream to QStringList, performance is about 15% higher

    Vec3 origin(0, 0, 0);
    Vec3 current_pos(0, 0, 0);
    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    QTime t;
    t.start();

    bool decoded;
    int index = 0;
    QStringList gCodeLines;
    QString lastCmd;

    while(!stream.atEnd()) { // restruct lines
        QString lineStream = stream.readLine().toUpper().trimmed();

        if (lineStream.isEmpty() || lineStream.at(0) == ';' || lineStream.at(0) == '(' || lineStream.at(0) == '%') {
            continue;
        }

        int posComment = lineStream.indexOf(";");

        if (posComment >= 0) {
            lineStream = lineStream.mid(posComment);

            if (lineStream.isEmpty()) {
                continue;
            }
        }

        int commentBeg = lineStream.indexOf('(');
        int commentEnd = -1;

        if (commentBeg >= 0) {
            commentEnd = lineStream.lastIndexOf(')');

            if (commentEnd > commentBeg) {
                lineStream = lineStream.remove(commentBeg, commentEnd - commentBeg + 1);
            }
        }

        //      while (lineStream.length() > 0 && commentBeg >= 0 && commentEnd >= 0) {
        //  lineStream = lineStream.remove(commentBeg, commentEnd - commentBeg + 1);
        //  if (lineStream.length() > 0){
        //   commentBeg = lineStream.indexOf('(');
        //   commentEnd = lineStream.lastIndexOf(')');
        //  }
        //      }

        if (lineStream.length() == 0) {
            continue;
        }

        //      lineStream = lineStream.remove(' ');

        lineStream = lineStream.replace(fromDecimalPoint, toDecimalPoint);
#if 0
        QString tmp = lineStream;

        foreach(QChar c, tmp) {
            if (c
        }
#else
        int pos = lineStream.indexOf(QRegExp("N(\\d+)"));

        if ( pos == 0) { // remove command number from lineStream
            int posNotDigit = lineStream.indexOf(QRegExp("([A-Z])"), pos + 1);

            if (posNotDigit > 0) {
                lineStream = lineStream.mid(posNotDigit);
            }
        }

        for (int iPos = 1; iPos >= 0; ) {
            iPos = lineStream.indexOf(QRegExp("([A-Z])"), iPos);

            if (iPos > 0) {
                lineStream.insert(iPos, " ");
                iPos += 2;
            }
        }

#endif

    if (lineStream.indexOf(QRegExp("[G|M|F](\\d+)($|\\s)")) == -1) { // Gxx, Fxx or Mxx not found
            if (lastCmd.length() > 0) {
                lineStream = QString(lastCmd + " " + lineStream);
            } else {
                QString msg = translate(_NOT_DECODED);
                badList << msg.arg(QString::number(index)) + lineStream;
            }
        } else {
            int posSpace = lineStream.indexOf(" ");

            if (posSpace > 0) { // command with parameter
                if (posSpace == 2) { // insert '0' if two characters
                    lineStream.insert(1, "0");
                    posSpace++;
                }

                lastCmd = lineStream.left(posSpace);

            } else { // command without parameter
                if (lineStream.length() == 2) { // insert '0' if two characters
                    lineStream.insert(1, "0");
                    posSpace++;
                }

                lastCmd = lineStream;
            }
        }

        index++;
        gCodeLines << lineStream;
    }

    qDebug("read gcode, loaded. Time elapsed: %d ms", t.elapsed());

    t.restart();

    index = 0;
    GCodeData *tmpCommand = new GCodeData();

    foreach(QString line, gCodeLines) {
        decoded = true;
        QStringList lst = line.simplified().split(" ");
        QString cmd = lst.at(0);

        if (cmd.isEmpty()) {
            continue;
        }

        bool movingCommand = true;

        switch(cmd[0].toLatin1()) {
            case 'G': {
                if (cmd == "G00") { // eilgang
                    Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->X = next_pos.x();
                    tmpCommand->Y = next_pos.y();
                    tmpCommand->Z = next_pos.z();
                    tmpCommand->splits = 0;

                    tmpCommand->typeMoving = GCodeData::Line;

                    tmpCommand->feed = false;

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    gCodeList << *tmpCommand;
                    // init of next instuction
                    tmpCommand = new GCodeData(tmpCommand);

                    tmpCommand->numberLine = index;

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    break;
                }

                if (cmd == "G01") { // feed
                    Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                    float E(-1.0);

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->X = next_pos.x();
                    tmpCommand->Y = next_pos.y();
                    tmpCommand->Z = next_pos.z();
                    tmpCommand->splits = 0;

                    tmpCommand->typeMoving = GCodeData::Line;

                    tmpCommand->feed = true;

                    if (E > 0.0) {
                        cached_lines.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                        cached_points.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                    }

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    if (E > 0.0) {
                        cached_lines.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                        cached_points.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                    }

                    gCodeList << *tmpCommand;
                    // init of next instuction
                    tmpCommand = new GCodeData(tmpCommand);

                    tmpCommand->numberLine = index;

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    break;
                }

                // http://www.manufacturinget.org/2011/12/cnc-g-code-g02-and-g03/
                if (cmd == "G02" || cmd == "G03") { // arc
                    Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                    float E(-1.0);

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->plane = NonePlane;

                    tmpCommand->X = next_pos.x();
                    tmpCommand->Y = next_pos.y();
                    tmpCommand->Z = next_pos.z();

                    Vec3 arc_center(current_pos);
                    // float E_arc(-1.0);
                    float radius = 0.0;

                    if (parseArc(line, arc_center, radius, coef ) == false) {
                        decoded = false;
                        break;
                    }

                    if (radius == 0.0) {
                        // the arc center coordinateds
                        tmpCommand->I = arc_center.x();
                        tmpCommand->J = arc_center.y();
                        tmpCommand->K = arc_center.z();

                        if (tmpCommand->K == COORD_TOO_BIG) {
                            tmpCommand->plane = XY;
                        } else if (tmpCommand->I == COORD_TOO_BIG) {
                            tmpCommand->plane = YZ;
                        } else if (tmpCommand->J == COORD_TOO_BIG) {
                            tmpCommand->plane = ZX;
                        }
                    } else { // radius detected, ijk should be calculated
                        // circle ?
                        if (current_pos.x() == next_pos.x() && current_pos.y() == next_pos.y()) {
                            tmpCommand->plane = XY;
                        } else if (current_pos.y() == next_pos.y() && current_pos.z() == next_pos.z()) {
                            tmpCommand->plane = YZ;
                        } else if (current_pos.z() == next_pos.z() && current_pos.x() == next_pos.x()) {
                            tmpCommand->plane = ZX;
                        } else if((current_pos.x() != next_pos.x() || current_pos.y() != next_pos.y()) && current_pos.z() == next_pos.z()) {
                            tmpCommand->plane = XY;
                        } else if((current_pos.y() != next_pos.y() || current_pos.z() != next_pos.z()) && current_pos.x() == next_pos.x()) {
                            tmpCommand->plane = YZ;
                        } else if((current_pos.z() != next_pos.z() || current_pos.x() != next_pos.x()) && current_pos.y() == next_pos.y()) {
                            tmpCommand->plane = ZX;
                        }
                    }

                    tmpCommand->Radius = radius;

                    if (cmd == "G02" ) {
                        tmpCommand->typeMoving = GCodeData::ArcCW;
                    } else {
                        tmpCommand->typeMoving = GCodeData::ArcCCW;
                    }

                    tmpCommand->feed = true;

                    if (E > 0.0) {
                        //  cached_arcs.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                        cached_points.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                    }

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    if (E > 0.0) {
                        //  cached_arcs.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                        cached_points.push_back(Vec3f(current_pos.x(), current_pos.y(), current_pos.z()));
                    }

                    // qDebug() << "line " << tmpCommand->numberLine << "before convertArcToLines()" << gCodeList.count() << "splits" << tmpCommand->splits;
                    convertArcToLines(tmpCommand); // tmpCommand has data of last point

                    gCodeList << *tmpCommand;
                    // init of next instuction
                    tmpCommand = new GCodeData(tmpCommand);

                    tmpCommand->numberLine = index;

                    tmpCommand->changeInstrument = false;
                    tmpCommand->pauseMSeconds = -1; // no pause

                    // qDebug() << "after " << gCodeList.count() << "splits" << tmpCommand->splits;
                    break;
                }

                if (cmd == "G04") {
                    // need next parameter
                    QString property1 = lst.at(1).mid(0, 1);
                    QString value1 = lst.at(1).mid(1);

                    if (property1 == "P") {
                        //  tmpCommand->needPause = true;
                        bool res;
                        tmpCommand->pauseMSeconds = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                            break;
                        }
                    }

                    if (property1 == "X") {
                        //  tmpCommand->needPause = true;
                        bool res;
                        tmpCommand->pauseMSeconds = value1.toFloat(&res) * 1000;

                        if (res == false) {
                            decoded = false;
                            break;
                        }
                    }

                    break;
                }

                if (cmd == "G28") {
                    Vec3 next_pos(std::numeric_limits<float>::infinity(),
                                  std::numeric_limits<float>::infinity(),
                                  std::numeric_limits<float>::infinity());
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    if (next_pos[0] == std::numeric_limits<float>::infinity()
                            && next_pos[1] == std::numeric_limits<float>::infinity()
                            && next_pos[2] == std::numeric_limits<float>::infinity()) {
                        current_pos = origin = Vec3(0, 0, 0);
                    } else {
                        for(size_t i = 0 ; i < 3 ; ++i) {
                            if (next_pos[i] != std::numeric_limits<float>::infinity()) {
                                current_pos[i] = 0;
                                origin[i] = 0;
                            }
                        }
                    }

                    break;
                }

                if (cmd == "G20") {
                    movingCommand = false;
                    coef = 25.4;
                    break;
                }

                if (cmd == "G21") {
                    movingCommand = false;
                    coef = 1.0;
                    break;
                }

                if (cmd == "G90") {
                    movingCommand = false;
                    b_absolute = true;
                    break;
                }

                if (cmd == "G91") {
                    movingCommand = false;
                    b_absolute = false;
                    break;
                }

                if (cmd == "G92") {
                    Vec3 next_pos(current_pos);
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    origin = current_pos - next_pos;
                }

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
                    QString property1 = lst.at(1).mid(0, 1);
                    QString value1 = lst.at(1).mid(1);

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

                        if (lst.count() > 2) {
                            QString property2 = lst.at(2).mid(0, 1);

                            if ( property2 == "D" ) {
                                QString value2 = lst.at(2).mid(1).replace(fromDecimalPoint, toDecimalPoint);

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

                if (cmd == "M206") {
                    float E;

                    if (parseCoord(line, origin, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    break;
                }

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
            QString msg = translate(_NOT_DECODED);
            badList << msg.arg(QString::number(index)) + line;
        } else {
#if 0

            if (movingCommand == true) {
                //   if (cmd != "G02" && cmd != "G03"){
                gCodeList << *tmpCommand;
                // init of next instuction

                tmpCommand = new GCodeData(tmpCommand);
                //   }


                //   tmpCommand->numberInstruct++;
                tmpCommand->numberLine = index;

                //   tmpCommand->needPause = false;
                tmpCommand->changeInstrument = false;
                tmpCommand->pauseMSeconds = -1; // no pause

            }

#endif
            goodList << line;
        }

        index++;
    }

    qDebug("read gcode, parsed. Time elapsed: %d ms", t.elapsed());
    //  qDebug() << "data parsed";
    gCodeLines.clear();

    //  delete tmpCommand;

    // qDebug() << "LIst" << goodList.count();
    for(size_t i = 0 ; i < cached_lines.size() ; ++i) {
        cached_color.push_back(Vec3f(1, 1, 1) * (float(i) / cached_lines.size()));
    }

    //  std::pair<Vec3, Vec3> bbox(Vec3(std::numeric_limits<float>::infinity(),
    //   std::numeric_limits<float>::infinity(),
    //   std::numeric_limits<float>::infinity()),
    // -Vec3(std::numeric_limits<float>::infinity(),
    //    std::numeric_limits<float>::infinity(),
    //    std::numeric_limits<float>::infinity()));
    //
    //  for(const auto &p : cached_points) {
    //      for(size_t i = 0 ; i < 3 ; ++i) {
    //  bbox.first[i] = std::min<float>(bbox.first[i], p[i]);
    //  bbox.second[i] = std::max<float>(bbox.second[i], p[i]);
    //      }
    //  }
    //  unlock();

    return true;
}


float Reader::determineAngle(const Vec3 &pos, const Vec3 &pos_center, PlaneEnum pl)
{
    float radians = 0.0;

    switch (pl) {
        case XY: {
            if (pos[0] == pos_center[0] && pos[1] == pos_center[1]) { // if diff is 0
                return 0.0;
            }

            radians = atan2(pos[1] - pos_center[1], pos[0] - pos_center[0]);

            break;
        }

        case YZ: {
            if (pos[1] == pos_center[1] && pos[2] == pos_center[2]) {
                return 0.0;
            }

            radians = atan2(pos[2] - pos_center[2], pos[1] - pos_center[1]);

            break;
        }

        case ZX: {
            if (pos[2] == pos_center[2] && pos[0] == pos_center[0]) {
                return 0.0;
            }

            radians = atan2(pos[0] - pos_center[0], pos[2] - pos_center[2]);

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

//
// 'endData' is the pointer of arc start
void Reader::convertArcToLines(GCodeData *endData)
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
    float a, r; // length of sides
    float x2, x1, y2, y1, z2, z1;

    x1 = begData.X;;
    x2 = endData->X;

    y1 = begData.Y;
    y2 = endData->Y;

    z1 = begData.Z;
    z2 = endData->Z;

    float i, j, k;
    i = endData->I;
    j = endData->J;
    k = endData->K;

    Vec3 pos1(x1, y1, z1);
    Vec3 pos2(x2, y2, z2);

    float dPos = 0.0;
    float begPos = 0.0;

    switch (endData->plane) {
        case XY: {
            a = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

            if (endData->Radius == 0.0) {
                r = sqrt(pow(x1 - i, 2) + pow(y1 - j, 2));
            } else {
                r = endData->Radius;
                // compute i, j
                float a = determineAngle (pos1, pos2, endData->plane) + PI;
                qDebug() << "radius " << r << "alpha" << a << "xy point 1" << x1 << y1 << "xy point 2" << x2 << y2;
            }

            dPos = z2 - z1;
            begPos = z1;
        }
        break;

        case YZ: {
            a = sqrt(pow(y2 - y1, 2) + pow(z2 - z1, 2));

            if (endData->Radius == 0.0) {
                r = sqrt(pow(y1 - j, 2) + pow(z1 - k, 2));
            } else {
                r = endData->Radius;
                // compute j, k
            }

            dPos = x2 - x1;
            begPos = x1;
        }
        break;

        case ZX: {
            a = sqrt(pow(z2 - z1, 2) + pow(x2 - x1, 2));

            if (endData->Radius == 0.0) {
                r = sqrt(pow(z1 - k, 2) + pow(x1 - i, 2));
            } else {
                r = endData->Radius;
                // compute k, i
            }

            dPos = y2 - y1;
            begPos = y1;
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

    Vec3 posC(i, j, k);

    alpha_beg = determineAngle (pos1, posC, endData->plane);
    alpha_end = determineAngle (pos2, posC, endData->plane);

    if (endData->typeMoving == GCodeData::ArcCW) {
        if (alpha_beg == alpha_end) {
            alpha_beg += 2.0 * PI;
        }

        alpha = alpha_beg - alpha_end;

        if (alpha_beg < alpha_end) {
            alpha = fabs(alpha_beg + (2.0 * PI - alpha_end));
        }

    } else {
        if (alpha_beg == alpha_end) {
            alpha_end += 2.0 * PI;
        }

        alpha = alpha_end - alpha_beg;

        if (alpha_beg > alpha_end) {
            alpha = fabs(alpha_end + (2.0 * PI - alpha_beg));
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

    dPos = dPos / n;

    if (endData->typeMoving == GCodeData::ArcCW) {
        dAlpha = -dAlpha;
    }

#if DEBUG_ARC
    QString dbg;
#endif
    float angle = alpha_beg;
    float loopPos = begPos;

    GCodeData *ncommand = new GCodeData(*endData);

#if DEBUG_ARC
    qDebug() << "arc from " << begData.X << begData.Y << begData.Z  << "to" << endData->X << endData->Y << endData->Z << "splits: " << n;
#endif

//     begData.splits = n;

    ncommand->X = begData.X;
    ncommand->Y = begData.Y;
    ncommand->Z = begData.Z;
    ncommand->A = begData.A;
    ncommand->splits = n;
    ncommand->accelCode = ACCELERAT_CODE;

    // stepsCounter

    // now split
    bool endLoop = false;

    for (int step = 0; step < n; ++step) {
        //coordinates of next arc point
        angle += dAlpha;
        loopPos += dPos;

        float c = cos(angle);
        float s = sin(angle);

        switch (endData->plane) {
            case XY: {
                float x_new = i + r * c;
                float y_new = j + r * s;
                ncommand->angle = atan2(y_new - ncommand->Y, x_new - ncommand->X);
                ncommand->X = x_new;
                ncommand->Y = y_new;
                ncommand->Z = loopPos;
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d x=%f y=%f angle=%f sin=%f cos=%f\n", step, x_new, y_new, angle, s, c);
#endif
            }
            break;

            case YZ: {
                float y_new = j + r * c;
                float z_new = k + r * s;
                ncommand->angle = atan2(z_new - ncommand->Z, y_new - ncommand->Y);
                ncommand->Y = y_new;
                ncommand->Z = z_new;
                ncommand->X = loopPos;
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d y=%f z=%f angle=%f sin=%f cos=%f\n", step, y_new, z_new, angle, s, c);
#endif
            }
            break;

            case ZX: {
                float z_new = k + r * c;
                float x_new = i + r * s;
                ncommand->angle = atan2(x_new - ncommand->X, z_new - ncommand->Z);
                ncommand->Z = z_new;
                ncommand->X = x_new;
                ncommand->Y = loopPos;
#if DEBUG_ARC
                dbg += QString().sprintf("n=%d z=%f x=%f angle=%f sin=%f cos=%f\n", step, z_new, x_new, angle, s, c);
#endif
            }
            break;

            default:
                break;
        }

        if (endLoop == true) {
            break;
        }

        gCodeList << *ncommand;
        ncommand = new GCodeData(*ncommand);
        ncommand->accelCode = CONSTSPEED_CODE;
        ncommand->splits =0;
    }

    // last
    endData->accelCode = DECELERAT_CODE; //
    //  endData->splits = 0;

    //  gCodeList << *endData;

#if DEBUG_ARC

    if ((fabs (x2 - res.last().X) > (bLength / splitsPerMm)) || (fabs (y2 - res.last().Y) > (bLength / splitsPerMm))) { // wenn zu weit vom ziel...
        if (endData->typeMoving == ArcCW) {
            qDebug() << "CW";
        } else {
            qDebug() << "CCW";
        }

        qDebug() << "anfang: " << x1 << y1 << "ende" << x2 << y2 << "center" << i << j;
        qDebug() << "bogen " << bLength << "mm" << "r" << r << "a" << a << "triangle alpha" << alpha;
        qDebug() << "alpha:" << alpha_beg << "->" << alpha_end << "d alpha: " << dAlpha; // rad
        qDebug() << dbg;
    }

#endif
}


// if anything is detected, return true
bool Reader::parseArc(const QString &line, Vec3 &pos, float &R, const float coef)
{
    if (line.isEmpty() == true) {
        return false;
    }

    //  qDebug() << line;
    const QStringList &chunks = line.toUpper().simplified().split(' ');

    Vec3 arc(COORD_TOO_BIG, COORD_TOO_BIG, COORD_TOO_BIG); // too big coordinates

    if (chunks.count() == 0) {
        return false;
    }

    bool res = false;

    R = 0.0;

    for(int i = 1 ; i < chunks.size() ; ++i) {
        const QString &s = chunks[i];
        bool conv;

        switch(s[0].toLatin1()) {
            case 'I': {
                arc.x() = pos.x() + coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'J': {
                arc.y() = pos.y() + coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'K': {
                arc.z() = pos.z() + coef * (s.right(s.size() - 1).toDouble(&conv));

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


// if anything is detected, return true
bool Reader::parseCoord(const QString &line, Vec3 &pos, float &E, const float coef, float *F)
{
    if (line.isEmpty() == true) {
        return false;
    }

    //  qDebug() << line;
    const QStringList &chunks = line.toUpper().simplified().split(' ');

    if (chunks.count() == 0) {
        return false;
    }

    bool res = false;

    for(int i = 1 ; i < chunks.size() ; ++i) {
        const QString &s = chunks[i];
        bool conv;

        switch(s[0].toLatin1()) {
            case 'X': {
                pos.x() = coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'Y': {
                pos.y() = coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
                }

                break;
            }

            case 'Z': {
                pos.z() = coef * (s.right(s.size() - 1).toDouble(&conv));

                if (conv == true) {
                    res = true;
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


bool Reader::readPLT( const QByteArray &arr )
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
        //начальная точка
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


bool Reader::readSVG( const QByteArray &arr)
{
    return true;
}


bool Reader::readEPS( const QByteArray &arr)
{
    return true;
}


bool Reader::readDXF( const QByteArray &arr)
{
    return true;
}


bool Reader::readDRL( const QByteArray &arr)
{
    data.clear();

    QList<Point> points;

    //  StreamReader fs = new StreamReader(tbFile.Text);
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
            float diametr = s.mid(pos1 + 1).replace(fromDecimalPoint, toDecimalPoint).toFloat();

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

void Reader::Swap(int &p1, int &p2)
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
void Reader::BresenhamCircle(QVector<QVector< byte > > &arrayPoint,  int x0, int y0, int radius, byte setvalue, bool needFill)
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


void Reader::BresenhamLine(QVector<QVector<byte> > &arrayPoint, int x0, int y0, int x1, int y1, typeSpline _Splane)
{
    //матрицу сплайна
    //  byte[,] spArray = new byte[1, 1];
    //  spArray[0, 0] = 1; //просто обычная точка
    QVector<QVector<byte> > spArray;

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
        spArray.push_back(QVector< byte > ());

        for (int x = 0; x <= sizeMatrixX; x++) {
            //  parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
            byte v = 0;//{0.0, 0.0, 0.0, 0.0 };
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

    bool steep = abs(y1 - y0) > abs(x1 - x0); // Проверяем рост отрезка по оси икс и по оси игрек

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
    int dy = abs(y1 - y0);
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
bool Reader::readGBR( const QByteArray &arr)
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

                float sizeRound = s.mid(sstart + 1, sEnd - sstart - 1).replace(fromDecimalPoint, toDecimalPoint).toDouble();

                grb.typeSplines <<  (typeSpline) {
                    numb, C_circle, sizeRound
                };
            }

            if (letterAperture == "R") { //если прямоугольник
                int sstart1 = s.indexOf(",");
                int sstart2 = s.indexOf("X");
                int sEnd = s.indexOf("*");

                float sizeX = s.mid(sstart1 + 1, sstart2 - sstart1 - 1).replace(fromDecimalPoint, toDecimalPoint).toDouble();
                float sizeY = s.mid(sstart2 + 1, sEnd - sstart2 - 1).replace(fromDecimalPoint, toDecimalPoint).toDouble();

                grb.typeSplines << (typeSpline) {
                    numb, R_rectangle, sizeX, sizeY
                };
            }

            if (letterAperture == "O") { //если овал
                int sstart1 = s.indexOf(",");
                int sstart2 = s.indexOf("X");
                int sEnd = s.indexOf("*");

                float sizeX = s.mid(sstart1 + 1, sstart2 - sstart1 - 1).replace(fromDecimalPoint, toDecimalPoint).toDouble();
                float sizeY = s.mid(sstart2 + 1, sEnd - sstart2 - 1).replace(fromDecimalPoint, toDecimalPoint).toDouble();

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

    qDebug() << "Наполнение массива";

    QVector<QVector<byte> > arrayPoint;// = new byte[grb.X_max + 1, grb.Y_max + 1];

    for (int y = 0; y <= grb.Y_max; y++) {
        //matrixYline matrixline = new matrixYline();

        //matrixline.Y = numPosY->value() + (y* numStep->value());
        arrayPoint.push_back(QVector< byte > ());

        for (int x = 0; x <= grb.X_max; x++) {
            //  parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
            byte v = 0;//{0.0, 0.0, 0.0, 0.0 };
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
                        0, C_circle, sized2 / 100
                    };
                    BresenhamLine(arrayPoint, oX1, oY1, oX2, oY2, tps);
                } else {
                    //овал вертикальный
                    int oX1 = newX;
                    int oY1 = newY - (sized2 / 2) + (sized1 / 2);
                    int oX2 = newX;
                    int oY2 = newY + (sized2 / 2) - (sized1 / 2);
                    typeSpline tps = (typeSpline) {
                        0, C_circle, sized1 / 100
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
    qDebug() << "Готово!";

    return true;
    //  arrayPoint = null;

    //System.Diagnostics.Process proc = System.Diagnostics.Process.Start("mspaint.exe", "d:\sample.bmp"); //Запускаем блокнот
    //proc.WaitForExit();//и ждем, когда он завершит свою работу
}


