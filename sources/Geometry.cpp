/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergej Zheigurov                              *
 *                                                                          *
 * Qt developing                                                            *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
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


#include "includes/Geometry.h"


/******************************************************************************
** Geometry
*/
// http://people.sc.fsu.edu/~jburkardt/m_src/m_src.html
// see the Hermite polynomial calculation

//класс для работы с геометрией
// public static class Geometry
// {
/*
 *    Корректировка высоты по оси Z, у точки №5, зная высоту по Z у точек 1,2,3,4
 *
 *  /\ ось Y
 *  |
 *  |    (точка №1) -------------*--------------- (точка №2)
 *  |                            |
 *  |                            |
 *  |                            |
 *  |                       (точка №5)
 *  |                            |
 *  |                            |
 *  |    (точка №3) -------------*--------------- (точка №4)
 *  |
 *  |
 *  *----------------------------------------------------------------> ось X
 *  Корректировка выполняется следующим образом:
 *  1) зная координату X у точки 5, и координаты точек 1 и 2, вычисляем высоту Z в точке которая находится на линии точек 1,2 и перпендикулярно 5-й точке (получает точку №12)
 *  2) Тоже самое вычисляется для точки на линии точек 3,4 (получает точку №34)
 *  3) Зная координаты точек №12, №34 и значение по оси Y у точки 5, вычисляем высоту по оси Z
 */


///
/// Функция корректирует высоту по оси Z
///
/// параметр "p1" первая точка первой линии X
/// параметр "p2" вторая точка первой линии X
/// параметр "p3" первая точка второй линии X
/// параметр "p4" вторая точка второй линии X
/// параметр "p5" точка у которой нужно скорректировать высоту
/// возвращаемый

dPoint Geometry::GetZ(dPoint p1, dPoint p2, dPoint p3, dPoint p4, dPoint p5)
{
    dPoint p12 = CalcPX(p1, p2, p5);
    dPoint p34 = CalcPX(p3, p4, p5);

    dPoint p1234 = CalcPY(p12, p34, p5);

    return p1234;
}


//нахождение высоты Z точки p0, лежащей на прямой которая паралельна оси X
dPoint Geometry::CalcPX(dPoint p1, dPoint p2, dPoint p0)
{
    dPoint retPoint = (dPoint) {
        p0.X, p0.Y, p0.Z, 0.0
    };// = new dPoint(p0.X, p0.Y, p0.Z);

    retPoint.Z = p1.Z + (((p1.Z - p2.Z) / (p1.X - p2.X)) * (p0.X - p1.X));

    //TODO: учесть на будущее что точка 1 и 2 могут лежать не на одной паралльной линии оси Х
    retPoint.Y = p1.Y;

    return retPoint;
}


//TODO: деление на ноль
//нахождение высоты Z точки p0, лежащей на прямой между точками p3 p4  (прямая паралельна оси Y)
dPoint Geometry::CalcPY(dPoint p1, dPoint p2, dPoint p0)
{
    dPoint retPoint  = (dPoint) {
        p0.X, p0.Y, p0.Z, 0.0
    };//= new dobPoint(p0.X, p0.Y, p0.Z);

    retPoint.Z = p1.Z + (((p1.Z - p2.Z) / (p1.Y - p2.Y)) * (p0.Y - p1.Y));

    return retPoint;
}




