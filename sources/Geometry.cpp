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

#include <QtCore/qmath.h>
#include "includes/Geometry.h"

enum AxisNames { X = 0, Y, Z, A, B, C, U, V, W };


/******************************************************************************
** Geometry
*/
// http://people.sc.fsu.edu/~jburkardt/m_src/m_src.html
// see the Hermite polynomial calculation

// public static class Geometry
// {
/*
 *    Корректировка высоты по axis Z, у точки 5, зная высоту по Z у точек 1,2,3,4
 *
 *  /\ axis Y
 *  |
 *  |     (point 1) -------------*--------------- (point 2)
 *  |                            |
 *  |                            |
 *  |                            |
 *  |                        (point 5)
 *  |                            |
 *  |                            |
 *  |     (point 3) -------------*--------------- (point 4)
 *  |
 *  |
 *  *----------------------------------------------------------------> axis X
 *  Корректировка выполняется следующим образом:
 *  1) зная координату X у точки 5, и координаты точек 1 и 2, вычисляем высоту Z в точке которая находится на линии точек 1,2 и перпендикулярно 5-й точке (получает точку 1..2)
 *  2) Тоже самое вычисляется для точки на линии точек 3,4 (получает точку 3..4)
 *  3) Зная координаты точек 1..2, 3..4 и значение по axis Y у точки 5, вычисляем высоту по axis Z
 */


///
/// methode for correction of heght of Z axis
///
/// p1 first point of second line X
/// p2 second point of first line X
/// p3 first point of second line X
/// p4 second point of second line X
/// p5 correcture pont for the Z calculation
///

coord Geometry::GetZ(coord p1, coord p2, coord p3, coord p4, coord p5)
{
    coord p12 = CalcPX(p1, p2, p5);
    coord p34 = CalcPX(p3, p4, p5);

    coord p1234 = CalcPY(p12, p34, p5);

    return p1234;
}


//нахождение высоты Z точки p0, лежащей на прямой которая паралельна оси X
coord Geometry::CalcPX(coord p1, coord p2, coord p0)
{
    coord retPoint = (coord) {
        p0.pos[X], p0.pos[Y], p0.pos[Z], 0.0
    };

    if ((p1.pos[X] != p2.pos[X]) && (p0.pos[X] != p1.pos[X])) {
        retPoint.pos[Z] = p1.pos[Z] + (((p1.pos[Z] - p2.pos[Z]) / (p1.pos[X] - p2.pos[X])) * (p0.pos[X] - p1.pos[X]));
    }

    //TODO: учесть на будущее что точка 1 и 2 могут лежать не на одной паралльной линии оси Х
    retPoint.pos[Y] = p1.pos[Y];

    return retPoint;
}

//
//нахождение высоты Z точки p0, лежащей на прямой между точками p3 p4  (прямая паралельна оси Y)
coord Geometry::CalcPY(coord p1, coord p2, coord p0)
{
    coord retPoint  = (coord) {
        p0.pos[X], p0.pos[Y], p0.pos[Z], 0.0
    };

    if ((p1.pos[Y] != p2.pos[Y]) && (p0.pos[Y] != p1.pos[Y])) {
        retPoint.pos[Z] = p1.pos[Z] + (((p1.pos[Z] - p2.pos[Z]) / (p1.pos[Y] - p2.pos[Y])) * (p0.pos[Y] - p1.pos[Y]));
    }

    return retPoint;
}



// source:
// http://blog.demofox.org/2015/08/09/cubic-hermite-rectangles/
// http://blog.demofox.org/2015/08/08/cubic-hermite-interpolation/

// QVector< QVector <dPoint> > c_ControlPointsX = {
//         {
//     { 0.7, 0.8, 0.9, 0.3 },
//     { 0.2, 0.5, 0.4, 0.1 },
//     { 0.6, 0.3, 0.1, 0.4 },
//     { 0.8, 0.4, 0.2, 0.7 }
//         }
// };
//
//
// const vector<float> c_ControlPointsY[][4] = {
// //         {
//     { 0.2, 0.8, 0.5, 0.6 },
//     { 0.6, 0.9, 0.3, 0.8 },
//     { 0.7, 0.1, 0.4, 0.9 },
//     { 0.6, 0.5, 0.3, 0.2 }
// //         }
// };
//
//
// const vector<float> c_ControlPointsZ[][4] = {
// //         {
//     { 0.6, 0.5, 0.3, 0.2 },
//     { 0.7, 0.1, 0.9, 0.5 },
//     { 0.8, 0.4, 0.2, 0.7 },
//     { 0.6, 0.3, 0.1, 0.4 }
// //         }
// };

// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
// when t is 0, this will return B.  When t is 1, this will return C.
float Geometry::cubicHermiteInterpolate (const float v[4], float t)
{
    float a0 = -0.5 * v[0] + 1.5 * v[1] - 1.5 * v[2] + 0.5 * v[3];
    float a1 = v[0] - 2.5 * v[1] + 2.0f * v[2] - 0.5 * v[3];
    float a2 = -0.5 * v[0] + 0.5 * v[2];
    float a3 = v[1];

    return a0 * t * t * t + a1 * t * t + a2 * t + a3;
}


// call this function for X, Y, Z separate
float Geometry::bicubicHermitePatch(const float vv[4][4], float u, float v)
{
    float uValues[4];
    uValues[0] = cubicHermiteInterpolate(vv[0], u);
    uValues[1] = cubicHermiteInterpolate(vv[1], u);
    uValues[2] = cubicHermiteInterpolate(vv[2], u);
    uValues[3] = cubicHermiteInterpolate(vv[3], u);
    return cubicHermiteInterpolate(uValues, v);
}


float Geometry::bicubicInterpolate(QRectF borderRect, QAbstractTableModel *basePoints, float x, float y)
{
    // Setup grid
    int gridPointsX = basePoints->columnCount();
    int gridPointsY = basePoints->rowCount();

    float gridStepX = gridPointsX > 1 ? borderRect.width() / (gridPointsX - 1) : 0;
    float gridStepY = gridPointsY > 1 ? borderRect.height() / (gridPointsY - 1) : 0;

    // Get 16 points
    x -= borderRect.x();
    y -= borderRect.y();

    int ix = qFloor(x / gridStepX);
    int iy = qFloor(y / gridStepY);

    if (ix > basePoints->columnCount() - 2) {
        ix = basePoints->columnCount() - 2;
    }

    if (iy > basePoints->rowCount() - 2) {
        iy = basePoints->rowCount() - 2;
    }

    float p[4][4];

    p[0][0] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), (ix > 0 ? ix - 1 : ix)), Qt::UserRole).toFloat();
    p[0][1] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), ix), Qt::UserRole).toFloat();
    p[0][2] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), ix + 1), Qt::UserRole).toFloat();
    p[0][3] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), (ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1)), Qt::UserRole).toFloat();

    p[1][0] = basePoints->data(basePoints->index(iy, ix > 0 ? ix - 1 : ix), Qt::UserRole).toFloat();
    p[1][1] = basePoints->data(basePoints->index(iy, ix), Qt::UserRole).toFloat();
    p[1][2] = basePoints->data(basePoints->index(iy, ix + 1), Qt::UserRole).toFloat();
    p[1][3] = basePoints->data(basePoints->index(iy, ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1), Qt::UserRole).toFloat();

    p[2][0] = basePoints->data(basePoints->index(iy + 1, ix > 0 ? ix - 1 : ix), Qt::UserRole).toFloat();
    p[2][1] = basePoints->data(basePoints->index(iy + 1, ix), Qt::UserRole).toFloat();
    p[2][2] = basePoints->data(basePoints->index(iy + 1, ix + 1), Qt::UserRole).toFloat();
    p[2][3] = basePoints->data(basePoints->index(iy + 1, ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1), Qt::UserRole).toFloat();

    p[3][0] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix > 0 ? ix - 1 : ix), Qt::UserRole).toFloat();
    p[3][1] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix), Qt::UserRole).toFloat();
    p[3][2] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix + 1), Qt::UserRole).toFloat();
    p[3][3] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1), Qt::UserRole).toFloat();

    // Interpolate
    return bicubicHermitePatch(p, x / gridStepX - ix, y / gridStepY - iy);
}


const QVector<int> Geometry::calculateAntPath(const QVector3D &v)
{
    points = v.length();

    path.clear();
    path.resize(points);
  
    for (int i = 0; i < distance.size(); ++i) {
        distance[i].clear();
    }
    distance.clear();
    
    distance.resize(points);
    for (int i = 0; i < distance.size(); ++i) {
        distance[i].resize(points);
    }
    
    AntColonyOptimization();
    
    return path;
}
/**
 * @brief 
 * 
 * @see Ant Colony Optimization algorihms
 * @link https://hackaday.io/project/4955-g-code-optimization
 */
void Geometry::AntColonyOptimization(/*int[] path, double[][] dis*/)
{
//     int cities = dis.GetLength(0), i, j;
//     START:
    if (points == 0){
        return;
    }
    
    for (int i = 0; i < points - 2; i++){
        for (int j = i + 2; j < points; j++){
            float swap_length = distance[path[i]][path[j]] + distance[path[i + 1]][path[j + 1]];
            float old_length = distance[path[i]][path[i + 1]] + distance[path[j]][path[j + 1]];
            if (swap_length < old_length) {
                // Make the new and shorter path.
                for (int x = 0; x < (j - i) / 2; x++) {
                    // swap 
                    int temp = path[i + 1 + x];
                    path[i + 1 + x] = path[j - x];
                    path[j - x] = temp;
                }
                // recursive
                AntColonyOptimization();
//                 goto START;
            }
        }
    }
//     return path;
}


bool Geometry::gernerateBicubicHermiteField()
{
    // how many values to display on each axis. Limited by console resolution!
    const int c_numValues = 4;

    //     printf("Cubic Hermite rectangle:\n");
    for (int i = 0; i < c_numValues; ++i) {
        float iPercent = ((float)i) / ((float)(c_numValues - 1));

        for (int j = 0; j < c_numValues; ++j) {
            //             if (j == 0)
            //                 printf("  ");
            float jPercent = ((float)j) / ((float)(c_numValues - 1));

            //             float valueX = BicubicHermitePatch(*c_ControlPointsX, jPercent, iPercent);
            //             float valueY = BicubicHermitePatch(*c_ControlPointsY, jPercent, iPercent);
            //             float valueZ = BicubicHermitePatch(*c_ControlPointsZ, jPercent, iPercent);
            //             printf("(%0.2f, %0.2f, %0.2f) ", valueX, valueY, valueZ);
        }

        //         printf("\n");
    }

    //     printf("\n");
    return true;
}



