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
#include <QGLWidget>

#include <deque>
#include <utility>
#include "includes/vec.h"

#include "includes/MainWindow.h"
#include "includes/mk1Controller.h"
#include "includes/GLWidget.h"

#include <math.h>

#define PI 3.14159265358979323846


GLint GLWidget::xAxis[][3] = {
    {0, 0, 0}, {10, 0, 0}, {10, 0, 0},
    {9, 1, 0}, {10, 0, 0}, {9, -1, 0}
};


GLint GLWidget::yAxis[][3] = {
    {0, 0, 0}, {0, 10, 0}, {0, 10, 0},
    {1, 9, 0}, {0, 10, 0}, { -1, 9, 0}
};


GLint GLWidget::zAxis[][3] = {
    {0, 0, 0}, {0, 0, 10}, {0, 0, 10},
    {1, 1, 9}, {0, 0, 10}, { -1, -1, 9}
};


GLint GLWidget::instrumentArray[][3] = {
    {0, 0, 0}, {0, 0, 4}, { -1, -1, 2},
    { -1, 1, 2}, {1, -1, 2}, {1, 1, 2},
    {1, 1, 2}, { -1, 1, 2}, {1, -1, 2},
    { -1, -1, 2}, { -1, -1, 2}, {0, 0, 0},
    {1, 1, 2}, {0, 0, 0}, {1, -1, 2},
    {0, 0, 0}, { -1, 1, 2}, {0, 0, 0}
};


GLWidget::GLWidget(QWidget *p)
    : QGLWidget(p)
{
    if (p == NULL) {
        return;
    }

    coordArray = NULL;
    colorArray = NULL;

    parent = (MainWindow*)p;

    workNum = 0;
    
    QString glStr = QString().sprintf("%s %s %s %s", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION), (char*)glGetString(GL_EXTENSIONS));

    qDebug() << glStr;
    initPreviewSettings();
}


GLWidget::~GLWidget()
{
}


void GLWidget::surfaceReloaded()
{
    for (int i = 0; i < workNum; i++) {
        if (parent->deltaFeed) {
            pointGL p;
            float pointX = parent->GCodeList.at(i).X;
            float pointY = parent->GCodeList.at(i).Y;
            float pointZ = parent->GCodeList.at(i).Z;
            pointZ += parent->GetDeltaZ(pointX, pointY);

            p = (pointGL) {
                pointX, pointY, pointZ
            };

            *(coordArray + i) = p;
        }
    }
}


void GLWidget::matrixReloaded()
{
    workNum = 0;

    if ( coordArray != NULL) {
        free(coordArray);
        coordArray = NULL;
    }

    if ( colorArray != NULL) {
        free(colorArray);
        colorArray = NULL;
    }

    workNum = parent->GCodeList.count();

    if (workNum > 1) {
        coordArray = (pointGL*) malloc( sizeof(pointGL) * (workNum));
        colorArray = (colorGL*) malloc( sizeof(colorGL) * (workNum));

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
            //coordinates of next point
            float pointX = vv.X;
            float pointY = vv.Y;
            float pointZ = vv.Z;

            //moving in G-code
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

            *(coordArray + currWorkPoint) = p;
            *(colorArray + currWorkPoint) = cl;

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
    //     glFrustum (-1, 1, -1, 1, 3, 15);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
}


void GLWidget::paintGL()
{
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Draw();
}


void GLWidget::resizeGL(int width, int height)
{
    // set the current size of view
    glViewport(0, 0, width, height);
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
    glClearColor(0.75f, 0.75f, 0.75f, 1);                  // set gray color of background

    glLoadIdentity();                                   // clean

    glPushMatrix();                                     // push current matrix

    /// move eyes point for better view of object
    glTranslated(parent->PosX / GLSCALE, parent->PosY / GLSCALE, parent->PosZ / GLSCALE);

    ///угловое вращение
    glRotated(parent->PosAngleX, 1, 0, 0);
    glRotated(parent->PosAngleY, 0, 1, 0);
    glRotated(parent->PosAngleZ, 0, 0, 1);

    //TODO: to implement the proportions if width or high will be changed

    //glScalef(preview_setting.posZoom / (p1*1000), preview_setting.posZoom / p2, preview_setting.posZoom / 1000.0);

    // ReSharper disable once PossibleLossOfFraction

    if (windowState() != Qt::WindowMinimized) {
        int p1 = width();
        int p2 = height();

        float n = 1;

        if (p2 < p1) {
            n = (p1 / p2);
        }

        float scaleX = parent->PosZoom / (GLSCALE * n);
        float scaleY = parent->PosZoom / GLSCALE;
        float scaleZ = parent->PosZoom / GLSCALE;

        glScaled(scaleX, scaleY, scaleZ);
    }

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

    glEnableClientState(GL_VERTEX_ARRAY);

    // x
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertexPointer(3, GL_INT, 0, xAxis);
    glDrawArrays(GL_LINES, 0, 6); // draw array of lines
    renderText(12.0, 0.0, 0.0, QString("X")); //coordinates of text

    // y
    glColor3f(1.0F, 0, 0.0F);
    glVertexPointer(3, GL_INT, 0, yAxis);
    glDrawArrays(GL_LINES, 0, 6);
    renderText(0.0, 12.0, 0.0, QString("Y")); //coordinates of text

    // z
    glColor3f(0.0F, 1.0, 1.0F);
    glVertexPointer(3, GL_INT, 0, zAxis);
    glDrawArrays(GL_LINES, 0, 6);
    renderText(0.0, 0.0, 12.0, QString("Z")); //coordinates of text

    glDisableClientState(GL_VERTEX_ARRAY);
}


void GLWidget::drawWorkField()
{
    if (coordArray == NULL || colorArray == NULL) {
        return;
    }

    if (workNum < 2) {
        return;
    }

    glPushMatrix();

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_COORD_ARRAY);

    glLineWidth(0.3f);

    glVertexPointer(3, GL_FLOAT, 0, &coordArray[0]);
    glColorPointer(3, GL_FLOAT, 0, &colorArray[0]);
    glDrawArrays(GL_LINE_STRIP, 0, workNum);

    // select with 3.0 the current cut of object
    if (Task::Status == Waiting) {
        int numSelectStart = Task::posCodeStart;
        int numSelectStop = Task::posCodeEnd;
        glLineWidth(3.0f);
        glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelectStart]);
        glColorPointer(3, GL_FLOAT, 0, &colorArray[numSelectStart]);
        glDrawArrays(GL_LINE_STRIP, 0, numSelectStop - numSelectStart - 1);
    } else {
        int numSelect = parent->cnc->numberCompleatedInstructions() - 1;
        glLineWidth(3.0f);
        glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelect]);
        glColorPointer(3, GL_FLOAT, 0, &colorArray[numSelect]);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
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
        glColor3f(0.0, 0.0, 0.0);

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
        glColor3f(0.0, 0.0, 0.0); // white

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

    //Добавим связи между точками
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
    //нарисуем курсор
    float startX = DeviceInfo::AxesX_PositionMM();
    float startY = DeviceInfo::AxesY_PositionMM();
    float startZ = DeviceInfo::AxesZ_PositionMM();

    glPushMatrix();

    glEnableClientState(GL_VERTEX_ARRAY);

    glColor3f(1.000f, 1.000f, 0.000f);
    glLineWidth(3);

    glTranslated(startX, startY, startZ);

    glVertexPointer(3, GL_INT, 0, instrumentArray);
    glDrawArrays(GL_LINES, 0, 18);

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}


void GLWidget::drawGrate()
{
    //
    glLineWidth(4.0f);

    glColor3f(0.541f, 0.169f, 0.886f);

    glBegin(GL_LINE_STRIP); //normal lines

    glVertex3d(parent->grateXmin, parent->grateYmin, 0);
    glVertex3d(parent->grateXmax, parent->grateYmin, 0);
    glVertex3d(parent->grateXmax, parent->grateYmax, 0);
    glVertex3d(parent->grateXmin, parent->grateYmax, 0);
    glVertex3d(parent->grateXmin, parent->grateYmin, 0);

    glEnd();
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

