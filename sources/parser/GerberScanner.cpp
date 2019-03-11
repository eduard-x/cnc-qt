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
#include "GerberScanner.h"

#include "parse_gerber.h"


#define DEBUG_ARC 0


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


gerber::driver::driver(cDataManager *p)
{
    parent = p;
}


gerber::driver::~driver ()
{
}


void gerber::driver::parse (const QString &f)
{
    result = false;
    file = f.toUtf8().data();
    std::ifstream in(file.c_str());
    lexer = new gerber::scanner(&in);
    //         lexer->set_debug(trace_scanning);
    gerber::parser parser (*this);
    //  parser.set_debug_level (trace_parsing);
    result = parser.parse ();
    delete lexer;
    lexer = 0;
    in.close();
}


void gerber::driver::error (const gerber::parser::location_type& l, const std::string& m)
{
    parent->logBuffer << QString().sprintf("error in line %d pos %d : %s", l.begin.line, l.begin.column, m.c_str());
    //     std::cerr << l << ": " << m << std::endl;
}


void gerber::driver::error (const std::string& m)
{
    parent->logBuffer << QString().sprintf("%s", m.c_str());
    //     std::cerr << m << std::endl;
}

#if 0
//
// gerber reader
//
bool Parser::GCode::read(char *indata)
{
    //     GerberData grb;
    int ret = true;

    dataVector.clear();

    mut.lock();

    gerberInit();

    /* because the data in already in buffer 'indata' */

    YY_BUFFER_STATE bs = gerber__scan_string(indata);
    gerber__switch_to_buffer(bs);

    if ( gerber_parse() != 0) {
        ret = false;
    }

    gerber_lex_destroy();

    mut.unlock();

    if (!ret) {
        gerberDestroy();
        return false;
    }

    gerberDestroy();

    return true;

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


/**
 * @brief
 *
 */
// QVector<ParserData> *Gerber::dataVector()
// {
//     //     qDebug() << "return gerber data" << gCodeList.count();
//     return &gCodeVector;
// }

