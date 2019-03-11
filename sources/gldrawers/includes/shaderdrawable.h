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

#ifndef SHADERDRAWABLE_H
#define SHADERDRAWABLE_H

#include <QObject>
#include <QColor>
#include <QVector3D>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
// #include "utils/util.h"


#define sNan qQNaN()


struct VertexData {
    QVector3D position;
    QColor color;
    QVector3D start;
};

class ShaderDrawable : protected QOpenGLFunctions
{
    public:
        explicit ShaderDrawable();
        ~ShaderDrawable();
        void update();
        void draw(QOpenGLShaderProgram *shaderProgram);

        bool needsUpdateGeometry() const;
        void updateGeometry(QOpenGLShaderProgram *shaderProgram = 0);

        virtual QVector3D getSizes();
        virtual QVector3D getMinimumExtremes();
        virtual QVector3D getMaximumExtremes();
        virtual int getVertexCount();

        double lineWidth() const;
        void setLineWidth(double lineWidth);

        bool visible() const;
        void setVisible(bool visible);

        double pointSize() const;
        void setPointSize(double pointSize);

    signals:

    public slots:

    protected:
        double m_lineWidth;
        double m_pointSize;
        bool m_visible;
        QVector<VertexData> m_lines;
        QVector<VertexData> m_points;

        QOpenGLBuffer m_vbo; // Protected for direct vbo access

        virtual bool updateData();

    private:
        QOpenGLVertexArrayObject m_vao;

        bool m_needsUpdateGeometry;

        void init();
};

#endif // SHADERDRAWABLE_H
