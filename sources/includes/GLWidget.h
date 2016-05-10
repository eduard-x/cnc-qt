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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "version.h"


#include <QEvent>
#include <QWidget>

#include <QtOpenGL/QGLWidget>

#include <QGLShader>
#include <QGLShaderProgram>

#include <QtOpenGL/QGLFunctions>

#include <QMainWindow>
#include <QTimeLine>


#include "Geometry.h"
#include "MainWindow.h"

class MainWindow;
class mk1Controller;



// for GLES2 are QGLFunctions to implement
class GLWidget : public QGLWidget, protected QGLFunctions
{
        Q_OBJECT

    public:
        GLWidget(QWidget *parent = 0);
        ~GLWidget();

        void matrixReloaded();
        void surfaceReloaded();
        void Draw();

    private:
        void initPreviewSettings();
        void initStaticElements();

        void normalizeAngle(int &angle);

        void drawGrate();
        void drawInstrument();
        void drawSurface();
        void drawTool();
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
        void fpsChanged(int val);
        void rotationChanged(void);
        //         void yRotationChanged(int angle);
        //         void zRotationChanged(int angle);
        void scaleChanged(int scale);

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *);

        void saveGLState();
        void restoreGLState();
        //         void paintEvent(QPaintEvent *);

    private slots:
        //         void repaintView();
        void showFPS(void);
        void processing(void);


    private:
        MainWindow* parent;
        mk1Controller* cnc;

        QGLShaderProgram* program;

        GLuint m_posAttrX;
        GLuint m_posAttrY;
        GLuint m_posAttrZ;

        QMatrix4x4 viewMatrix; // Projection matrix
        //         QMatrix4x4 Model; // Model matrix
        //         QMatrix4x4 View; // Camera matrix
        //         QMatrix4x4 MVP;

        GLuint vertexbuffer;
        GLuint colorbuffer;

        QVector<pointGL> instrumentArray;
        QVector<pointGL> footArray;
        QVector<pointGL> traverseArray;
        QVector<pointGL> holderArray;
        QVector<pointGL> motorArray;
        QVector<pointGL> xAxis;
        QVector<pointGL> yAxis;
        QVector<pointGL> zAxis;

        QVector<pointGL> surfaceArray; //
        QVector<pointGL> coordArray; //
        QVector<colorGL> colorArray; //

        QTimer *glTimer;

        float fps;

        QPoint lastPos;
};


#endif // GLWIDGET_H

