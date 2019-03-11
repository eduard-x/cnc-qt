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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "version.h"


#include <QString>
#include <QEvent>
#include <QWidget>
#include <QVector>

#include <QFont>
#include <QBasicTimer>
#include <QObject>

#include <QGLShaderProgram>
#include <QGLWidget>

#include <QGLFunctions>

#include <QSlider>
#include <QMainWindow>
#include <QTimeLine>

#include "GData.h"
#include "Geometry.h"
#include "MainWindow.h"
#include "Translator.h"


class cTranslator;
class MainWindow;
class mk1Controller;

// for GLES2 in Qt4 are QGLFunctions to implement
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
        void initVertexArrays();
        void initPreviewSettings();
        void createButtons();
        void displayRotation();
        float normalizeAngle(float angle);
        float calculateVolume(QVector3D size);
        void beginViewAnimation();
        void stopViewAnimation();

        QTime spendTime() const;
        void setSpendTime(const QTime &spendTime);

        QTime estimatedTime() const;
        void setEstimatedTime(const QTime &estimatedTime);

        void setFps(int fps);

        QVector<QVector3D> generateCone();
        QVector<QVector3D> generateCylinder();
        QVector<VertexData> addPointVector(const QVector<QVector3D> &p, const QColor &c);
        QVector<QVector<VertexData> > textToVector(float x, float y, float z, const QString &s, const QColor &c, int direction, const QFont & font = QFont());

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
        void scaleChanged(int scale);
        void rotationChanged();
        void resized();

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *e);
        void timerEvent(QTimerEvent *e);
        void updateProjection();
        void updateView();

    private slots:
        void showFPS(void);

        void onFramesTimer();
        void viewAnimation();


    private:
        MainWindow* parent;
        mk1Controller* cnc;

        QBasicTimer timer;
        QOpenGLShaderProgram *m_shaderProgram;

        QMatrix4x4 m_projectionMatrix;
        QMatrix4x4 m_viewMatrix;

        QColor m_colorBackground;
        QColor m_colorText;

        float fps;

        float m_xRot, m_yRot, m_xLastRot, m_yLastRot;
        float m_xPan, m_yPan, m_xLastPan, m_yLastPan;
        float m_xLookAt, m_yLookAt, m_zLookAt;
        QPoint m_lastPos;
        float m_zoom;
        float m_distance;
        float m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax, m_xSize, m_ySize, m_zSize;
        float m_lineWidth;
        bool m_antialiasing;
        bool m_msaa;
        bool m_zBuffer;
        int m_frames = 0;
        int m_fps = 0;
        int m_targetFps;
        int m_animationFrame;
        QTime m_spendTime;
        QTime m_estimatedTime;
        QBasicTimer m_timerAnimation;
        float m_xRotTarget, m_yRotTarget;
        float m_xRotStored, m_yRotStored;
        bool m_animateView;
        QString m_parserStatus;
        QString m_bufferState;
        bool m_updatesEnabled;

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

        QVector<QVector3D> footArray;
        QVector<QVector3D> traverseArray;

        QVector<QVector3D> holderArray;
        QVector<QVector3D> motorArray;

        QVector<QVector3D> xAxis;
        QVector<QVector3D> yAxis;
        QVector<QVector3D> zAxis;

        QVector<QVector3D> surfaceArray; //

        QVector<VertexData> figure;
        QVector<VertexData> border;
        QVector<VertexData> axis;
        QVector<VertexData> messure;
        QVector<VertexData> instrument;
        QVector<VertexData> gridLines;
        QVector<VertexData> gridPoints;
        QVector<VertexData> surfaceLines;
        QVector<VertexData> surfacePoints;

        QVector< QVector<VertexData> > aText;
        QVector< QVector<VertexData> > mText;

    private:
        Q_DISABLE_COPY(GLWidget);
};


#endif // GLWIDGET_H

