/****************************************************************************
 * C++ Implementation:                                                      *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
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


#ifndef READER_H
#define READER_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMutex>

#include <deque>
#include <utility>
#include "vec.h"
// #include "MainWindow.h"
#include "Translator.h"

#define byte unsigned char

class cTranslator;

//
// Инструкция для станка
//
class GCodeCommand
{
    public:
        bool   changeInstrument; // если true - то необходима остановка, для смены инструмента
        int    numberInstrument;  // собственно номер инструмента

        bool   needPause;        // необходимость паузы
        int    mSeconds;       // длительность паузы, если 0 - то ожидание от пользователя о продолжении

        //
        // координата в мм
        //
        double X;
        //
        // координата в мм
        //
        double Y;
        //
        // координата в мм
        //
        double Z;

        //
        // angle in grad
        double A;

        int    speed;       // скорость
        bool   spindelON;  // вкл. шпинделя
        int    numberInstruct;     // номер инструкции
        bool   workspeed; // true=G1 false=G0
        double diametr; // диаметр инструмента

        //
        // Пустой конструктор
        //
        GCodeCommand();

        //         GCodeCommand(int _numberInstruct, bool _spindelON, double _X, double _Y, double _Z, double _A, int _speed,
        //                      bool _workspeed, bool _changeInstrument = false, int _numberInstrument = 0,
        //                      bool _needPause = false, int _timeSeconds = 0, double _diametr = 0.0);

        //Конструктор на основе существующей команды
        GCodeCommand(GCodeCommand *_cmd);
};


//
// Результат парсинга G-кода
//
struct GCode_resultParse {
    //     QString FullStr; //
    QString GoodStr; // для распознанных
    QString BadStr;  //  для нераспознанных
};




enum typeFileLoad {
    None,
    GCODE,
    PLT,
    DRL,
    GBR
};


enum Apertures {
    C_circle,
    R_rectangle,
    O_obround,
    P_polygon
};

//возможные типы данных
enum typeCollections {
    Points,
    Instruments,
    Property,
};


struct typeSpline {
    int number;
    Apertures aperture;
    double size1;
    double size2;

    //     public typeSpline(int _number, Apertures _aperture, double _size1 = 0, double _size2 = 0)
    //     {
    //         number = _number;
    //         aperture = _aperture;
    //         size1 = _size1;
    //         size2 = _size2;
    //     }
};

//для работы с гербером
struct grbPoint {
    int X;
    int Y;
    QString typePoint; // D1 - видимое движение D2 - невидимое движение D3 - точка
    int numberSplane;

    //     public grbPoint(int _x, int _y, QString _typePoint, int _numberSplane)
    //     {
    //         X = _x;
    //         Y = _y;
    //         typePoint = _typePoint;
    //         numberSplane = _numberSplane;
    //     }
};


//класс описания точки
struct Point {
    float X;
    float Y;
    bool visible; //включение отображения данных
    int size; //размер линии

    //     public Point(float _x, float _y, bool _visible = true, int _size = 1)
    //     {
    //         X = _x;
    //         Y = _y;
    //         visible = _visible;
    //         size = _size;
    //     }
};

//класс описания инструмента
struct Instrument {
    int Number;
    float Diametr;

    //     public Instrument(int _number, float _diametr)
    //     {
    //         Numer = _number;
    //         Diametr = _diametr;
    //     }
};


//Набор однотипных данных
class DataCollections
{
        ///
        /// Конструктор набора точек
        ///
        /// парам: _Points">Список точек
    public:
        DataCollections(const QList<Point> &_Points, Instrument _intrument = (Instrument)
        {
            0, 0.0
        })
        {
            TypeData = Points;
            points = _Points;
            intrument = _intrument;
        };
    public:
        typeCollections TypeData;
        QList<Point> points;
        Instrument intrument;
};


///
/// Класс для хранения данных gerber файла
///
class GerberData
{
        ///
        /// Тип единицы измерения, мм или дюймы
        ///
    public:
        QString UnitsType;

        ///
        /// Типы сплайнов
        ///
        QList<typeSpline> typeSplines;// = new QList<typeSpline>();

        ///
        /// Точки из файла
        ///
        QList<grbPoint> points;// = new QList<grbPoint>();

        //длина всего числа
        int countDigitsX;
        //длина всего числа
        int countDigitsY;
        //длина дробной части
        int countPdigX;
        //длина дробной части
        int countPdigY;


        int X_min;
        int X_max;

        int Y_min;
        int Y_max;

        ///
        /// Вычисление размерности необходимого массива, для анализа
        ///
        /// парам: accuracy">Коэфициент уменьшения размеров данных
    public:
        GerberData();
        void CalculateGatePoints(int _accuracy);

};



// class for reading of different formats

class Reader : public cTranslator
{
        //         Q_OBJECT
    public:
        Reader();

        void BresenhamLine(QVector<QVector<byte> > &p, int x0, int y0, int x1, int y1, typeSpline _Splane);
        void BresenhamCircle(QVector<QVector<byte> > &p,  int x0, int y0, int radius, byte setvalue = 4, bool needFill = false);

        void readFile( const QString &fileName);

        //         void loadGCode(const QString &filename);


        //         void loadGCodeFromText(QStringList lines);
        //         bool parserGCodeLine(const QString &value);
        void OpenFile(const QString &name = "");
        void SaveFile();
        QStringList getGoodList();
        QStringList getBadList();

    public:
        QList<DataCollections> data;
        std::deque<std::pair<double, std::vector<Vec2d> > > layers;
        std::vector<Vec3f> cached_lines;
        std::vector<Vec3f> cached_points;
        std::vector<Vec3f> cached_color;

        QList<GCodeCommand> GCodeList;
        //             signals:
        //                 void logMessage(const QString &s);

    private:
        void Swap(int &p1, int &p2);
        bool parseCoord(const QString &line, Vec3 &pos, double &E, const double coef, double *F = NULL);
        bool readGCode( const QByteArray &gcode );
        void readGBR( const QByteArray &gcode );
        void readDRL( const QByteArray &gcode );
        void readPLT( const QByteArray &gcode );
        void lock() const;
        void unlock() const;
        //         GCode_resultParse parseStringGCode(const QString &value);

    private:
        QChar fromDecimalPoint;
        QChar toDecimalPoint;
        typeFileLoad TypeFile;// = typeFileLoad.None;
        QStringList goodList; //массив только для распознаных!!! G-кодов
        QStringList badList;
        //         MainWindow* parent;
        mutable QMutex mutex;
};


#endif // ABOUT_H
