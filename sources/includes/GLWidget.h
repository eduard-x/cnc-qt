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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL>
#include <QEvent>
#include <QWidget>
#include <QGLWidget>
#include <QMainWindow>
#include <QImage>
#include <QTimeLine>
#include <QSvgRenderer>

#include "Geometry.h"
#include "MainWindow.h"

class MainWindow;


struct pointGL {
    GLfloat X;       // координата в мм
    GLfloat Y;       // координата в мм
    GLfloat Z;       // координата в мм
};

struct colorGL {
    GLfloat r;
    GLfloat g;
    GLfloat b;
};


class GLWidget : public QGLWidget
{
        Q_OBJECT

    public:
        GLWidget(QWidget *parent = 0);
        ~GLWidget();

        void matrixReloaded();

    private:
        void Draw();

        void initPreviewSettings();
        void normalizeAngle(int *angle);
        //         void Init3D();

        void drawGrate();
        void drawInstrument();
        void drawSurface();
        void drawAxes();
        void drawWorkField();
        void drawGrid();

    public slots:
        void onRenderTimer();  // not connected

        // 3d buttons
        void onPosAngleXm();
        void onPosAngleX(); // to reset
        void onPosAngleXp();

        void onPosAngleYp();
        void onPosAngleY(); // to reset
        void onPosAngleYm();

        void onPosAngleZp();
        void onPosAngleZ(); // to reset
        void onPosAngleZm();


        void onDefaulPreview();
        // end of 3d buttons


        void setXRotation(int angle);
        void setYRotation(int angle);
        void setZRotation(int angle);

        void setYCoord(int y);
        void setXCoord(int x);

    signals:
        void xRotationChanged(int angle);
        void yRotationChanged(int angle);
        void zRotationChanged(int angle);
        void scaleChanged(int scale);

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void timerEvent(QTimerEvent *);
        void wheelEvent(QWheelEvent *);

        void saveGLState();
        void restoreGLState();
        //         void paintEvent(QPaintEvent *);

    private slots:
        //         void repaintView();


    private:
        MainWindow* parent;
        static GLint instrumentArray[][3];
        static GLint xArray[][3];
        static GLint yArray[][3];
        static GLint zArray[][3];

        pointGL *workArray; //
        colorGL *colorArray; //
        int workNum;

        QPoint lastPos;
};


#endif // GLWIDGET_H

