/****************************************************************************
 * This file is a part of "grblControl" application.                        *
 * Copyright 2015 Hayrullin Denis Ravilevich                                *
 * https://github.com/Denvi/grblControl                                     *
 *                                                                          *
 * Qt, Linux developing                                                     *
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

#ifndef TOOLDRAWER_H
#define TOOLDRAWER_H

#include <QVector3D>
#include <QTimer>
#include <QColor>
#include <cmath>
#include "shaderdrawable.h"

class ToolDrawer : public ShaderDrawable
{
    public:
        explicit ToolDrawer();

        double toolDiameter() const;
        void setToolDiameter(double toolDiameter);

        double toolLength() const;
        void setToolLength(double toolLength);

        QVector3D toolPosition() const;
        void setToolPosition(const QVector3D &toolPosition);

        double rotationAngle() const;
        void setRotationAngle(double rotationAngle);
        void rotate(double angle);

        double toolAngle() const;
        void setToolAngle(double toolAngle);

        QColor color() const;
        void setColor(const QColor &color);

    signals:

    public slots:

    protected:
        bool updateData();

    private:
        double m_toolDiameter;
        double m_toolLength;
        double m_endLength;
        QVector3D m_toolPosition;
        double m_rotationAngle;
        double m_toolAngle;
        QColor m_color;

        double normalizeAngle(double angle);
        QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, QColor color);
};

#endif // TOOLDRAWER_H
