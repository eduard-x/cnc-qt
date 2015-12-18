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


/******************************************************************************
** Reader
*/


GCodeCommand::GCodeCommand()
{
    changeInstrument = false;
    numberInstrument = 0;
    needPause        = false;
    mSeconds      = 0;

    X = 0.0;
    Y = 0.0;
    Z = 0.0;
    A = 0.0;

    angleVectors = 0;
    Distance = 0.0;

    spindelON      = false;
    numberInstruct = 0;
    speed          = 0;
    workspeed      = false;
    diametr = 0.0;
};


// GCodeCommand::GCodeCommand(int _numberInstruct, bool _spindelON, float _X, float _Y, float _Z, float _A, int _speed, bool _workspeed, bool _changeInstrument, int _numberInstrument, bool _needPause, int _timeSeconds, float _diametr)
// {
//     X = _X;
//     Y = _Y;
//     Z = _Z;
//     A = _A;
//     spindelON = _spindelON;
//     numberInstruct = _numberInstruct;
//     speed = _speed;
//     workspeed = _workspeed;
//
//     changeInstrument = _changeInstrument;
//     numberInstrument = _numberInstrument;
//     needPause        = _needPause;
//     mSeconds      = _timeSeconds;
//     diametr = _diametr;
// };


// constructor based on existing command
GCodeCommand::GCodeCommand(GCodeCommand *_cmd)
{
    X = _cmd->X;
    Y = _cmd->Y;
    Z = _cmd->Z;
    A = _cmd->A;

    spindelON = _cmd->spindelON;
    numberInstruct = _cmd->numberInstruct;
    speed = _cmd->speed;
    workspeed = _cmd->workspeed;

    angleVectors = _cmd->angleVectors;
    Distance = _cmd->Distance;

    changeInstrument = _cmd->changeInstrument;
    numberInstrument = _cmd->numberInstrument;
    needPause = _cmd->needPause;
    mSeconds = _cmd->mSeconds;
    diametr = _cmd->diametr;
};


//
// units of messure, mm or inches
//
GerberData::GerberData()
{
    UnitsType = "";

    //длина всего числа
    countDigitsX = 1;
    //длина всего числа
    countDigitsY = 1;
    //длина дробной части
    countPdigX = 0;
    //длина дробной части
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
//     : mutex(QMutex::Recursive)
{
    TypeFile = None;
}


// void Reader::lock() const
// {
//     mutex.lock();
// }
//
//
// void Reader::unlock() const
// {
//     mutex.unlock();
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

    if (sz > 20e6) {
        return false;
    }

    const QByteArray arr = file.readAll();
    QByteArray detectArray = arr.left(1024); // first 1024 bytes for format detection

    file.close();

    TypeFile = None;

    if ((detectArray.indexOf("G90") >= 0) || (detectArray.indexOf("G91") >= 0)) { // G-Code program detect
        TypeFile == GCODE;
        return readGCode(arr);
        //         return;
    }

    if ( detectArray.indexOf("IN1;") >= 0 ) { // plotter format
        TypeFile = PLT;
        return readPLT(arr);
        //         return;
    }

    if ( detectArray.indexOf("<svg") >= 0 ) { // svg
        TypeFile = SVG;
        return readSVG(arr);
        //         return;
    }

    if ( detectArray.indexOf("") >= 0 ) { // eps
        TypeFile = EPS;
        return readEPS(arr);
        //         return;
    }

    if ( detectArray.indexOf("") >= 0 ) { // polylines
        TypeFile = DXF;
        return readDXF(arr);
        //         return;
    }

    if ( detectArray.indexOf("") >= 0 ) { // excellon
        TypeFile = DRL;
        return readDRL(arr);
        //         return;
    }

    if ((detectArray.indexOf("G04 ") >= 0) && (detectArray.indexOf("%MOMM*%") > 0 || detectArray.indexOf("%MOIN*%") > 0) ) { // extended gerber
        TypeFile = GBR;
        return readGBR(arr);
        //         return;
    }

    if (TypeFile == None) { // error
        // qmessagebox
    }

    return false;
}


//
// Вызов диалога пользователя для выбора файла, и посылка данных в процедуру: LoadDataFromText(QStringList lines)
//
bool Reader::OpenFile(QString &fileName)
{
    QString name;

    if (fileName == "") {
        name = QFileDialog::getOpenFileName ( 0, translate(_LOAD_FROM_FILE), QDir::homePath() );//File.ReadAllLines(openFileDialog.FileName);

        if (name.length() == 0) {
            return false;
        }
    } else {
        name = fileName;
    }

    if (name.length() > 0) {
        return readFile(name);
    }

    return false;
#if 0
    QStringList sData;
    QFile file(name);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        sData << line;
    }

    file.close();

    loadGCodeFromText(sData);
#endif
}



//
// Преобразование строки, в список строк, с разделением параметров
//
// params: Строка с командами
// return: Список команд в строке по раздельности
#if 0
bool Reader::parserGCodeLine(const QString &inputL)
{
    Vec3 origin(0, 0, 0);
    Vec3 current_pos(0, 0, 0);
    bool b_absolute = true;
    float coef = 1.0;
    QString line = inputL;
    GCodeCommand *tmpCommand = new GCodeCommand();
    bool res = true;

    switch(line[0].toLatin1()) {
        case 'G':
            if (line.startsWith("G0 ") || line == "G0") {
                Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                float E;

                if (parseCoord(line, next_pos, E, coef) == false) {
                    res = false;
                    break;
                }

                if (b_absolute) {
                    current_pos = next_pos + origin;
                } else {
                    current_pos += next_pos;
                }
            } else if (line.startsWith("G1 ") || line == "G1") {
                Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                float E(-1.0);

                if (parseCoord(line, next_pos, E, coef) == false) {
                    res = false;
                    break;
                }

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
            } else if (line.startsWith("G28 ") || line == "G28") {
                Vec3 next_pos(std::numeric_limits<float>::infinity(),
                              std::numeric_limits<float>::infinity(),
                              std::numeric_limits<float>::infinity());
                float E;

                if (parseCoord(line, next_pos, E, coef) == false) {
                    res = false;
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
            } else if (line.startsWith("G20 ") || line == "G20") {
                coef = 25.4;
            } else if (line.startsWith("G21 ") || line == "G21") {
                coef = 1.0;
            } else if (line.startsWith("G90 ") || line == "G90") {
                b_absolute = true;
            } else if (line.startsWith("G91 ") || line == "G91") {
                b_absolute = false;
            } else if (line.startsWith("G92 ") || line == "G92") {
                Vec3 next_pos(current_pos);
                float E;

                if (parseCoord(line, next_pos, E, coef) == false) {
                    res = false;
                    break;
                }

                origin = current_pos - next_pos;
            }

            break;

        case 'M':
            if (line.startsWith("M206 ") || line == "M206") {
                float E;

                if (parseCoord(line, origin, E, coef) == false) {
                    res = false;
                    break;
                }
            }

            break;

        default:
            res = false;
            break;
    }


    delete tmpCommand;
    return res;
}
#endif

#if 0
QStringList Reader::parserGCodeLine(const QString &value)
{

    QStringList lcmd;// = new QStringList();
    QString internalVal = value.trimmed().toUpper();

    if (internalVal == "") {
        return lcmd;
    }

    // если строка начинается со скобки, то эту строку не анализируем, т.к. это комментарий!!!
    if (internalVal.mid(0, 1) == "(") {
        lcmd << internalVal;
        return lcmd;
    }

    if (internalVal.mid(0, 1) == "%") { //тоже пропускаем эту сторку
        lcmd << internalVal;
        return lcmd;
    }

    int inx = 0;

    bool collectCommand = false;

    foreach (QChar symb, internalVal) {
        if (symb >= 'A' && symb <= 'Z') { //символы от A до Z
            if (collectCommand) {
                inx++;
                collectCommand = false;
            }

            collectCommand = true;
            lcmd << "";
        }

        if (collectCommand && symb != ' ') {
            lcmd[inx] += symb.toAscii();
        }
    }

    return lcmd;
}
#endif
//
// Быстрый парсинг строки с G-кодом (только для визуальной части, проверка кие коды выполняем, а какие нет)
//
// params: строка с G-кодом
#if 0
GCode_resultParse Reader::parseStringGCode(const QString &value)
{
    GCode_resultParse result;

    // 1) распарсим строку
    QStringList lcmd = parserGCodeLine(value);

    QString sGoodsCmd = "";
    QString sBadCmd = "";

    // 2) проанализируем список команд, и разберем команды на те которые знаем и не знаем
    for (int i = 0; i < lcmd.count(); i++) {
        QString sCommd = lcmd.at(i).mid(0, 1);
        QString sValue = lcmd.at(i).mid(1);

        bool good = false;

        if (sCommd == "G") {
            //скорости движения
            if (sValue == "0" || sValue == "00") {
                good = true;
            }

            if (sValue == "1" || sValue == "01") {
                good = true;
            }

            // пауза в работе
            if (sValue == "4" || sValue == "04") {
                if ((i + 1) < lcmd.count()) {
                    //проверим что есть ещё параметр "P"
                    if (lcmd.at(i + 1).mid(0, 1).toUpper() == "P") {
                        sGoodsCmd += lcmd.at(i) + " " + lcmd.at(i + 1);
                        continue;
                    }
                }
            }
        }

        if (sCommd == "M") {
            //остановка до нажатия кнопки продолжить
            if (sValue == "0" || sValue == "00") {
                good = true;
            }

            //вкл/выкл шпинделя
            if (sValue == "3" || sValue == "03") {
                good = true;
            }

            if (sValue == "5" || sValue == "05") {
                good = true;
            }

            //смена инструмента
            if (sValue == "6" || sValue == "06") {
                if ((i + 2) < lcmd.count()) {
                    //проверим что есть ещё параметр "T" и "D"
                    if ((lcmd.at(i + 1).mid(0, 1) == "T") && (lcmd.at(i + 2).mid(0, 1) == "D")) {
                        sGoodsCmd += lcmd.at(i) + " " + lcmd.at(i + 1) + " " + lcmd.at(i + 2);
                        continue;
                    }
                }
            }
        }

        if (sCommd == "X" || sCommd == "Y" || sCommd == "Z") {
            //координаты 3-х осей
            good = true;
        }

        if (good) {
            sGoodsCmd += lcmd.at(i) + " ";
        } else {
            sBadCmd += lcmd.at(i) + " ";
        }
    }


    if (lcmd.count() == 0) {
        sBadCmd = value;
    }

    //     result.FullStr = value;
    result.GoodStr = sGoodsCmd;
    result.BadStr = sBadCmd;

    return result;
}
#endif

//
// Парсинг G-кода
//
// params: Массив строк с G-кодом
#if 0
void Reader::loadGCodeFromText(QStringList lines)
{
    //     statusProgress->setRange( 0, lines.count());
    //     statusProgress->setValue( 0 );

    int index = 0;
    goodList.clear();
    badList.clear();

    foreach (QString str, lines) {
        //         statusProgress->setValue( index++);

        GCode_resultParse graw = parseStringGCode(str.toUpper());

        if (graw.GoodStr.trimmed().length() == 0) {
            QString msg = translate(_NOT_DECODED);
            //             emit logMessage(msg.arg(QString::number(index)) + graw.BadStr);
            //             AddLog(msg.arg(QString::number(index)) + graw.BadStr);
            continue;
        }

        goodList << graw.GoodStr;
    }

    //     statusProgress->setValue(lines.count());
    //     QTimer::singleShot(5000, statusProgress, SLOT(reset()));

    //запуск анализа нормальных команд
    //     fillData(goodstr);
}
#endif

QStringList Reader::getGoodList()
{
    return goodList;
}

QStringList Reader::getBadList()
{
    return badList;
}


#if 0
void Reader::loadGCode(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    const QByteArray gcode = file.readAll();
    //     gcode_view->setPlainText(gcode);
    //     scroll_bar->setMaximum(gcode_view->getMaxScroll());
    //     scroll_bar->setValue(0);
    file.close();

    readGCode(gcode);
}
#endif


// read and parse into GCodeCommand list and OpenGL list
bool Reader::readGCode(const QByteArray &gcode)
{
    //     QMutexLocker mLock(&mutex);
    GCodeList.clear();

    //     lock();

    cached_lines.clear();
    cached_points.clear();
    cached_color.clear();

    goodList.clear();
    badList.clear();

    //     unlock();

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
    GCodeCommand *tmpCommand = new GCodeCommand();

    //     tmpCommand->angleVectors = 0;

    bool decoded;
    int index = 0;
    QStringList gCodeList;
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


        //         while (lineStream.length() > 0 && commentBeg >= 0 && commentEnd >= 0) {
        //             lineStream = lineStream.remove(commentBeg, commentEnd - commentBeg + 1);
        //             if (lineStream.length() > 0){
        //                 commentBeg = lineStream.indexOf('(');
        //                 commentEnd = lineStream.lastIndexOf(')');
        //             }
        //         }

        if (lineStream.length() == 0) {
            continue;
        }

        //         lineStream = lineStream.remove(' ');

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
        gCodeList << lineStream;
    }

    qDebug("read gcode, loaded. Time elapsed: %d ms", t.elapsed());
    //     qDebug() << "data loaded";

    t.restart();

    index = 0;
    foreach(QString line, gCodeList) {
        decoded = true;
        QStringList lst = line.simplified().split(" ");
        QString cmd = lst.at(0);

        if (cmd == "") {
            continue;
        }

        switch(cmd[0].toLatin1()) {
            case 'G':
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

                    tmpCommand->workspeed = false;

                    if (b_absolute) {
                        current_pos = next_pos + origin;
                    } else {
                        current_pos += next_pos;
                    }

                    break;
                }

                if (cmd == "G01") {
                    Vec3 next_pos(b_absolute ? current_pos - origin : Vec3(0, 0, 0));
                    float E(-1.0);

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    tmpCommand->X = next_pos.x();
                    tmpCommand->Y = next_pos.y();
                    tmpCommand->Z = next_pos.z();

                    tmpCommand->workspeed = true;

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

                    break;
                }

                if (cmd == "G04") {
                    //нужен следующий параметр
                    QString property1 = lst.at(1).mid(0, 1);
                    QString value1 = lst.at(1).mid(1);

                    if (property1 == "P") {
                        tmpCommand->needPause = true;
                        bool res;
                        tmpCommand->mSeconds = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                        }
                    }

                    if (property1 == "X") {
                        tmpCommand->needPause = true;
                        bool res;
                        tmpCommand->mSeconds = value1.toFloat(&res) * 1000;

                        if (res == false) {
                            decoded = false;
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
                    coef = 25.4;
                    break;
                }

                if (cmd == "G21") {
                    coef = 1.0;
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
                    Vec3 next_pos(current_pos);
                    float E;

                    if (parseCoord(line, next_pos, E, coef) == false) {
                        decoded = false;
                        break;
                    }

                    origin = current_pos - next_pos;
                }

                break;

            case 'M':
                if (cmd == "M00") {
                    tmpCommand->needPause = true;
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
                    //нужен следующий параметр
                    QString property1 = lst.at(1).mid(0, 1);
                    QString value1 = lst.at(1).mid(1);



                    if (property1 == "T") {
                        tmpCommand->changeInstrument = true;
                        bool res;
                        tmpCommand->numberInstrument = value1.toInt(&res);

                        if (res == false) {
                            decoded = false;
                        }

                        tmpCommand->mSeconds = value1.toInt(&res);

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
            GCodeList << *tmpCommand;

            tmpCommand->numberInstruct++;
            tmpCommand->needPause = false;
            tmpCommand->changeInstrument = false;
            tmpCommand->mSeconds = 0;

            goodList << line;
        }

        index++;
    }
#if 0

    if (!checkBoxNewSpped.Checked) {
        return;
    }

    // Вычисление угла между отрезками
    for (int numPos = 1; numPos < GKODS.Count; numPos++) {
        double xn = (double)(GKODS[numPos].X - GKODS[numPos - 1].X);
        double yn = (double)(GKODS[numPos].Y - GKODS[numPos - 1].Y);
        double zn = (double)(GKODS[numPos].Z - GKODS[numPos - 1].Z);

        //длина отрезка
        GKODS[numPos].Distance = (decimal) Math.Sqrt((xn * xn) + (yn * yn) + (zn * zn));

        if (numPos > GKODS.Count - 2) {
            continue;    //первую и последнюю точку не трогаем
        }

        //получим 3 точки
        double xa = (double)(GKODS[numPos - 1].X - GKODS[numPos].X);
        double ya = (double)(GKODS[numPos - 1].Y - GKODS[numPos].Y);
        double za = (double)(GKODS[numPos - 1].Z - GKODS[numPos].Z);
        double xb = (double)(GKODS[numPos + 1].X - GKODS[numPos].X);
        double yb = (double)(GKODS[numPos + 1].Y - GKODS[numPos].Y);
        double zb = (double)(GKODS[numPos + 1].Z - GKODS[numPos].Z);

        double angle = Math.Acos(   (xa * xb + ya * yb + za * zb) /  ( Math.Sqrt(xa * xa + ya * ya + za * za) * Math.Sqrt(xb * xb + yb * yb + zb * zb )  )       );
        double angle1 = angle * 180 / Math.PI   ;

        GKODS[numPos].angleVectors = (int)angle1;
    }

#endif

    qDebug("read gcode, parsed. Time elapsed: %d ms", t.elapsed());
    //     qDebug() << "data parsed";
    gCodeList.clear();

    delete tmpCommand;

    // qDebug() << "LIst" << goodList.count();
    for(size_t i = 0 ; i < cached_lines.size() ; ++i) {
        cached_color.push_back(Vec3f(1, 1, 1) * (float(i) / cached_lines.size()));
    }

    //     std::pair<Vec3, Vec3> bbox(Vec3(std::numeric_limits<float>::infinity(),
    //                                     std::numeric_limits<float>::infinity(),
    //                                     std::numeric_limits<float>::infinity()),
    //                                -Vec3(std::numeric_limits<float>::infinity(),
    //                                      std::numeric_limits<float>::infinity(),
    //                                      std::numeric_limits<float>::infinity()));
    //
    //     for(const auto &p : cached_points) {
    //         for(size_t i = 0 ; i < 3 ; ++i) {
    //             bbox.first[i] = std::min<float>(bbox.first[i], p[i]);
    //             bbox.second[i] = std::max<float>(bbox.second[i], p[i]);
    //         }
    //     }
    //     unlock();

    return true;
}


// if anything is detected, return true
bool Reader::parseCoord(const QString &line, Vec3 &pos, float &E, const float coef, float *F)
{
    if (line.isEmpty() == true) {
        return false;
    }

    //     qDebug() << line;
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

    //     parent->treeView1.Nodes.clear();

    //     TreeNode trc;// = new TreeNode("");


    //     qDebug() << "анализ файла";

    int index = 0;

    QTextStream stream(arr);
    stream.setLocale(QLocale("C"));

    while (!stream.atEnd()) {
        QString s = stream.readLine();
        qDebug() << "анализ файла строка " + QString::number(index);

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
                //                 indexList++;
                //             } else {
                //                 indexList++;
                //checkedListBox1.Items.Add("линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек");
                //                 trc.Text = "линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек";

                //                 data.last().Points <<  points;
                data << DataCollections(points);

                points.clear();
                //                 points = new QList<Point>();

                //                 treeView1.Nodes.Add(trc);
                //                 trc = new TreeNode("");
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

            // Пересчет в милиметры
            posX = posX / 40.0;
            posY = posY / 40.0;

            points <<  (Point) {
                posX, posY
            };
            //             trc.Nodes.Add("Точка - X: " + QString::number(posX) + "  Y: " + QString::number(posY));

        }

        //         s = fs.ReadLine();
        index++;
    }

    //     fl.close();

    //     indexList++;
    //     Instument instr = {0, 0.0}; // number, diameter
    data <<  DataCollections(points);
    //checkedListBox1.Items.Add("линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек");
    //     trc.Text = "линия - " + QString::number(indexList) + ", " + QString::number(points.count()) + " точек";
    //     data <<  points;
    //     points = new QList<Point>();

    //     points.clear();

    //     treeView1.Nodes.Add(trc);
    //     trc = new TreeNode("");


    //     qDebug() << "загружено!!!!!!!!";
    //     fs = null;

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

    //     StreamReader fs = new StreamReader(tbFile.Text);
    //     QString s = fs.ReadLine();

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

        //         s = fs.ReadLine();
    }

    //     fs = null;
    //     fl.close();

    //     treeView1.Nodes.clear();
    //
    //     foreach (DataCollections VARIABLE, data) {
    //         TreeNode trc = new TreeNode("Сверловка - " + QString::number(VARIABLE.intrument.Diametr));
    //
    //         foreach (Point VARIABLE2, VARIABLE.Points) {
    //             trc.Nodes.Add("Точка - X: " + QString::number(VARIABLE2.X) + "  Y: " + QString::number(VARIABLE2.Y));
    //         }
    //         treeView1.Nodes.Add(trc);
    //     }

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
    //     byte[,] spArray = new byte[1, 1];
    //     spArray[0, 0] = 1; //просто обычная точка
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
            //             parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
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
    //        debugstr += QString::number(spArray[dxx, dyy]);
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
        //         s = fs.ReadLine();
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
            //             parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
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

                int sized1 = (int)(splaynNow.size1 * 100);
                int sized2 = (int)(splaynNow.size2 * 100);

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
    //     arrayPoint = null;

    //System.Diagnostics.Process proc = System.Diagnostics.Process.Start("mspaint.exe", "d:\sample.bmp"); //Запускаем блокнот
    //proc.WaitForExit();//и ждем, когда он завершит свою работу
}


