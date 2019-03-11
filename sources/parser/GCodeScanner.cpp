/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2019 by Eduard Kalinowski                             *
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
#include <QtMath>

#include "Settings.h"
#include "GCodeScanner.h"

#include "GData.h"


gcode::driver::driver(cDataManager *p)
{
    parent = p;
}


gcode::driver::~driver ()
{
}


void gcode::driver::parse (const QString &f)
{
    result = false;
    file = f.toUtf8().data();
    std::ifstream in(file.c_str());
    lexer = new gcode::scanner(&in);
    //         lexer->set_debug(trace_scanning);
    gcode::parser parser (*this);
    //  parser.set_debug_level (trace_parsing);
    result = parser.parse ();
    delete lexer;
    lexer = 0;
    in.close();
}


bool gcode::driver::find_key(const QString &s)
{
    if (parent->dataVaris.find(s) == parent->dataVaris.end()) {
        return false;
    } else {
        return true;
    }
}


float gcode::driver::get_vari(const QString &s)
{
    return parent->dataVaris.find(s).value();
}


void gcode::driver::add_varimap(const QString &s, float f)
{
    parent->dataVaris.insert(s, f);
}


void gcode::driver::add_gcode(GData &g)
{
    parent->dataVector << g;
}


void gcode::driver::error (const gcode::parser::location_type& l, const QString& m)
{
    parent->logBuffer << QString(QString().sprintf("error in line %d pos %d : ", l.begin.line, l.begin.column) + m);
}


void gcode::driver::error (const QString& m)
{
    parent->logBuffer << m;
}


/**
 * @brief check the parsed data and create the vector of SerialData
 * with calculated data for sending to mk1
 *
 */
void gcode::driver::dataChecker()
{
    // for Ant optimization
    parent->gCities.clear();

    parent->filteredList.clear();
    parent->serialDataVector.clear();

    bool b_absolute = true;
    float coef = 1.0; // 1 or 24.5

    QVector3D origin(0, 0, 0);
    QVector3D current_pos(0, 0, 0);

    // TODO home pos
    // goodList has the text for table
    parent->gCities << GCodeOptim {QVector3D(0, 0, 0), 0/*originalList.count()*/, -1, parent->filteredList.count(), -1, parent->dataVector.count(), -1};

    parent->currentMCmd = new MData();

    parent->currentMCmd->plane = XY;

    // the first pos (cur = 0) is 0 or home
    for(int cur = 1; cur < parent->dataVector.count(); cur++) {
        GData d = parent->dataVector.at(cur);

        if (d.decoded == false) {
            qInfo() << "not decoded line" << d.numberLine;
            continue;
        }

        QString filteredLine;

        SerialData *sTmp = 0;

        if (d.gCmd >= 0) {
            sTmp = new SerialData(d.coord);

            parent->detectMinMax(sTmp->coord);

            filteredLine = QString().sprintf("G%02d ", d.gCmd);

            // filter duplicates
            if (Settings::filterRepeat == true) {
                if (parent->dataVector.at(cur - 1).coord.x() != sTmp->coord.x()) {
                    filteredLine += QString().sprintf("X%g ", sTmp->coord.x());
                }

                if (parent->dataVector.at(cur - 1).coord.y() != sTmp->coord.y()) {
                    filteredLine += QString().sprintf("Y%g ", sTmp->coord.y());
                }

                if (parent->dataVector.at(cur - 1).coord.z() != sTmp->coord.z()) {
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
            delta_pos = d.coord - parent->dataVector.at(cur - 1).coord;

            if (b_absolute) {
                current_pos = d.coord + origin;
            } else {
                current_pos += d.coord;
            }

            switch (d.gCmd) {
                case 0: {
                    sTmp->movingCode = RAPID_LINE_CODE;

                    // for the way optimizing
                    parent->checkCity(current_pos, cur);

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

                    parent->convertArcToLines(cur);

                    break;
                }

                case 4: { //
                    break;
                }

                case 17: {
                    MData *m = new MData();
                    m->plane = XY;
                    parent->currentMCmd->plane = XY;
                    sTmp->pMCommand = m;
                    break;
                }

                case 18: {
                    MData *m = new MData();
                    m->plane = YZ;
                    parent->currentMCmd->plane = YZ;
                    sTmp->pMCommand = m;
                    break;
                }

                case 19: {
                    MData *m = new MData();
                    m->plane = ZX;
                    parent->currentMCmd->plane = ZX;
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
                    origin = parent->dataVector.at(cur - 1).coord - d.coord;
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
                    parent->logBuffer << QString().sprintf("Not decoded line %d G command %d", d.numberLine, d.gCmd);
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
                    parent->logBuffer << QString().sprintf("Not decoded line %d, M command %d", d.numberLine, d.mCmd);
                    d.decoded = false;
                    break;
                }
            }
        }

        if (d.decoded == true) {
            if (sTmp) {
                parent->serialDataVector << sTmp;
            }

            if (d.lineComment.length()) {
                if (Settings::filterRepeat == false) {
                    filteredLine += d.lineComment;
                }
            }

            if (filteredLine.length()) {
                parent->filteredList << filteredLine;
                filteredLine.clear();
            }
        }
    }
}

#if 0
bool GCode::readPLT( char *indata )
{
#if 0
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
#endif
    return true;

}


bool GCode::readEPS( char *indata)
{
    return true;
}

bool GCode::readDRL(char *indata)
{
#if 0
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
#endif
    return true;

}
#endif


