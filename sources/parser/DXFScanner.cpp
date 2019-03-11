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
#include "DXFScanner.h"

#include "parse_dxf.h"


#define DEBUG_ARC 0


DXFData::DXFData()
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

#if 0
//
// Вычисление размерности необходимого массива, для анализа
//
// accuracy: Коэфициент уменьшения размеров данных
void DXFData::CalculateGatePoints(int _accuracy)
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

#endif

// using namespace Parser;


dxf::driver::driver(cDataManager *p)
{
    parent = p;
}



dxf::driver::~driver ()
{
}


void dxf::driver::parse (const QString &f)
{
    result = false;
    file = f.toUtf8().data();
    std::ifstream in(file.c_str());
    lexer = new dxf::scanner(&in);
    //         lexer->set_debug(trace_scanning);
    dxf::parser parser (*this);
    //  parser.set_debug_level (trace_parsing);
    result = parser.parse ();
    delete lexer;
    lexer = 0;
    in.close();
}


void dxf::driver::error (const dxf::parser::location_type& l, const std::string& m)
{
    parent->logBuffer << QString().sprintf("error in line %d pos %d : %s", l.begin.line, l.begin.column, m.c_str());
    //     std::cerr << l << ": " << m << std::endl;
}


void dxf::driver::error (const std::string& m)
{
    parent->logBuffer << QString().sprintf("%s", m.c_str());
    //     std::cerr << m << std::endl;
}



