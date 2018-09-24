/****************************************************************************
 * This file is a part of "grblControl" application.                        *
 * Copyright 2015 Hayrullin Denis Ravilevich                                *
 * https://github.com/Denvi/grblControl                                     *
 *                                                                          *
 * Qt, Linux developing                                                     *
 * Copyright (C) 2015-2018 by Eduard Kalinowski                             *
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

#ifndef HEIGHTMAPDRAWER_H
#define HEIGHTMAPDRAWER_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <QRectF>
#include <QPointF>
#include <QAbstractTableModel>
#include "shaderdrawable.h"

class HeightMapDrawer : public ShaderDrawable
{
    public:
        HeightMapDrawer();

        QRectF gridSize() const;
        void setGridSize(const QRectF &gridSize);

        QRectF borderRect() const;
        void setBorderRect(const QRectF &borderRect);

        double zTop() const;
        void setZTop(double zTop);

        double zBottom() const;
        void setZBottom(double zBottom);

        QAbstractTableModel *model() const;
        void setModel(QAbstractTableModel *model);

        QVector<QVector<double> > *data() const;
        void setData(QVector<QVector<double> > *data);

    protected:
        bool updateData();
        void updateLinesData();
        void updateGridData();
        void updteInterpolationData();

    private:
        QRectF m_borderRect;
        double m_zTop;
        double m_zBottom;
        QAbstractTableModel *m_model;
        QRectF m_gridSize;
        QVector<QVector<double>> *m_data;
        double Min(double v1, double v2);
        double Max(double v1, double v2);
};

#endif // HEIGHTMAPDRAWER_H
