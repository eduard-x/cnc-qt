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

#include <QtGui/QImage>
#include <QtOpenGL>
#include <QGLFormat>
#include <QGLWidget>

#include <deque>
#include <utility>
#include "includes/vec.h"

#include "includes/MainWindow.h"
#include "includes/mk1Controller.h"
#include "includes/GLWidget.h"

#include <math.h>



GLWidget::GLWidget(QWidget *p)
    : QGLWidget(p)
{
    if (p == NULL) {
        return;
    }

    coordArray.clear();
    colorArray.clear();

    parent = (MainWindow*)p;

    cnc = parent->cnc;

    initStaticElements();

    initializeGL();

    fps = 0;

    QTimer* fpsTimer = new QTimer();
    QObject::connect(fpsTimer, SIGNAL(timeout()), this, SLOT(showFPS()));
    fpsTimer->start(1000);

    //     glTimer = new QTimer();
    //     QObject::connect(glTimer, SIGNAL(timeout()), this, SLOT(processing()));
    //     glTimer->start( 20 );
    //

    QGLFormat::OpenGLVersionFlags f = QGLFormat::openGLVersionFlags();
    qDebug() << "GL enabled" << QGLFormat::hasOpenGL() << "flags:" << f;
    QString glStr = QString().sprintf("%s %s %s", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));

    qDebug() << glStr;
    initPreviewSettings();

    paintGL();
}


GLWidget::~GLWidget()
{
}


void GLWidget::initStaticElements()
{
    xAxis = (QVector<pointGL>()
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        10.0, 0.0, 0.0
    }
    << (pointGL) {
        10.0, 0.0, 0.0
    }
    << (pointGL) {
        9.0, 1.0, 0.0
    }
    << (pointGL) {
        10.0, 0.0, 0.0
    }
    << (pointGL) {
        9.0, -1.0, 0.0
    });

    yAxis = (QVector<pointGL>()
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        0.0, 10.0, 0.0
    }
    << (pointGL) {
        0.0, 10.0, 0.0
    }
    << (pointGL) {
        1.0, 9.0, 0.0
    }
    << (pointGL) {
        0.0, 10.0, 0.0
    }
    << (pointGL) {
        -1.0, 9.0, 0.0
    });

    zAxis = (QVector<pointGL>()
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        0.0, 0.0, 10.0
    }
    << (pointGL) {
        0.0, 0.0, 10.0
    }
    << (pointGL) {
        1.0, 1.0, 9.0
    }
    << (pointGL) {
        0.0, 0.0, 10.0
    }
    << (pointGL) {
        -1.0, -1.0, 9.0
    });


    instrumentArray = (QVector<pointGL>()
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        0.0, 0.0, 4.0
    }
    << (pointGL) {
        -1.0, -1.0, 2.0
    }
    << (pointGL) {
        -1.0, 1.0, 2.0
    }
    << (pointGL) {
        1.0, -1.0, 2.0
    }
    << (pointGL) {
        1.0, 1.0, 2.0
    }
    << (pointGL) {
        1.0, 1.0, 2.0
    }
    << (pointGL) {
        -1.0, 1.0, 2.0
    }
    << (pointGL) {
        1.0, -1.0, 2.0
    }
    << (pointGL) {
        -1.0, -1.0, 2.0
    }
    << (pointGL) {
        -1.0, -1.0, 2.0
    }
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        1.0, 1.0, 2.0
    }
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        1.0, -1.0, 2.0
    }
    << (pointGL) {
        0.0, 0.0, 0.0
    }
    << (pointGL) {
        -1.0, 1.0, 2.0
    }
    << (pointGL) {
        0.0, 0.0, 0.0
    });
}


void GLWidget::surfaceReloaded()
{
    for (int i = 0; i < coordArray.count(); i++) {
        if (parent->deltaFeed) {
            pointGL p;
            float pointX = parent->GCodeList.at(i).X;
            float pointY = parent->GCodeList.at(i).Y;
            float pointZ = parent->GCodeList.at(i).Z;
            pointZ += parent->GetDeltaZ(pointX, pointY);

            p = (pointGL) {
                pointX, pointY, pointZ
            };

            coordArray[i] = p;
        }
    }
}


void GLWidget::showFPS()
{
    emit fpsChanged(fps);

    fps = 0;
}


void GLWidget::processing()
{
    paintGL();
    fps++;
}


void GLWidget::matrixReloaded()
{
    int workNum = 0;

    workNum = parent->GCodeList.count();

    coordArray.clear();
    colorArray.clear();

    if (workNum > 1) {
        int currWorkPoint = 0;

        foreach (GCodeCommand vv, parent->GCodeList) {
            colorGL cl;

            if (vv.workspeed) {
                cl = (colorGL) {
                    0, 255, 0
                };
            } else {
                cl = (colorGL) {
                    255, 0, 0
                };
            }


            pointGL p;
            // coordinates of next point
            float pointX = vv.X;
            float pointY = vv.Y;
            float pointZ = vv.Z;

            // moving in G-code
            if (parent->Correction) {
                // proportions
                pointX *= parent->koeffSizeX;
                pointY *= parent->koeffSizeY;

                // offset
                pointX += parent->deltaX;
                pointY += parent->deltaY;
                pointZ += parent->deltaZ;

                // to use the scanned surface, z correcture
                if (parent->deltaFeed) {
                    pointZ += parent->GetDeltaZ(pointX, pointY);
                }
            }

            p = (pointGL) {
                pointX, pointY, pointZ
            };

            coordArray << p;
            colorArray << cl;

            currWorkPoint++;
        }
    }

    emit xRotationChanged(parent->PosAngleX);
    emit yRotationChanged(parent->PosAngleY);
    emit zRotationChanged(parent->PosAngleZ);

    initializeGL();
}


void GLWidget::initPreviewSettings()
{
    parent->PosX = -96;
    parent->PosY = -64;
    parent->PosZ = -300;
    parent->PosAngleX = 180;
    parent->PosAngleY = 180;
    parent->PosAngleZ = 180;
    parent->PosZoom = 25;

    emit xRotationChanged(parent->PosAngleX);
    emit yRotationChanged(parent->PosAngleY);
    emit zRotationChanged(parent->PosAngleZ);

    updateGL();
}


//
// init of 3d viewing
//
void GLWidget::initializeGL()//Init3D()//*OK*
{
    makeCurrent();
    // activate projection matrix
    glMatrixMode(GL_PROJECTION);

    // clening
    glLoadIdentity();

    glScalef( 1, 1, -1 ); // negative z is top

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
}


void GLWidget::paintGL()
{
    Draw();
    fps++;
}


void GLWidget::resizeGL(int w, int h)
{
    // set the current size of view
//     glViewport(0, 0, width, height);
    int left = 0, top = 0;
    int width = w, height = h;

    if (w > h * 4 / 3) {// Если экран шире необходимого
        width = h * 4 / 3;
        left = (w - width) / 2;
    }
    else { // Наоборот, если экран выше необходимого
        height = w * 3 / 4;
        top = (h - height) / 2;
    }

    glViewport(left, top, width, height); // Устанавливаем область отображения
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}


void GLWidget::wheelEvent(QWheelEvent *e)
{
    e->delta() > 0 ? parent->PosZoom++ : parent->PosZoom--;

    e->setAccepted(true);
    updateGL();
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXCoord(parent->PosX += dx);
        setYCoord(parent->PosY -= dy);
    } else if (event->buttons() & Qt::RightButton) {
        switch (parent->fixedAxes) {
            case FixY: {
                setXRotation(parent->PosAngleX + dy);
                setZRotation(parent->PosAngleZ + dx);
                break;
            }

            case FixX: {
                setYRotation(parent->PosAngleY + dx);
                setZRotation(parent->PosAngleZ + dy);
                break;
            }

            case FixZ: {
                setXRotation(parent->PosAngleX + dy);
                setYRotation(parent->PosAngleY + dx);
                break;
            }
        }
    }

    lastPos = event->pos();

    event->setAccepted(true);
    updateGL();
}


#define GLSCALE 2000.0


void GLWidget::Draw() // drawing, main function
{
    //
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean color buffer and depth buff
    glClearColor(0.45f, 0.45f, 0.45f, 1);                  // set gray color of background

    glLoadIdentity();                                   // clean

    glPushMatrix();                                     // push current matrix

    if (windowState() != Qt::WindowMinimized) {
        int w = width();
        int h = height();

        float n = 1;

        if (h < w) {
            n = (w / h);
        }
        
    /// move eyes point for better view of object
        glTranslated(parent->PosX / GLSCALE, parent->PosY / GLSCALE, parent->PosZ / GLSCALE);

        
        float scaleX = parent->PosZoom / (GLSCALE * n);
        float scaleY = parent->PosZoom / GLSCALE;
        float scaleZ = parent->PosZoom / GLSCALE;

        glScaled(scaleX, scaleY, scaleZ);
    }
    
    
    ///угловое вращение
    glRotated(parent->PosAngleX, 1, 0, 0);
    glRotated(parent->PosAngleY, 0, 1, 0);
    glRotated(parent->PosAngleZ, 0, 0, 1);
    
    

    glEnable(GL_LINE_SMOOTH);

    // axes
    if (parent->ShowAxes) {
        drawAxes();
    }

    // coordinate grid
    if (parent->ShowGrid) {
        drawGrid();
    }

    //  the scanned surface
    if (parent->ShowSurface) {
        drawSurface();
    }

    // draw the tool
    if (parent->ShowInstrument) {
        drawInstrument();
    }

    drawWorkField();

    // draw the border
    if (parent->ShowGrate) {
        drawGrate();
    }

    // end of drawing
    glDisable(GL_LINE_SMOOTH);

    //
    glPopMatrix();
    //
    glFlush();
}


void GLWidget::drawAxes()
{
    glLineWidth(2);

    glDisable(GL_DEPTH_TEST); // because of text rendering

    glEnable(GL_VERTEX_ARRAY);

    glDisable(GL_NORMAL_ARRAY);

    // x
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertexPointer(3, GL_FLOAT, 0, &xAxis[0]);
    glDrawArrays(GL_LINES, 0, xAxis.count()); // draw array of lines
    renderText(12.0, 0.0, 0.0, QString("X")); //coordinates of text

    // y
    glColor3f(1.0F, 0, 0.0F);
    glVertexPointer(3, GL_FLOAT, 0, &yAxis[0]);
    glDrawArrays(GL_LINES, 0, yAxis.count());
    renderText(0.0, 12.0, 0.0, QString("Y")); //coordinates of text

    // z
    glColor3f(0.0F, 1.0, 1.0F);
    glVertexPointer(3, GL_FLOAT, 0, &zAxis[0]);
    glDrawArrays(GL_LINES, 0, zAxis.count());
    renderText(0.0, 0.0, 12.0, QString("Z")); //coordinates of text

    glDisable(GL_VERTEX_ARRAY);

    glEnable(GL_NORMAL_ARRAY);

    glEnable(GL_DEPTH_TEST);
}


void GLWidget::drawWorkField()
{
    if (coordArray.count() < 2) {
        return;
    }

    glPushMatrix();

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_COORD_ARRAY);

    glLineWidth(0.3f);

    // the object
    glVertexPointer(3, GL_FLOAT, 0, &coordArray[0]);
    glColorPointer(3, GL_FLOAT, 0, &colorArray[0]);
    glDrawArrays(GL_LINE_STRIP, 0, coordArray.count());
    //

    // select with 3.0 the current cut of object
    switch (Task::Status) {
        case Waiting: {
            int numSelectStart = Task::posCodeStart;
            int numSelectStop = Task::posCodeEnd;
            glLineWidth(3.0f);
            glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelectStart]);
            glColorPointer(3, GL_FLOAT, 0, &colorArray[numSelectStart]);
            glDrawArrays(GL_LINE_STRIP, 0, numSelectStop - numSelectStart - 1);
            break;
        }

        case Stop:  {
            //         int numSelect = cnc->numberCompleatedInstructions() - 1;
            glLineWidth(3.0f);
            glVertexPointer(3, GL_FLOAT, 0, &coordArray[Task::posCodeStart]);
            glColorPointer(3, GL_FLOAT, 0, &colorArray[Task::posCodeStart]);
            glDrawArrays(GL_LINE_STRIP, 0, 2);
            break;
        }

        case Paused: {
            int numSelect = cnc->numberCompleatedInstructions() - 1;

            if (numSelect >= 0 ) {
                glLineWidth(3.0f);
                glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelect]);
                glColorPointer(3, GL_FLOAT, 0, &colorArray[numSelect]);
                glDrawArrays(GL_LINE_STRIP, 0, 2);
            }

            break;
        }

        case Working: {
            int numSelect = cnc->numberCompleatedInstructions() - 1;

            if (numSelect >= 0 && numSelect < coordArray.count()) {
                glLineWidth(3.0f);
                glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelect]);
                glColorPointer(3, GL_FLOAT, 0, &colorArray[numSelect + 1]);
                glDrawArrays(GL_LINE_STRIP, 0, 2);
            }

            break;
        }
    }

    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_ARRAY);
    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}


void GLWidget::drawGrid()
{
    if (parent->ShowLines) {
        glLineWidth(0.1f);
        glColor3f(0.99, 0.99, 0.99);

        glBegin(GL_LINES);

        for (int gX = parent->GridXstart; gX < parent->GridXend + 1; gX += parent->GrigStep) {
            glVertex3d(gX, parent->GridYstart, 0);
            glVertex3d(gX, parent->GridYend, 0);
        }

        for (int gY = parent->GridYstart; gY < parent->GridYend + 1; gY += parent->GrigStep) {
            glVertex3d(parent->GridXstart, gY, 0);
            glVertex3d(parent->GridXend, gY, 0);
        }

        glEnd();
    }

    if (parent->ShowPoints) {
        glPointSize(1.0f);
        //         glLineWidth(0.3f);
        glColor3f(0.99, 0.99, 0.99); // white

        glBegin(GL_POINTS);


        for (int y = parent->GridYstart; y <  parent->GridYend + 1; y += parent->GrigStep) {
            for (int x = parent->GridXstart; x < parent->GridXend + 1; x += parent->GrigStep) {
                //point
                glVertex3d(x, y, 0);
            }
        }

        glEnd();
    }
}


void GLWidget::drawSurface()
{
    int maxY, maxX;
    maxY = parent->surfaceMatrix.size();

    if (maxY == 0) {
        return;
    }

    maxX = parent->surfaceMatrix[0].size(); // because of rectangle matrix

    if (maxX == 0) {
        return;
    }

    //points
    glColor3f(1.000f, 0.498f, 0.314f); // red
    glPointSize(10.0F);
    glBegin(GL_POINTS);

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            //point
            glVertex3d(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
        }
    }

    glEnd();

    // connections between the points
    glColor3f(0.678f, 1.000f, 0.184f); // green
    glLineWidth(0.4f);
    glBegin(GL_LINES);

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            if (y > 0) {
                //line 1
                glVertex3d(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3d(parent->surfaceMatrix[y - 1][x].X, parent->surfaceMatrix[y - 1][x].Y, parent->surfaceMatrix[y - 1][x].Z);
            }

            if (y < maxY - 1) {
                //line2
                glVertex3d(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3d(parent->surfaceMatrix[y + 1][x].X, parent->surfaceMatrix[y + 1][x].Y, parent->surfaceMatrix[y + 1][x].Z);
            }

            if (x > 0) {
                //line 3
                glVertex3d(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3d(parent->surfaceMatrix[y][x - 1].X, parent->surfaceMatrix[y][x - 1].Y, parent->surfaceMatrix[y][x - 1].Z);
            }

            if (x < maxX - 1) {
                //line4
                glVertex3d(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3d(parent->surfaceMatrix[y][x + 1].X, parent->surfaceMatrix[y][x + 1].Y, parent->surfaceMatrix[y][x + 1].Z);
            }
        }
    }

    glEnd();
}


void GLWidget::drawInstrument()
{
    // instrument
    float startX = cnc->coord[X].posMm();
    float startY = cnc->coord[Y].posMm();
    float startZ = cnc->coord[Z].posMm();

    glColor3f(1.000f, 1.000f, 0.000f);
    glLineWidth(3);

    glTranslated(startX, startY, startZ);

    glVertexPointer(3, GL_FLOAT, 0, &instrumentArray[0]);

    glEnable(GL_VERTEX_ARRAY);

    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_COORD_ARRAY);

    glDrawArrays(GL_LINES, 0, instrumentArray.count());

    glDisable(GL_VERTEX_ARRAY);

    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);
}


void GLWidget::drawGrate()
{
    //
    glLineWidth(2.0f);

    glColor3f(0.541f, 0.169f, 0.886f);

    glEnable(GL_LINE_STIPPLE);

    GLushort pattern = 0x00FF;
    GLint factor = 2;

    glLineStipple( factor, pattern );

    glBegin(GL_LINE_STRIP); // normal lines

    glVertex3d(cnc->coord[X].softLimitMin, cnc->coord[Y].softLimitMin, 0);
    glVertex3d(cnc->coord[X].softLimitMax, cnc->coord[Y].softLimitMin, 0);
    glVertex3d(cnc->coord[X].softLimitMax, cnc->coord[Y].softLimitMax, 0);
    glVertex3d(cnc->coord[X].softLimitMin, cnc->coord[Y].softLimitMax, 0);
    glVertex3d(cnc->coord[X].softLimitMin, cnc->coord[Y].softLimitMin, 0);

    glEnd();

    glDisable(GL_LINE_STIPPLE);
}


void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0) {
        *angle += 360;
    }

    while (*angle > 360) {
        *angle -= 360;
    }
}


// from slider
void GLWidget::setXCoord(int x)
{
    if (parent->PosX != x) {
        parent->PosX = x;
    }
}


void GLWidget::setYCoord(int y)
{
    if (parent->PosY != y) {
        parent->PosY = y;
    }
}


// from slider
void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);

    if (angle != parent->PosAngleX) {
        parent->PosAngleX = angle;
        emit xRotationChanged(angle);
    }
}


void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);

    if (angle != parent->PosAngleY) {
        parent->PosAngleY = angle;
        emit yRotationChanged(angle);
    }
}


void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);

    if (angle != parent->PosAngleZ) {
        parent->PosAngleZ = angle;
        emit zRotationChanged(angle);
    }
}


void GLWidget::onPosAngleXm()
{
    --parent->PosAngleX;
    normalizeAngle(&parent->PosAngleX);
    emit xRotationChanged(parent->PosAngleX);
    updateGL();
}


void GLWidget::onPosAngleX()
{
    parent->PosAngleX = 0;
    emit xRotationChanged(parent->PosAngleX);
    updateGL();
}


void GLWidget::onPosAngleXp()
{
    ++parent->PosAngleX;
    normalizeAngle(&parent->PosAngleX);
    emit xRotationChanged(parent->PosAngleX);
    updateGL();
}


void GLWidget::onPosAngleYp()
{
    ++parent->PosAngleY;
    normalizeAngle(&parent->PosAngleY);
    emit yRotationChanged(parent->PosAngleY);
    updateGL();
}


void GLWidget::onPosAngleY()
{
    parent->PosAngleY = 0;
    emit yRotationChanged(parent->PosAngleY);
    updateGL();

}


void GLWidget::onPosAngleYm()
{
    --parent->PosAngleY;
    normalizeAngle(&parent->PosAngleY);
    emit yRotationChanged(parent->PosAngleY);
    updateGL();
}


void GLWidget::onPosAngleZp()
{
    ++parent->PosAngleZ;
    normalizeAngle(&parent->PosAngleZ);
    emit zRotationChanged(parent->PosAngleZ);
    updateGL();
}


void GLWidget::onPosAngleZ()
{
    parent->PosAngleZ = 0;
    emit zRotationChanged(parent->PosAngleZ);
    updateGL();
}


void GLWidget::onPosAngleZm()
{
    --parent->PosAngleZ;
    normalizeAngle(&parent->PosAngleZ);
    emit zRotationChanged(parent->PosAngleZ);
    updateGL();
}


void GLWidget::onDefaulPreview()
{
    initPreviewSettings();
}


void GLWidget::onRenderTimer()
{
    // ??
    Draw();
}


void GLWidget::saveGLState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}


void GLWidget::restoreGLState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

