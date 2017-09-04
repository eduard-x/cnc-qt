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

#include "includes/version.h"

#include "includes/GLWidget.h"

#include <QDebug>

#include <QtOpenGL/QtOpenGL>

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QToolButton>

// for GLES2 are QGLFunctions to implement

#include "includes/Settings.h"
#include "includes/MainWindow.h"
#include "includes/mk1Controller.h"


#define ZOOMSTEP 1.1


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

    m_zoom = 1;

    parent = (MainWindow*)p;

    cnc = parent->mk1;

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
    //         connect(scene3d, SIGNAL(scaleChanged(int)), this, SLOT(getScale(int)));

    //         connect(pushDefaultPreview, SIGNAL(clicked()), this, SLOT(onDefaulPreview()));

    //     cmdIsometric->setToolTip(translate(_ISO));//"Iso");
}


QVector<QVector3D> GLWidget::xAxis = {
    { 0.0, 0.0, 0.0 },
    { 10.0, 0.0, 0.0 },
    { 10.0, 0.0, 0.0 },
    { 9.0, 1.0, 0.0 },
    { 10.0, 0.0, 0.0 },
    { 9.0, -1.0, 0.0 }
};

QVector<QVector3D> GLWidget:: yAxis = {
    { 0.0, 0.0, 0.0 },
    { 0.0, 10.0, 0.0 },
    { 0.0, 10.0, 0.0 },
    { 1.0, 9.0, 0.0 },
    { 0.0, 10.0, 0.0 },
    { -1.0, 9.0, 0.0 }
};

QVector<QVector3D> GLWidget::zAxis = {
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 10.0 },
    { 0.0, 0.0, 10.0 },
    { 1.0, 1.0, 9.0 },
    { 0.0, 0.0, 10.0 },
    { -1.0, -1.0, 9.0 }
};

QVector<QVector3D> GLWidget::instrumentArray = {
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 4.0 },
    { -1.0, -1.0, 2.0 },
    { -1.0, 1.0, 2.0 },
    { 1.0, -1.0, 2.0 },
    { 1.0, 1.0, 2.0 },
    { 1.0, 1.0, 2.0 },
    { -1.0, 1.0, 2.0 },
    { 1.0, -1.0, 2.0 },
    { -1.0, -1.0, 2.0 },
    { -1.0, -1.0, 2.0 },
    { 0.0, 0.0, 0.0 },
    { 1.0, 1.0, 2.0 },
    { 0.0, 0.0, 0.0 },
    { 1.0, -1.0, 2.0 },
    { 0.0, 0.0, 0.0 },
    { -1.0, 1.0, 2.0 },
    { 0.0, 0.0, 0.0 }
};


QVector<QVector3D> GLWidget::footArray = { // GL_LINE_LOOP array
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

QVector<QVector3D> GLWidget::traverseArray = { // width of traverse is 64 cm
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
#if 0
    int i = 0;

    // color correcture in the main object
    foreach (const GCodeData vv, parent->gCodeData) {
        QColor cl;

        if (vv.movingCode == RAPID_LINE_CODE) {
            cl = Settings::colorSettings[COLOR_RAPID];
        } else {
            cl = Settings::colorSettings[COLOR_WORK];
        }

#if 0
        // coordinates of next point
        float pointX = vv.X;
        float pointY = vv.Y;
        float pointZ = vv.Z;

        // moving in G-code
        if (Settings::Correction) {
            // proportions
            pointX *= Settings::coeffSizeX;
            pointY *= Settings::coeffSizeY;

            // offset
            pointX += Settings::deltaX;
            pointY += Settings::deltaY;
            pointZ += Settings::deltaZ;

            // to use the scanned surface, z correcture
            if (Settings::deltaFeed) {
                pointZ += Settings::getDeltaZ(pointX, pointY);
            }
        }

#endif
        figure[i].color = QVector3D (cl.redF(), cl.greenF(), cl.blueF()); //QVector3D {cl.r, cl.g, cl.b};
        i++;
        //         vv << (VertexData) {
        //             QVector3D { pointX, pointY, pointZ},
        //                         QVector3D {cl.r, cl.g, cl.b}
        //         };

    }

#endif

    axis.clear();
    axis << addPointVector(xAxis, Settings::colorSettings[COLOR_X]);
    axis << addPointVector(yAxis, Settings::colorSettings[COLOR_Y]);
    axis << addPointVector(zAxis, Settings::colorSettings[COLOR_Z]);

    instrument.clear();
    instrument << addPointVector(instrumentArray, Settings::colorSettings[COLOR_TOOL]);

    border.clear();
    border << (VertexData) {
        QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0},
                   QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF())
    };
    border << (VertexData) {
        QVector3D{ Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin, 0},
                   QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF())
    };
    border << (VertexData) {
        QVector3D{ Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMax, 0},
                   QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF())
    };
    border << (VertexData) {
        QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMax, 0},
                   QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF())
    };
    border << (VertexData) {
        QVector3D{ Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0},
                   QVector3D(Settings::colorSettings[COLOR_BORDER].redF(), Settings::colorSettings[COLOR_BORDER].greenF(), Settings::colorSettings[COLOR_BORDER].blueF())
    };


    gridLines.clear();

    for (int gX = Settings::GridXstart; gX < Settings::GridXend + 1; gX += Settings::GrigStep) {
        gridLines << (VertexData) {
            QVector3D{(GLfloat)gX, (GLfloat)Settings::GridYstart, 0.0 },
                      QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF())
        };
        gridLines << (VertexData) {
            QVector3D{(GLfloat)gX, (GLfloat)Settings::GridYend, 0.0},
                      QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF())
        };
    }

    for (int gY = Settings::GridYstart; gY < Settings::GridYend + 1; gY += Settings::GrigStep) {
        gridLines << (VertexData) {
            QVector3D{(GLfloat)Settings::GridXstart, (GLfloat)gY, 0.0},
                      QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF())
        };
        gridLines << (VertexData) {
            QVector3D{(GLfloat)Settings::GridXend, (GLfloat)gY, 0.0},
                      QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF())
        };
    }

    gridPoints.clear();

    for (int y = Settings::GridYstart; y <  Settings::GridYend + 1; y += Settings::GrigStep) {
        for (int x = Settings::GridXstart; x < Settings::GridXend + 1; x += Settings::GrigStep) {
            //point
            gridPoints << (VertexData) {
                QVector3D{(GLfloat)x, (GLfloat)y, 0.0},
                          QVector3D(Settings::colorSettings[COLOR_GRID].redF(), Settings::colorSettings[COLOR_GRID].greenF(), Settings::colorSettings[COLOR_GRID].blueF())
            };
        }
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

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            surfaceLines << (VertexData) {
                QVector3D{parent->surfaceMatrix[y][x].pos[X], parent->surfaceMatrix[y][x].pos[Y], parent->surfaceMatrix[y][x].pos[Z]},
                          QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
            };

            if (y > 0) {
                //line 1
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y - 1][x].pos[X], parent->surfaceMatrix[y - 1][x].pos[Y], parent->surfaceMatrix[y - 1][x].pos[Z]},
                              QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
                };
            }

            if (y < maxY - 1) {
                //line2
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y + 1][x].pos[X], parent->surfaceMatrix[y + 1][x].pos[Y], parent->surfaceMatrix[y + 1][x].pos[Z]},
                              QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
                };
            }

            if (x > 0) {
                //line 3
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y][x - 1].pos[X], parent->surfaceMatrix[y][x - 1].pos[Y], parent->surfaceMatrix[y][x - 1].pos[Z]},
                              QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
                };
            }

            if (x < maxX - 1) {
                //line4
                surfaceLines << (VertexData) {
                    QVector3D{parent->surfaceMatrix[y][x + 1].pos[X], parent->surfaceMatrix[y][x + 1].pos[Y], parent->surfaceMatrix[y][x + 1].pos[Z]},
                              QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
                };
            }

            // now points
            surfacePoints << (VertexData) {
                QVector3D{parent->surfaceMatrix[y][x].pos[X], parent->surfaceMatrix[y][x].pos[Y], parent->surfaceMatrix[y][x].pos[Z]},
                          QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
            };
        }
    }


#if 0

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            //point
            surfacePoints << (VertexData) {
                QVector3D{parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z},
                          QVector3D(Settings::colorSettings[COLOR_SURFACE].redF(), Settings::colorSettings[COLOR_SURFACE].greenF(), Settings::colorSettings[COLOR_SURFACE].blueF())
            };
        }
    }

#endif
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
            QVector3D point = parent->gCodeData.at(i).xyz;
            //             float pointY = parent->gCodeData.at(i).Y;
            //             float pointZ = parent->gCodeData.at(i).Z;

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

    if (parent->gCodeData.count() > 1) {
        foreach (const GCodeData vv, parent->gCodeData) {
            QColor cl;

            if (vv.movingCode == RAPID_LINE_CODE) {
                cl = Settings::colorSettings[COLOR_RAPID];
            } else {
                cl = Settings::colorSettings[COLOR_WORK];
            }

            // coordinates of next point
            float pointX = vv.xyz.x();
            float pointY = vv.xyz.y();
            float pointZ = vv.xyz.z();

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
    //     emit yRotationChanged(Settings::PosAngleY);
    //     emit zRotationChanged(Settings::PosAngleZ);

    update();
}


/**
 * @brief init of 3d viewing
 *
 */
void GLWidget::initializeGL()//Init3D()//*OK*
{
    // OpenGLES2
    initializeOpenGLFunctions();

    glClearColor(Settings::colorSettings[COLOR_BACKGROUND].redF(), Settings::colorSettings[COLOR_BACKGROUND].greenF(), Settings::colorSettings[COLOR_BACKGROUND].blueF(), 1.0f);

    // Use QBasicTimer because its faster than QTimer
    timer.start(100, this);

    const char *vsrc =
        "#ifdef GL_ES\n"
        "// Set default precision to medium\n"
        "precision mediump int;\n"
        "precision mediump float;\n"
        "#endif\n"
        "\n"
        "uniform mat4 mvp_matrix;\n"
        "uniform mat4 mv_matrix;\n"
        "uniform float point_size;\n"
        "\n"
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        //         "attribute vec4 a_start;\n"
        "\n"
        "varying vec4 v_color;\n"
        "varying vec2 v_position;\n"
        //         "varying vec2 v_start;\n"
        //         "\n"
        //         "bool isNan(float val)\n"
        //         "{\n"
        //         "    return (val > 65535.0);\n"
        //         "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // Calculate interpolated vertex position & line start point\n"
        "    v_position = (mv_matrix * a_position).xy;\n"
        "\n"
        //         "    if (!isNan(a_start.x)) v_start = (mv_matrix * a_start).xy;\n"
        //         "    else v_start = a_start.xy;\n"
        //         "\n"
        "    // Calculate vertex position in screen space\n"
        "    gl_Position = mvp_matrix * a_position;\n"
        "\n"
        "    // Value will be automatically interpolated to fragments inside polygon faces\n"
        "    v_color = a_color;\n"
        "\n"
        "    // Set point size\n"
        "    gl_PointSize = point_size;\n"
        "}\n"
        "\n";

    const char *fsrc =
        "#ifdef GL_ES\n"
        "// Set default precision to medium\n"
        "precision mediump int;\n"
        "precision mediump float;\n"
        "#endif\n"
        "\n"
        "//Dash grid (px) = factor * pi;\n"
        "const float factor = 2.0;\n"
        "\n"
        "uniform float point_size;\n"
        "\n"
        "varying vec4 v_color;\n"
        "varying vec2 v_position;\n"
        //         "varying vec2 v_start;\n"
        //         "\n"
        //         "bool isNan(float val)\n"
        //         "{\n"
        //         "    return (val > 65535.0);\n"
        //         "}\n"
        "\n"
        "void main()\n"
        "{\n"
        //         "    // Draw dash lines\n"
        //         "    if (!isNan(v_start.x)) {\n"
        //         "        vec2 sub = v_position - v_start;\n"
        //         "        float coord = length(sub.x) > length(sub.y) ? gl_FragCoord.x : gl_FragCoord.y;\n"
        //         "        if (cos(coord / factor) > 0.0) discard;\n"
        //         "    }\n"
        "#ifdef GL_ES\n"
        "    if (point_size > 0.0) {\n"
        "        vec2 coord = gl_PointCoord.st - vec2(0.5, 0.5);\n"
        "        if (length(coord) > 0.5) discard;\n"
        "    }\n"
        "#endif\n"
        "\n"
        "    // Set fragment color from texture\n"
        "    gl_FragColor = v_color;\n"
        "}\n";

    program = new QOpenGLShaderProgram(this);

    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);

    program->link();


    m_posAttr = program->attributeLocation("a_position");
    //     m_startAttr = program->attributeLocation("a_start");
    m_colAttr = program->attributeLocation("a_color");
    m_matrixUniform = program->uniformLocation("mvp_matrix"); // matrix
    m_mvUniform = program->uniformLocation("mv_matrix");
    m_pointSizeUniform = program->uniformLocation("point_size");
    //     m_idx = program->uniformLocation("idx");

    program->bind();


    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

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
    if (!program) {
        return;
    }

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

    program->setUniformValue(m_matrixUniform, matrix);

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
            program->setUniformValue(m_pointSizeUniform, (GLfloat)Settings::pointSize);

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
            program->setUniformValue(m_pointSizeUniform, (GLfloat)Settings::pointSize);

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

        program->setUniformValue(m_matrixUniform, matrix);

        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &instrument[0].coord);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &instrument[0].color);

        glEnableVertexAttribArray(m_posAttr);
        glEnableVertexAttribArray(m_colAttr);

        glDrawArrays(GL_LINES, 0, instrument.count());

        glDisableVertexAttribArray(m_colAttr);
        glDisableVertexAttribArray(m_posAttr);
    }
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
    normalizeAngle(angle);

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
    normalizeAngle(angle);

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
    normalizeAngle(angle);

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
    normalizeAngle(Settings::PosAngleX);
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
    normalizeAngle(Settings::PosAngleX);
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
    normalizeAngle(Settings::PosAngleY);
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
    normalizeAngle(Settings::PosAngleY);
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
    normalizeAngle(Settings::PosAngleZ);
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
    normalizeAngle(Settings::PosAngleZ);
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

