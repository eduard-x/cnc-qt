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

#include "version.h"

#include "GLWidget.h"

#include <QDebug>

#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLShaderProgram>
#include <QtOpenGL/QGLWidget>
#include <QSurfaceFormat>
#include <QToolButton>

// for GLES2 are QGLFunctions to implement

#include "Settings.h"
#include "MainWindow.h"
#include "mk1Controller.h"


#define ZOOMSTEP 1.1

// #include "shader.h"

/**
 * @brief constructor
 *
 * switched to OpenGL ES2 for hardware raspberry supporting
 * @see https://www.khronos.org/opengles/sdk/docs/man/xhtml/
 * @see Qt5 OpenGL: http://mepem.com/pemcode/?p=243
 */
GLWidget::GLWidget(QWidget *p)
    : QOpenGLWidget(p)//, indexVBO(QOpenGLBuffer::IndexBuffer)
{
    if (p == NULL) {
        return;
    }

    parent = static_cast<MainWindow*>(p);

    cnc = parent->mk1;

    m_animateView = false;
    m_updatesEnabled = false;

    m_xRot = 90;
    m_yRot = 0;

    m_xRotTarget = 90;
    m_yRotTarget = 0;

    m_zoom = 1;

    m_xPan = 0;
    m_yPan = 0;
    m_distance = 100;

    m_xLookAt = 0;
    m_yLookAt = 0;
    m_zLookAt = 0;

    m_xMin = 0;
    m_xMax = 0;
    m_yMin = 0;
    m_yMax = 0;
    m_zMin = 0;
    m_zMax = 0;
    m_xSize = 0;
    m_ySize = 0;
    m_zSize = 0;

    initVertexArrays();

    Settings::PosX = -50;
    Settings::PosY = -50;

    //     m_lineWidth = 1.0;
    //     m_pointSize = 6.0;

    fps = 0;

    QTimer* fpsTimer = new QTimer();
    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(showFPS()));
    fpsTimer->start(1000);

    //
    for (int i = 0; i < 3; i++) {
        cmdX[i] = NULL;
        cmdY[i] = NULL;
        cmdZ[i] = NULL;
    }

    setMinimumSize(350, 200);

    //     QSurfaceFormat f;
    //     f.version();
    //     qDebug() << "GL enabled" << f.version() << "flags:" << f.profile();
    //     QString glStr = QString().sprintf("%s %s %s", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));
    //
    //     qDebug() << glStr;

    initStaticElements();


    initPreviewSettings();
}

/**
 * @brief destructor
 *
 */
GLWidget::~GLWidget()
{
    //     arrayVBO.destroy();
    //     indexVBO.destroy();
}


void GLWidget::setFps(int fps)
{
    if (fps <= 0) {
        return;
    }

    m_targetFps = fps;
    m_timerAnimation.stop();
    m_timerAnimation.start(1000 / fps, Qt::PreciseTimer, this);
}


void GLWidget::onFramesTimer()
{
    m_fps = m_frames;
    m_frames = 0;

    QTimer::singleShot(1000, this, SLOT(onFramesTimer()));
}

void GLWidget::viewAnimation()
{
    double t = (double)m_animationFrame++ / (m_targetFps * 0.2);

    if (t == 1) {
        stopViewAnimation();
    }

    QEasingCurve ec(QEasingCurve::OutExpo);
    double val = ec.valueForProgress(t);

    m_xRot = m_xRotStored + double(m_xRotTarget - m_xRotStored) * val;
    m_yRot = m_yRotStored + double(m_yRotTarget - m_yRotStored) * val;

    updateView();
}

QTime GLWidget::estimatedTime() const
{
    return m_estimatedTime;
}

void GLWidget::setEstimatedTime(const QTime &estimatedTime)
{
    m_estimatedTime = estimatedTime;
}

QTime GLWidget::spendTime() const
{
    return m_spendTime;
}

void GLWidget::setSpendTime(const QTime &spendTime)
{
    m_spendTime = spendTime;
}

/**
 * @brief change the information about rotations on the push buttons
 *
 */
void GLWidget::displayRotation()
{
    if (cmdX[1] != NULL ) {
        cmdX[1]->setText( QString().sprintf("X: %d°", Settings::PosAngleX));
    }

    if (cmdY[1] != NULL ) {
        cmdY[1]->setText( QString().sprintf("Y: %d°", Settings::PosAngleY));
    }

    if (cmdZ[1] != NULL ) {
        cmdZ[1]->setText( QString().sprintf("Z: %d°", Settings::PosAngleZ));
    }
}


void GLWidget::createButtons()
{
    cmdIsometric = new QToolButton((QWidget*)this);
    cmdIsometric->setBaseSize(QSize(24, 24));
    cmdIsometric->resize(24, 24);
    cmdIsometric->setToolTip(translate(ID_ISO));//"Iso");

    cmdTop = new QToolButton(this);
    cmdTop->setBaseSize(QSize(24, 24));
    cmdTop->resize(24, 24);
    cmdTop->setToolTip(translate(ID_TOP));

    cmdFront = new QToolButton(this);
    cmdFront->setBaseSize(QSize(24, 24));
    cmdFront->resize(24, 24);
    cmdFront->setToolTip(translate(ID_FRONT));

    cmdLeft = new QToolButton(this);
    cmdLeft->setBaseSize(QSize(24, 24));
    cmdLeft->resize(24, 24);
    cmdLeft->setToolTip(translate(ID_LEFT));

    cmdFit = new QToolButton(this);
    cmdFit->setBaseSize(QSize(24, 24));
    cmdFit->resize(24, 24);
    cmdFit->setToolTip(translate(ID_FIT));

    cmdFit->setIcon(QIcon(":/images/fit_1.png"));
    cmdIsometric->setIcon(QIcon(":/images/cube.png"));
    cmdFront->setIcon(QIcon(":/images/cubeFront.png"));
    cmdLeft->setIcon(QIcon(":/images/cubeLeft.png"));
    cmdTop->setIcon(QIcon(":/images/cubeTop.png"));

    cmdZoom = new QSlider(Qt::Vertical, this);
    cmdZoom->setBaseSize(QSize(24, 240));
    cmdZoom->resize(24, 240);
    cmdZoom->setRange(0, 500);
    //     cmdZoom->setTickInterval(50);

    cmdZoom->setToolTip("Zoom");

    connect(cmdIsometric, SIGNAL(clicked(bool)), this, SLOT(setIso()));
    connect(cmdFit, SIGNAL(clicked(bool)), this, SLOT(setFit()));
    connect(cmdLeft, SIGNAL(clicked(bool)), this, SLOT(setLeft()));
    connect(cmdFront, SIGNAL(clicked(bool)), this, SLOT(setFront()));
    connect(cmdTop, SIGNAL(clicked(bool)), this, SLOT(setTop()));

    connect(cmdZoom, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));

    cmdZoom->setValue(Settings::PosZoom);


    for (int i = 0; i < 3; i++) {
        cmdX[i] = new QToolButton(this);

        if(i != 1) {
            cmdX[i]->setBaseSize(QSize(24, 24));
            cmdX[i]->resize(24, 24);
        } else {
            cmdX[i]->setBaseSize(QSize(24 * 3, 24));
            cmdX[i]->resize(24 * 3, 24);
        }
    }

    cmdX[0]->setIcon(QIcon(":/images/undo.png"));
    cmdX[2]->setIcon(QIcon(":/images/redo.png"));

    for (int i = 0; i < 3; i++) {
        cmdY[i] = new QToolButton(this);

        if(i != 1) {
            cmdY[i]->setBaseSize(QSize(24, 24));
            cmdY[i]->resize(24, 24);
        } else {
            cmdY[i]->setBaseSize(QSize(24 * 3, 24));
            cmdY[i]->resize(24 * 3, 24);
        }
    }

    cmdY[0]->setIcon(QIcon(":/images/undo.png"));
    cmdY[2]->setIcon(QIcon(":/images/redo.png"));


    for (int i = 0; i < 3; i++) {
        cmdZ[i] = new QToolButton(this);

        if(i != 1) {
            cmdZ[i]->setBaseSize(QSize(24, 24));
            cmdZ[i]->resize(24, 24);
        } else {
            cmdZ[i]->setBaseSize(QSize(24 * 3, 24));
            cmdZ[i]->resize(24 * 3, 24);
        }
    }

    cmdZ[0]->setIcon(QIcon(":/images/undo.png"));
    cmdZ[2]->setIcon(QIcon(":/images/redo.png"));

    connect(cmdX[0], SIGNAL(pressed()), this, SLOT(onPosAngleXm()));
    connect(cmdX[1], SIGNAL(clicked()), this, SLOT(onPosAngleX())); // reset to 0
    //         connect(scene3d, SIGNAL(rotationChanged()), this, SLOT(getRotation()));
    //         connect(scene3d, SIGNAL(fpsChanged(int)), this, SLOT(getFPS(int)));
    connect(cmdX[2], SIGNAL(pressed()), this, SLOT(onPosAngleXp()));

    connect(cmdY[0], SIGNAL(pressed()), this, SLOT(onPosAngleYm()));
    connect(cmdY[1], SIGNAL(clicked()), this, SLOT(onPosAngleY())); // reset to 0
    //         connect(scene3d, SIGNAL(yRotationChanged(int)), this, SLOT(getYRotation(int)));
    connect(cmdY[2], SIGNAL(pressed()), this, SLOT(onPosAngleYp()));

    connect(cmdZ[0], SIGNAL(pressed()), this, SLOT(onPosAngleZm()));
    connect(cmdZ[1], SIGNAL(clicked()), this, SLOT(onPosAngleZ())); // reset to 0
    //         connect(scene3d, SIGNAL(zRotationChanged(int)), this, SLOT(getZRotation(int)));
    connect(cmdZ[2], SIGNAL(pressed()), this, SLOT(onPosAngleZp()));

    displayRotation();
}

void   GLWidget::initVertexArrays()
{
    xAxis = {
        { 0.0, 0.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 9.0, 1.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 9.0, -1.0, 0.0 }
    };

    yAxis = {
        { 0.0, 0.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { 1.0, 9.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { -1.0, 9.0, 0.0 }
    };

    zAxis = {
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 10.0 },
        { 0.0, 0.0, 10.0 },
        { 1.0, 1.0, 9.0 },
        { 0.0, 0.0, 10.0 },
        { -1.0, -1.0, 9.0 }
    };


    footArray = { // GL_LINE_LOOP array
        { 0.0, 0.0, 0.0 },      // 0
        { 0.0, 22.0, 0.0 },
        { 0.0, 22.0, 29.0 },
        { 0.0, 12.0, 29.0 },
        { 0.0, 0.0, 12.0 },
        { 0.0, 0.0, 0.0 },
        { 3.6, 0.0, 0.0 },
        { 3.6, 22.0, 0.0 },
        { 3.6, 22.0, 29.0 },
        { 3.6, 12.0, 29.0 },
        { 3.6, 0.0, 12.0 },     // 10
        { 3.6, 0.0, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 12.0 },
        { 3.6, 0.0, 12.0 },
        { 3.6, 12.0, 29.0 },
        { 0.0, 12.0, 29.0 },
        { 0.0, 22.0, 29.0 },
        { 3.6, 22.0, 29.0 },
        { 3.6, 22.0, 0.0 },
        { 0.0, 22.0, 0.0 },     // 20
        { 0.0, 0.0, 0.0 }
    };

    traverseArray = { // width of traverse is 64 cm
        { 0.0, 0.0, 0.0 },
        { 64.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 1.2, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 12.0 },
        { 0.0, 1.2, 0.0 },
        { 64.0, 1.2, 0.0 },
        { 0.0, 1.2, 0.0 },
        { 0.0, 1.2, 12.0 },
        { 0.0, 0.0, 12.0 },
        { 0.0, 1.2, 12.0 },
        { 0.0, 0.0, 12.0 },
        { 64.0, 0.0, 12.0 },
        { 0.0, 1.2, 12.0 },
        { 64.0, 1.2, 12.0 },
        { 64.0, 0.0, 0.0 },
        { 64.0, 1.2, 0.0 },
        { 64.0, 0.0, 12.0 },
        { 64.0, 1.2, 12.0 },
        { 64.0, 0.0, 0.0 },
        { 64.0, 0.0, 12.0 },
        { 64.0, 1.2, 0.0 },
        { 64.0, 1.2, 12.0 }
    };
}

/**
 *
 *
 */
void GLWidget::setIso()
{
    Settings::PosAngleX = -45;
    Settings::PosAngleY = 0;
    Settings::PosAngleZ = -45;
    initPreviewSettings();
}


/**
 *
 *
 */
void GLWidget::setLeft()
{
    Settings::PosAngleX = 0;
    Settings::PosAngleY = 90;
    Settings::PosAngleZ = 0;
    initPreviewSettings();
}


/**
 *
 *
 */
void GLWidget::setTop()
{
    Settings::PosAngleX = 0;
    Settings::PosAngleY = 0;
    Settings::PosAngleZ = 0;
    initPreviewSettings();
}

/**
 *
 *
 */
void GLWidget::setFront()
{
    Settings::PosAngleX = 90;
    Settings::PosAngleY = 90;
    Settings::PosAngleZ = 0;
    initPreviewSettings();
}

/**
 * @brief slot from qslider
 *
 */
void GLWidget::setZoom(int i)
{
    //     int v = cmdZoom->value();
    //      qDebug() << "zoom" << v;
    Settings::PosZoom = i; //((v - 50) * 0.01);

    if (Settings::smoothMoving) {
        update();
    }
}


#define DEF_D 30

QVector<QVector3D> GLWidget::generateCone()
{
    QVector<QVector3D> v;
    float h = Settings::ShowIntrumentHight;
    float r = Settings::ShowIntrumentDiameter * 0.5;

    float dRad = qDegreesToRadians((float)DEF_D);

    // for GL_TRIANGLES
    for (int k = 0; k < 360; k += DEF_D) {
        float kRad =  qDegreesToRadians((float)k);
        v << QVector3D(0, 0, 0);
        v << QVector3D(r * qCos(kRad), r * qSin(kRad), h);
        v << QVector3D(r * qCos(kRad + dRad), r * qSin(kRad + dRad), h);

        v << QVector3D(0, 0, 0);
        v << QVector3D(r * qCos(kRad), -r * qSin(kRad), h);
        v << QVector3D(r * qCos(kRad + dRad), -r * qSin(kRad + dRad), h);
    }

    return v;
}


QVector<QVector3D> GLWidget::generateCylinder()
{
    QVector<QVector3D> v;
    float h = Settings::ShowIntrumentHight;
    float r = Settings::ShowIntrumentDiameter * 0.5;

    float dRad = qDegreesToRadians((float)DEF_D);

    // for GL_TRIANGLES
    for (int k = 0; k < 360; k += DEF_D) {
        float kRad =  qDegreesToRadians((float)k);
        v << QVector3D(r * qCos(kRad), r * qSin(kRad), 0);
        v << QVector3D(r * qCos(kRad), r * qSin(kRad), h);
        v << QVector3D(r * qCos(kRad + dRad), r * qSin(kRad + dRad), h);

        v << QVector3D(r * qCos(kRad), -r * qSin(kRad), 0);
        v << QVector3D(r * qCos(kRad), -r * qSin(kRad), h);
        v << QVector3D(r * qCos(kRad + dRad), -r * qSin(kRad + dRad), h);
    }

    return v;
}


/**
 * @brief
 * @param x coordinate of text center
 *        y
 *        z
 *
 */
QVector<QVector<VertexData> > GLWidget::textToVector(float x, float y, float z, const QString &s, const QColor &c, int direction, const QFont &f)
{
    QPainterPath path;
    QVector<QVector<VertexData> > v;
    float coeff = 0.2;

    QFontMetricsF fm(f);
    float pixelsWide = fm.width(s) * coeff * 0.5; // 0.5 is half of text width
    float pixelsHigh = fm.height() * coeff * 0.25;

    path.addText(QPointF(0, 0), f, s);

    QList<QPolygonF> poly = path.toSubpathPolygons();

    switch (direction) {
        case X:
            for (QList<QPolygonF>::iterator i = poly.begin(); i != poly.end(); i++) {
                // generate the vector for GL_LINE_LOOP output
                QVector<VertexData> subv;

                for (QPolygonF::iterator p = (*i).begin(); p != i->end(); p++) {
                    subv << (VertexData) {
                        QVector3D(x - pixelsWide + p->rx() * coeff, y - pixelsHigh - p->ry() * coeff, z), QVector3D(c.redF(), c.greenF(), c.blueF())
                    };
                }

                v << subv;
            }

            return v;

        case Y:
            for (QList<QPolygonF>::iterator i = poly.begin(); i != poly.end(); i++) {
                // generate the vector for GL_LINE_LOOP output
                QVector<VertexData> subv;

                for (QPolygonF::iterator p = (*i).begin(); p != i->end(); p++) {
                    subv << (VertexData) {
                        QVector3D(x + pixelsHigh + p->ry() * coeff, y - pixelsWide + p->rx() * coeff, z), QVector3D(c.redF(), c.greenF(), c.blueF())
                    };
                }

                v << subv;
            }

            return v;

        case Z:
            for (QList<QPolygonF>::iterator i = poly.begin(); i != poly.end(); i++) {
                // generate the vector for GL_LINE_LOOP output
                QVector<VertexData> subv;

                for (QPolygonF::iterator p = (*i).begin(); p != i->end(); p++) {
                    subv << (VertexData) {
                        QVector3D(x + p->rx() * coeff, y, z - pixelsHigh - p->ry() * coeff), QVector3D(c.redF(), c.greenF(), c.blueF())
                    };
                }

                v << subv;
            }

            return v;
    }

    return v;
}

/**
 *
 *
 */
void GLWidget::setFit()
{
    qDebug() << "not implemented";
}


/**
 * @brief initialization of arrays after settings changing
 *
 */
void GLWidget::initStaticElements()
{
    axis.clear();

    foreach(QVector<VertexData> v, aText) {
        v.clear();
    }

    aText.clear();

    axis << addPointVector(xAxis, Settings::colorSettings[COLOR_X]);

    aText << textToVector(12, 0, 0, "X", Settings::colorSettings[COLOR_X], X);

    axis << addPointVector(yAxis, Settings::colorSettings[COLOR_Y]);

    aText << textToVector(0, 12, 0, "Y", Settings::colorSettings[COLOR_Y], Y);

    axis << addPointVector(zAxis, Settings::colorSettings[COLOR_Z]);

    aText << textToVector(-0.5, -0.5, 12, "Z", Settings::colorSettings[COLOR_Z], Z);

    instrument.clear();
    QVector<QVector3D> inst;

    if (Settings::ShowInstrumentCone) {
        inst = generateCone();
    } else {
        inst = generateCylinder();
    }

    instrument = addPointVector(inst, Settings::colorSettings[COLOR_TOOL]);

    border.clear();

    QVector3D brdColor = QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF());

    border = QVector<VertexData> {
        { QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0}, brdColor },
        { QVector3D{ Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin, 0}, brdColor },
        { QVector3D{ Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMax, 0}, brdColor },
        { QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMax, 0}, brdColor },
        { QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0}, brdColor }
    };

    gridLines.clear();

    QVector3D grdColor = QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF());

    for (int gX = Settings::GridXstart; gX < Settings::GridXend + 1; gX += Settings::GrigStep) {
        gridLines << (VertexData) {
            QVector3D{(GLfloat)gX, (GLfloat)Settings::GridYstart, 0.0 }, grdColor
        };
        gridLines << (VertexData) {
            QVector3D{(GLfloat)gX, (GLfloat)Settings::GridYend, 0.0}, grdColor
        };
    }

    for (int gY = Settings::GridYstart; gY < Settings::GridYend + 1; gY += Settings::GrigStep) {
        gridLines << (VertexData) {
            QVector3D{(GLfloat)Settings::GridXstart, (GLfloat)gY, 0.0}, grdColor
        };
        gridLines << (VertexData) {
            QVector3D{(GLfloat)Settings::GridXend, (GLfloat)gY, 0.0}, grdColor
        };
    }

    gridPoints.clear();

    for (int y = Settings::GridYstart; y <  Settings::GridYend + 1; y += Settings::GrigStep) {
        for (int x = Settings::GridXstart; x < Settings::GridXend + 1; x += Settings::GrigStep) {
            //point
            gridPoints << (VertexData) {
                QVector3D{(GLfloat)x, (GLfloat)y, 0.0}, grdColor
            };
        }
    }

    messure.clear();

    foreach(QVector<VertexData> v, mText) {
        v.clear();
    }

    mText.clear();

    if (border.count() > 0) {

        messure = QVector<VertexData> {
            // X messure
            { QVector3D(Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            // pfeil links
            { QVector3D(Settings::coord[X].softLimitMin + 2.5,  Settings::coord[Y].softLimitMin - 10.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin + 2.5,  Settings::coord[Y].softLimitMin - 9.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            // pfeil ende
            // pfeil rechts
            { QVector3D(Settings::coord[X].softLimitMax - 2.5, Settings::coord[Y].softLimitMin - 10.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax - 2.5, Settings::coord[Y].softLimitMin - 9.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin - 10.0, 0.0), grdColor },
            // pfeil ende
            { QVector3D(Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin - 7.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin - 12.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin - 7.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin - 12.5, 0.0), grdColor},
            // Y messure
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMin, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMax, 0.0), grdColor },
            // pfeil unten
            { QVector3D(Settings::coord[X].softLimitMin - 10.5,  Settings::coord[Y].softLimitMin + 2.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMin, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 9.5,  Settings::coord[Y].softLimitMin + 2.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMin, 0.0), grdColor },
            // pfeil ende
            // pfeil oben
            { QVector3D(Settings::coord[X].softLimitMin - 10.5,  Settings::coord[Y].softLimitMax - 2.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMax, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 9.5,  Settings::coord[Y].softLimitMax - 2.5, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMax, 0.0), grdColor },
            // pfeil ende
            { QVector3D(Settings::coord[X].softLimitMin - 7.5,  Settings::coord[Y].softLimitMin, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 12.5,  Settings::coord[Y].softLimitMin, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 7.5,  Settings::coord[Y].softLimitMax, 0.0), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 12.5,  Settings::coord[Y].softLimitMax, 0.0), grdColor },
            // Z messure
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMin - 10.0, Settings::coord[Z].softLimitMin), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 10.0,  Settings::coord[Y].softLimitMin - 10.0, Settings::coord[Z].softLimitMax), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 7.5,  Settings::coord[Y].softLimitMin - 7.5, Settings::coord[Z].softLimitMin), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 12.5,  Settings::coord[Y].softLimitMin - 12.5, Settings::coord[Z].softLimitMin), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 7.5,  Settings::coord[Y].softLimitMin - 7.5, Settings::coord[Z].softLimitMax), grdColor },
            { QVector3D(Settings::coord[X].softLimitMin - 12.5,  Settings::coord[Y].softLimitMin - 12.5, Settings::coord[Z].softLimitMax), grdColor }
        };

        QFont f("Times", 5);
        float xdiff = qFabs(Settings::coord[X].softLimitMax - Settings::coord[X].softLimitMin);
        mText << textToVector(xdiff * 0.5 + Settings::coord[X].softLimitMin, Settings::coord[Y].softLimitMin - 15.0, 0, QString().sprintf("%.2f mm", xdiff), Settings::colorSettings[COLOR_BORDER], X, f);

        float ydiff = qFabs(Settings::coord[Y].softLimitMax - Settings::coord[Y].softLimitMin);
        mText << textToVector( Settings::coord[X].softLimitMin - 15.0, ydiff * 0.5 + Settings::coord[Y].softLimitMin,  0, QString().sprintf("%.2f mm", ydiff), Settings::colorSettings[COLOR_BORDER], Y, f);

        float zdiff = qFabs(Settings::coord[Z].softLimitMax - Settings::coord[Z].softLimitMin);
        mText << textToVector(Settings::coord[X].softLimitMin - 10.0, Settings::coord[Y].softLimitMin - 10.0, zdiff * 0.5, QString().sprintf("%.2f mm", zdiff), Settings::colorSettings[COLOR_BORDER], Z, f);
    }

    int maxY, maxX;
    maxY = parent->surfaceMatrix.size();

    if (maxY == 0) {
        return;
    }

    maxX = parent->surfaceMatrix[0].size(); // because of rectangle matrix

    if (maxX == 0) {
        return;
    }

    surfaceLines.clear();
    surfacePoints.clear();

    QVector3D srfcColor = QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF());

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            surfaceLines << (VertexData) {
                QVector3D{parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z}, srfcColor
            };

            if (y > 0) {
                //line 1
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y - 1][x].X, parent->surfaceMatrix[y - 1][x].Y, parent->surfaceMatrix[y - 1][x].Z}, srfcColor
                };
            }

            if (y < maxY - 1) {
                //line2
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y + 1][x].X, parent->surfaceMatrix[y + 1][x].Y, parent->surfaceMatrix[y + 1][x].Z}, srfcColor
                };
            }

            if (x > 0) {
                //line 3
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y][x - 1].X, parent->surfaceMatrix[y][x - 1].Y, parent->surfaceMatrix[y][x - 1].Z}, srfcColor
                };
            }

            if (x < maxX - 1) {
                //line4
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y][x + 1].X, parent->surfaceMatrix[y][x + 1].Y, parent->surfaceMatrix[y][x + 1].Z}, srfcColor
                };
            }

            // now points
            surfacePoints << (VertexData) {
                QVector3D{parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z}, srfcColor
            };
        }
    }
}


/**
 * @brief after reload of surface matrix
 *
 */
void GLWidget::surfaceReloaded()
{
    for (int i = 0; i < figure.count(); i++) {
        if (parent->deltaFeed) {
            //             QVector3D p;
            QVector3D point = parent->serialDataVector.at(i)->coord;
            //             float pointY = parent->parseData.at(i).Y;
            //             float pointZ = parent->parseData.at(i).Z;

            point.setZ( point.z() + parent->getDeltaZ(point.x(), point.y()));

            figure[i].coord = point;
        }
    }
}


/**
 * @brief send current FPS number to the main widget
 *
 */
void GLWidget::showFPS()
{
    emit fpsChanged(fps);

    fps = 0;
}



/**
 * @brief main object data is reloaded or something in matrix was changed: coordinates or colors
 *        update the visualisation
 *
 */
void GLWidget::loadFigure()
{
    figure.clear();

    foreach (const SerialData *vv, parent->serialDataVector) {
        QColor cl;

        if (vv->movingCode == RAPID_LINE_CODE) {
            cl = Settings::colorSettings[COLOR_RAPID];
        } else {
            cl = Settings::colorSettings[COLOR_WORK];
        }

        // coordinates of next point
        float pointX = vv->coord.x();
        float pointY = vv->coord.y();
        float pointZ = vv->coord.z();

        // moving in G-code
        if (parent->Correction) {
            // proportions
            pointX *= parent->coeffSizeX;
            pointY *= parent->coeffSizeY;

            // offset
            pointX += parent->deltaX;
            pointY += parent->deltaY;
            pointZ += parent->deltaZ;

            // to use the scanned surface, z correcture
            if (parent->deltaFeed) {
                pointZ += parent->getDeltaZ(pointX, pointY);
            }
        }

        figure << (VertexData) {
            QVector3D { pointX, pointY, pointZ},
                      QVector3D(cl.redF(), cl.greenF(), cl.blueF())
        };
    }

    initStaticElements();

    initPreviewSettings();
}


QVector<VertexData> GLWidget::addPointVector(const QVector<QVector3D> &p, const QColor &col)
{
    QVector<VertexData> v;

    for (int i = 0; i < p.count(); i++) {
        v << (VertexData) {
            p.at(i), QVector3D(col.redF(), col.greenF(), col.blueF())
        };
    }

    return v;
}

/**
 * @brief send new data about angles to main widget for displaying on buttons
 *
 */
void GLWidget::initPreviewSettings()
{
    //     emit rotationChanged();
    displayRotation();

    beginViewAnimation();
    //     emit yRotationChanged(Settings::PosAngleY);
    //     emit zRotationChanged(Settings::PosAngleZ);

    update();
}


void GLWidget::updateProjection()
{
    // Reset projection
    m_projectionMatrix.setToIdentity();

    double asp = (double)width() / height();
    m_projectionMatrix.frustum((-0.5 + m_xPan) * asp, (0.5 + m_xPan) * asp, -0.5 + m_yPan, 0.5 + m_yPan, 2, m_distance * 2);
}


void GLWidget::updateView()
{
    // Set view matrix
    m_viewMatrix.setToIdentity();

    double r = m_distance;
    double angY = M_PI / 180 * m_yRot;
    double angX = M_PI / 180 * m_xRot;

    QVector3D eye(r * cos(angX) * sin(angY) + m_xLookAt, r * sin(angX) + m_yLookAt, r * cos(angX) * cos(angY) + m_zLookAt);
    QVector3D center(m_xLookAt, m_yLookAt, m_zLookAt);
    QVector3D up(fabs(m_xRot) == 90 ? -sin(angY + (m_xRot < 0 ? M_PI : 0)) : 0, cos(angX), fabs(m_xRot) == 90 ? -cos(angY + (m_xRot < 0 ? M_PI : 0)) : 0);

    m_viewMatrix.lookAt(eye, center, up.normalized());

    m_viewMatrix.translate(m_xLookAt, m_yLookAt, m_zLookAt);
    m_viewMatrix.scale(m_zoom, m_zoom, m_zoom);
    m_viewMatrix.translate(-m_xLookAt, -m_yLookAt, -m_zLookAt);

    m_viewMatrix.rotate(-90, 1.0, 0.0, 0.0);
}


/**
 * @brief init of 3d viewing
 *
 */
void GLWidget::initializeGL()//Init3D()//*OK*
{
    // OpenGLES2
    initializeOpenGLFunctions();

    glClearColor(Settings::colorSettings[COLOR_BGROUND].redF(), Settings::colorSettings[COLOR_BGROUND].greenF(), Settings::colorSettings[COLOR_BGROUND].blueF(), 1.0f);

    // Use QBasicTimer because its faster than QTimer
    if(Settings::smoothMoving) {
        timer.start(200, this);
    } else {
        timer.start(75, this);
    }

    m_shaderProgram = new QOpenGLShaderProgram(this);

    if (m_shaderProgram) {
        // Compile vertex shader
        m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader.glsl");
        // Compile fragment shader
        m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl");
        // Link shader pipeline
        m_shaderProgram->link();
        qDebug() << "shader program created";
    }

    //     m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    //     m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);

    //     m_shaderProgram->link();



    // Disable 2D Textures
    glDisable( GL_TEXTURE_2D );

    //     glShadeModel( GL_FLAT );

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    glDepthFunc( GL_LESS );

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    //     glEnable(GL_PROGRAM_POINT_SIZE);


    createButtons();
}


/**
 * @brief redraw the scene
 *
 */
void GLWidget::paintGL()
{
    Draw();
    fps++;
}


void GLWidget::beginViewAnimation()
{
    m_xRotStored = m_xRot;
    m_yRotStored = m_yRot;
    m_animationFrame = 0;
    m_animateView = true;
}


void GLWidget::stopViewAnimation()
{
    m_animateView = false;
}


float GLWidget::calculateVolume(QVector3D size)
{
    return size.x() * size.y() * size.z();
}


/**
 * @brief when angle over 360 or under 0, normalize it
 *
 */
float GLWidget::normalizeAngle(float angle)
{
    while (angle < 0) {
        angle += 360;
    }

    while (angle > 360) {
        angle -= 360;
    }

    return angle;
}

/**
 * @brief resize the scene
 *
 */
void GLWidget::resizeGL(int w, int h)
{
    cmdIsometric->move(w - (cmdIsometric->width() + 8), 8);
    cmdTop->move(cmdIsometric->geometry().left() - (cmdTop->width() + 8), 8);
    cmdLeft->move(w - (cmdLeft->width() + 8), cmdIsometric->geometry().bottom() + 8);
    cmdFront->move(cmdLeft->geometry().left() - (cmdFront->width() + 8), cmdIsometric->geometry().bottom() + 8);
    cmdFit->move(w - (cmdFit->width() + 8), cmdLeft->geometry().bottom() + 8);

    cmdZoom->resize(24, this->height() - 120);
    cmdZoom->move(w - (cmdZoom->width() + 8), cmdFit->geometry().bottom() + 8);

    int offs = 0;

    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            cmdX[i]->move(24, 8);
        } else {
            offs = 8 + cmdX[i - 1]->geometry().left() + cmdX[i - 1]->width();
            cmdX[i]->move(offs, 8);
        }
    }

    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            cmdY[i]->move(24 + cmdX[2]->geometry().left() + cmdX[2]->width(), 8);
        } else {
            offs = 8 + cmdY[i - 1]->geometry().left() + cmdY[i - 1]->width();
            cmdY[i]->move(offs, 8);
        }
    }

    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            cmdZ[i]->move(24 + cmdY[2]->geometry().left() + cmdY[2]->width(), 8);
        } else {
            offs = 8 + cmdZ[i - 1]->geometry().left() + cmdZ[i - 1]->width();
            cmdZ[i]->move(offs, 8);
        }
    }

#if 0
    int left = 0, top = 0;
    int width = w, height = h;
    float scale = ((float)w / (float)h);

    if (w > h * 4 / 3) {// wenn screen width over 4 / 3
        width = h * 4 * scale / 3;
        height = h * scale;
        left = (w - width) / (2 * scale);
    } else { // wenn screen width under 4 / 3
        height = w * 3 / (4 * scale);
        top = (h - height) / (2 * scale);
    }

    glViewport(left, top, width, height); // set the current size of view
#endif
}


void GLWidget::timerEvent(QTimerEvent *)
{
    // Request an update
    update();
}


#define GLSCALE 0.05


/**
 * @brief
 *
 */
void GLWidget::Draw() // drawing, main function
{
    if (!m_shaderProgram) {
        return;
    }

    m_shaderProgram->bind();

    m_posAttr = m_shaderProgram->attributeLocation("a_position");
    //     m_startAttr = m_shaderProgram->attributeLocation("a_start");
    m_colAttr = m_shaderProgram->attributeLocation("a_color");
    m_matrixUniform = m_shaderProgram->uniformLocation("mvp_matrix"); // matrix
    m_mvUniform = m_shaderProgram->uniformLocation("mv_matrix");
    m_pointSizeUniform = m_shaderProgram->uniformLocation("point_size");
    //     m_idx = m_shaderProgram->uniformLocation("idx");

    //     // Update geometries in current opengl context
    //     foreach (ShaderDrawable *drawable, m_shaderDrawables) {
    //         if (drawable->needsUpdateGeometry()) drawable->updateGeometry(m_shaderProgram);
    //     }
    //
    //     // Draw geometries
    //     foreach (ShaderDrawable *drawable, m_shaderDrawables) {
    //         drawable->draw(m_shaderProgram);
    //         if (drawable->visible()) vertices += drawable->getVertexCount();
    //     }

    m_shaderProgram->release();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int w = this->width();
    int h = this->height();

    qreal aspect = qreal(w) / qreal(h ? h : 1);

    QMatrix4x4 matrix;

    matrix.setToIdentity();
    matrix.perspective(60.0f, aspect, 10.f, 250.0f);
    matrix.translate(Settings::PosX, Settings::PosY, -100.0);

    float scaleX = Settings::PosZoom * GLSCALE;
    float scaleY = Settings::PosZoom * GLSCALE;
    float scaleZ = Settings::PosZoom * GLSCALE;

    matrix.scale(scaleX, scaleY, scaleZ);

    matrix.rotate(Settings::PosAngleX, 1.0f, 0.0f, 0.0f);
    matrix.rotate(Settings::PosAngleY, 0.0f, 1.0f, 0.0f);
    matrix.rotate(Settings::PosAngleZ, 0.0f, 0.0f, 1.0f);

    m_shaderProgram->setUniformValue(m_matrixUniform, matrix);

    if (figure.count() > 2) {
        // draw figure
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &figure[0].coord);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &figure[0].color);


        glEnableVertexAttribArray(m_posAttr);
        glEnableVertexAttribArray(m_colAttr);

        glDrawArrays(GL_LINE_STRIP, 0, figure.count());

        glDisableVertexAttribArray(m_colAttr);
        glDisableVertexAttribArray(m_posAttr);

        int numSelectStart = -1;
        int numSelectStop = -1;

        switch (parent->getStatus()) {
            case Task::Waiting: {
                numSelectStart = Task::lineCodeStart;

                if (numSelectStart < 0) {
                    break;
                }

                numSelectStop = Task::lineCodeEnd;

                break;
            }

            case Task::Stop:  {
                if (Task::lineCodeStart < 0) {
                    break;
                }

                numSelectStart = Task::lineCodeStart;
                numSelectStop = Task::lineCodeStart;

                break;
            }

            case Task::Paused: {
                int numSelect = cnc->numberCompleatedInstructions() - 1;

                if (numSelect < 0 ) {
                    break;
                }

                numSelectStart = numSelect;
                numSelectStop = numSelect + 1;

                break;
            }

            case Task::Working: {
                int numSelect = cnc->numberCompleatedInstructions() - 1;

                if (numSelect >= 0 && numSelect < figure.count()) {
                    numSelectStart = numSelect;
                    numSelectStop = numSelect + 1;
                }

                break;
            }

            default: {
                break;
            }
        }

        if (numSelectStart != -1 && numSelectStop != -1) {
            // select with 3.0 the current cut of object
            glLineWidth((GLfloat)Settings::lineWidth);

            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &figure[numSelectStart].coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &figure[numSelectStart].color);


            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_LINE_STRIP, 0, (numSelectStop - numSelectStart) + 1);

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);

            glLineWidth((GLfloat)1.0);
        }

        if(Settings::ShowMessure) {
            if (messure.count() > 0) {
                glLineWidth((GLfloat)1.0);

                glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &messure[0].coord);
                glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &messure[0].color);

                glEnableVertexAttribArray(m_posAttr);
                glEnableVertexAttribArray(m_colAttr);

                glDrawArrays(GL_LINES, 0, messure.count());

                glDisableVertexAttribArray(m_colAttr);
                glDisableVertexAttribArray(m_posAttr);

                // text output: X, Y, Z
                //                 if (mText.count() > 0) {
                for (int i = 0; i < mText.count(); i++) {
                    QVector<VertexData> v = mText.at(i);
                    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &v.at(0).coord);
                    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &v.at(0).color);

                    glEnableVertexAttribArray(m_posAttr);
                    glEnableVertexAttribArray(m_colAttr);

                    glDrawArrays(GL_LINE_LOOP, 0, v.count());

                    glDisableVertexAttribArray(m_colAttr);
                    glDisableVertexAttribArray(m_posAttr);
                }

                //                 }
            }
        }
    }

    if (Settings::ShowAxes) {
        glLineWidth((GLfloat)Settings::lineWidth);

        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &axis[0].coord);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &axis[0].color);

        glEnableVertexAttribArray(m_posAttr);
        glEnableVertexAttribArray(m_colAttr);

        glDrawArrays(GL_LINES, 0, axis.count());

        glDisableVertexAttribArray(m_colAttr);
        glDisableVertexAttribArray(m_posAttr);

        glLineWidth((GLfloat)1.0);

        // text output: X, Y, Z
        //         if (aText.count() >= 0) {
        for (int i = 0; i < aText.count(); i++) {
            QVector<VertexData> v = aText.at(i);
            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &v.at(0).coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &v.at(0).color);

            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_LINE_LOOP, 0, v.count());

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);
        }

        //         }
    }


    if (Settings::ShowBorder) {
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &border[0].coord);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &border[0].color);

        glEnableVertexAttribArray(m_posAttr);
        glEnableVertexAttribArray(m_colAttr);

        glDrawArrays(GL_LINE_STRIP, 0, border.count());

        glDisableVertexAttribArray(m_colAttr);
        glDisableVertexAttribArray(m_posAttr);
    }

    //  the scanned surface
    if (Settings::ShowSurface) {
        if (Settings::ShowLines) {
            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &surfaceLines[0].coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &surfaceLines[0].color);

            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_LINES, 0, surfaceLines.count());

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);
        }

        if (Settings::ShowPoints) {
            m_shaderProgram->setUniformValue(m_pointSizeUniform, (GLfloat)Settings::pointSize);

            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &surfacePoints[0].coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &surfacePoints[0].color);

            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_POINTS, 0, surfacePoints.count());

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);
        }
    }

    // draw the border
    if (Settings::ShowGrid) {
        if (Settings::ShowLines) {
            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &gridLines[0].coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &gridLines[0].color);

            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_LINES, 0, gridLines.count());

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);
        }

        if (Settings::ShowPoints) {
            m_shaderProgram->setUniformValue(m_pointSizeUniform, (GLfloat)Settings::pointSize);

            glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &gridPoints[0].coord);
            glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &gridPoints[0].color);

            glEnableVertexAttribArray(m_posAttr);
            glEnableVertexAttribArray(m_colAttr);

            glDrawArrays(GL_POINTS, 0, gridPoints.count());

            glDisableVertexAttribArray(m_colAttr);
            glDisableVertexAttribArray(m_posAttr);
        }
    }


    if (Settings::ShowInstrument) {
        matrix.translate(Settings::coord[X].posMm(), Settings::coord[Y].posMm(), Settings::coord[Z].posMm()); // moving to point x=10, y=10

        m_shaderProgram->setUniformValue(m_matrixUniform, matrix);

        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &instrument[0].coord);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &instrument[0].color);

        glEnableVertexAttribArray(m_posAttr);
        glEnableVertexAttribArray(m_colAttr);

        glDrawArrays(GL_TRIANGLES, 0, instrument.count());

        glDisableVertexAttribArray(m_colAttr);
        glDisableVertexAttribArray(m_posAttr);
    }

#if 0

    QPainter painter(this);

    // Segment counter
    int vertices = 0;

    painter.beginNativePainting();
    // Draw 2D
    //     glDisable(GL_DEPTH_TEST);
    //     glDisable(GL_MULTISAMPLE);
    //     glDisable(GL_LINE_SMOOTH);
    //     glDisable(GL_BLEND);



    QColor m_colorText = Qt::yellow;
    QPen pen(m_colorText);
    painter.setPen(pen);

    float x = 10;
    float y = this->height() - 60;

    //     painter.drawText(QPoint(x, y), QString("X: %1 ... %2").arg(m_xMin, 0, 'f', 3).arg(m_xMax, 0, 'f', 3));
    //     painter.drawText(QPoint(x, y + 15), QString("Y: %1 ... %2").arg(m_yMin, 0, 'f', 3).arg(m_yMax, 0, 'f', 3));
    //     painter.drawText(QPoint(x, y + 30), QString("Z: %1 ... %2").arg(m_zMin, 0, 'f', 3).arg(m_zMax, 0, 'f', 3));
    //     painter.drawText(QPoint(x, y + 45), QString("%1 / %2 / %3").arg(m_xSize, 0, 'f', 3).arg(m_ySize, 0, 'f', 3).arg(m_zSize, 0, 'f', 3));

    QFontMetrics fm(painter.font());

    //     painter.drawText(QPoint(x, fm.height() + 10), m_parserStatus);

    QString str = QString(tr("Vertices: %1")).arg(vertices);
    painter.drawText(QPoint(this->width() - fm.width(str) - 10, y + 30), str);
    str = QString("FPS: %1").arg(fps);
    painter.drawText(QPoint(this->width() - fm.width(str) - 10, y + 45), str);

    //     str = m_spendTime.toString("hh:mm:ss") + " / " + m_estimatedTime.toString("hh:mm:ss");
    //     painter.drawText(QPoint(this->width() - fm.width(str) - 10, y), str);

    //     str = m_bufferState;
    //     painter.drawText(QPoint(this->width() - fm.width(str) - 10, y + 15), str);

    painter.endNativePainting();
#endif
}


/**
 * @brief press event from mouse
 *
 */
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();

    event->setAccepted(true);
}


/**
 * @brief press event from mouse
 *
 */
void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //     m_lastPos = event->pos();

    event->setAccepted(true);
}


/**
 * @brief wheel event from mouse
 *
 */
void GLWidget::wheelEvent(QWheelEvent *we)
{
    if (we->orientation() == Qt::Vertical) {
        we->delta() > 0 ? Settings::PosZoom += 1.0 : Settings::PosZoom -= 1.0;

        cmdZoom->setValue(Settings::PosZoom);
    }

    we->setAccepted(true);
}

/**
 * @brief move event from mouse druring button pressed
 *
 */
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->pos().x() - m_lastPos.x();
    int dy = m_lastPos.y() - event->pos().y();

    if (event->buttons() & Qt::LeftButton) {
        setXCoord(dx);
        setYCoord(dy);

        if (Settings::smoothMoving) {
            update();
        }
    } else if (event->buttons() & Qt::RightButton) {
        switch (parent->fixedAxes) {
            case FixY: {
                setXRotation(Settings::PosAngleX + dy);
                setZRotation(Settings::PosAngleZ + dx);
                break;
            }

            case FixX: {
                setYRotation(Settings::PosAngleY + dx);
                setZRotation(Settings::PosAngleZ + dy);
                break;
            }

            case FixZ: {
                setXRotation(Settings::PosAngleX + dy);
                setYRotation(Settings::PosAngleY + dx);
                break;
            }
        }

        if (Settings::smoothMoving) {
            update();
        }
    }

    m_lastPos = event->pos();

    //     event->setAccepted(true);
}


/**
 * @brief when angle over 360 or under 0, normalize it
 *
 */
// float GLWidget::normalizeAngle(float angle)
// {
//     while (angle < 0) {
//         angle += 360;
//     }
//
//     while (angle > 360) {
//         angle -= 360;
//     }
//
//     return angle;
// }


/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::setXCoord(int dx)
{
    if (dx != 0) {
        Settings::PosX += dx * 0.5;
    }
}


/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::setYCoord(int dy)
{
    if (dy != 0) {
        Settings::PosY += dy * 0.5;
    }
}


/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::setXRotation(int angle)
{
    angle = normalizeAngle(angle);

    if (angle != Settings::PosAngleX) {
        Settings::PosAngleX = angle;
        displayRotation();
        //         emit rotationChanged();
    }
}

/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::setYRotation(int angle)
{
    angle = normalizeAngle(angle);

    if (angle != Settings::PosAngleY) {
        Settings::PosAngleY = angle;
        displayRotation();
        //         emit rotationChanged();
    }
}


/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::setZRotation(int angle)
{
    angle = normalizeAngle(angle);

    if (angle != Settings::PosAngleZ) {
        Settings::PosAngleZ = angle;
        displayRotation();
        //         emit rotationChanged();
    }
}


/**
 * @brief slot for button signal from mainwindow, x minus
 *
 */
void GLWidget::onPosAngleXm()
{
    --Settings::PosAngleX;
    Settings::PosAngleX = normalizeAngle(Settings::PosAngleX);
    displayRotation();
    //     emit rotationChanged();
}


/**
 * @brief slot for button signal from mainwindow. reset x
 *
 */
void GLWidget::onPosAngleX()
{
    Settings::PosAngleX = 0;
    displayRotation();
    //     emit rotationChanged();
}


/**
 * @brief slot for button signal from mainwindow, x plus
 *
 */
void GLWidget::onPosAngleXp()
{
    ++Settings::PosAngleX;
    Settings::PosAngleX = normalizeAngle(Settings::PosAngleX);
    displayRotation();
    //     emit rotationChanged();
}


/**
 * @brief slot for button signal from mainwindow, y plus
 *
 */
void GLWidget::onPosAngleYp()
{
    ++Settings::PosAngleY;
    Settings::PosAngleY = normalizeAngle(Settings::PosAngleY);
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow, reset y
 *
 */
void GLWidget::onPosAngleY()
{
    Settings::PosAngleY = 0;
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow, y minus
 *
 */
void GLWidget::onPosAngleYm()
{
    --Settings::PosAngleY;
    Settings::PosAngleY = normalizeAngle(Settings::PosAngleY);
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow, z plus
 *
 */
void GLWidget::onPosAngleZp()
{
    ++Settings::PosAngleZ;
    Settings::PosAngleZ = normalizeAngle(Settings::PosAngleZ);
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow, reset z
 *
 */
void GLWidget::onPosAngleZ()
{
    Settings::PosAngleZ = 0;
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow, z minus
 *
 */
void GLWidget::onPosAngleZm()
{
    --Settings::PosAngleZ;
    Settings::PosAngleZ = normalizeAngle(Settings::PosAngleZ);
    displayRotation();
    //     emit rotationChanged();
}

/**
 * @brief slot for button signal from mainwindow
 *
 */
void GLWidget::onDefaulPreview()
{
    initPreviewSettings();
}

