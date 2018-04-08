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
#include "includes/Gerber.h"
#include "includes/MainWindow.h"


#include "includes/GData.h"

#include "parse_gerber.h"
#include "scan_gerber.h"


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



// is static
QVector<GCodeData> Gerber::gCodeVector;

/**
 * @brief constructor
 *
 */
Gerber::Gerber()
{
}

/**
 * @brief destructor
 *
 */
Gerber::~Gerber()
{
    gCodeVector.clear();
}


void Gerber::gerberInit()
{
    gerber_lineno = 0;
    //      gerber_result = NULL;
    //   gerber_vector = NULL;
    //   gerber_header = NULL;
}



void Gerber::gerberDestroy()
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


/**
 * @brief read and parse into GCodeData list and OpenGL list
 * @see for the optimizations see https://blog.qt.io/blog/2014/06/13/qt-weekly-13-qstringliteral/
 * TODO convert QString to QStringLiteral
 *
 */
bool Gerber::readGCode(char *indata)
{
    int ret = true;

    gCodeVector.clear();

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
}



/**
 * @brief
 *
 */
QVector<GCodeData> *Gerber::dataVector()
{
    //     qDebug() << "return gerber data" << gCodeList.count();
    return &gCodeVector;
}

