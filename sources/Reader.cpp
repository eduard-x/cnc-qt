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
// #include <QFileDialog>
#include <QString>
#include <QDir>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/Reader.h"
// #include "includes/MainWindow.h"


/******************************************************************************
** Reader
*/

//
// for debugging information of arc calculations
// to disable seto to 0
//


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

    //     this->parent = parent;
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


void Reader::writeFile(const QString &fileName)
{
}


bool Reader::readFile(const QString &fileName)
{
    int pointPos = fileName.lastIndexOf(".");

    if (pointPos == -1) { // error
        return false;
    }

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
        TypeFile = GCODE;
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
    //     qDebug() << "Готово!";

    return true;
    //  arrayPoint = null;

    //System.Diagnostics.Process proc = System.Diagnostics.Process.Start("mspaint.exe", "d:\sample.bmp"); //Запускаем блокнот
    //proc.WaitForExit();//и ждем, когда он завершит свою работу
}


