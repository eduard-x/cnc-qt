/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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


#include <QString>
#include <QEvent>
#include <QWidget>

#include <QFont>
#include <QBasicTimer>
#include <QObject>

#include <QtOpenGL/QGLShaderProgram>
#include <QtOpenGL/QGLWidget>

#include <QtOpenGL/QGLFunctions>

#include <QSlider>
#include <QMainWindow>
#include <QTimeLine>


#include "Geometry.h"
#include "MainWindow.h"
#include "Translator.h"


struct VertexData {
    QVector3D coord;
    QVector3D color;
};


class cTranslator;
class MainWindow;
class mk1Controller;

// for GLES2 are QGLFunctions to implement
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions, public cTranslator
{
        Q_OBJECT

    public:
        explicit GLWidget(QWidget *parent = 0);
        ~GLWidget();

        void loadFigure();
        void surfaceReloaded();
        void initStaticElements();
        void Draw();

    private:
        void initPreviewSettings();
        float normalizeAngle(float angle);
        void createButtons();
        void displayRotation();
        QVector<VertexData> addPointVector(const QVector<QVector3D> &p, const QColor &c);
        QVector<QVector3D> textToVector(double x, double y, double z, const QString &s, const QFont & font = QFont());

    public slots:
        //         void onRenderTimer();  // not connected

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


        void setIso();
        void setLeft();
        void setTop();
        void setFront();
        void setFit();
        void setZoom(int i);

        // end of 3d buttons


        void setXRotation(int angle);
        void setYRotation(int angle);
        void setZRotation(int angle);

        void setYCoord(int y);
        void setXCoord(int x);

    signals:
        void fpsChanged(int val);
        //         void rotationChanged(void);
        void scaleChanged(int scale);

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *e);
        void timerEvent(QTimerEvent *e);

    private slots:
        void showFPS(void);


    private:
        MainWindow* parent;
        mk1Controller* cnc;

        QBasicTimer timer;
        QOpenGLShaderProgram *program;

        GLuint m_posAttr;
        GLuint m_startAttr;
        GLuint m_idx;
        GLuint m_colAttr;
        GLuint m_matrixUniform;
        GLuint m_mvUniform;
        GLfloat m_pointSizeUniform;

        //         GLfloat m_lineWidth;
        //         GLfloat m_pointSize;

        QToolButton *cmdFit;
        QToolButton *cmdIsometric;
        QToolButton *cmdTop;
        QToolButton *cmdFront;
        QToolButton *cmdLeft;

        QToolButton *cmdX[3];
        QToolButton *cmdY[3];
        QToolButton *cmdZ[3];

        QSlider *cmdZoom;

        static QVector<QVector3D> instrumentArray;
        static QVector<QVector3D> footArray;
        static QVector<QVector3D> traverseArray;

        QVector<QVector3D> holderArray;
        QVector<QVector3D> motorArray;

        static QVector<QVector3D> xAxis;
        static QVector<QVector3D> yAxis;
        static QVector<QVector3D> zAxis;

        QVector<QVector3D> surfaceArray; //

        QVector<VertexData> figure;
        QVector<VertexData> border;
        QVector<VertexData> axis;
        QVector<VertexData> instrument;
        QVector<VertexData> gridLines;
        QVector<VertexData> gridPoints;
        QVector<VertexData> surfaceLines;
        QVector<VertexData> surfacePoints;
        
        QVector<VertexData> vText;

        float fps;
        double m_zoom;
        double m_distance;
        double m_xRot, m_yRot, m_xLastRot, m_yLastRot;
        double m_xPan, m_yPan, m_xLastPan, m_yLastPan;
        double m_xLookAt, m_yLookAt, m_zLookAt;
        QPoint m_lastPos;

    private:
        Q_DISABLE_COPY(GLWidget);
};


#endif // GLWIDGET_H

