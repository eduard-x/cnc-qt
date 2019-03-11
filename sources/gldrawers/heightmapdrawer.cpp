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

#include "heightmapdrawer.h"

HeightMapDrawer::HeightMapDrawer()
{
    m_model = NULL;
    m_pointSize = 4;
    m_data = NULL;
}

QRectF HeightMapDrawer::borderRect() const
{
    return m_borderRect;
}


void HeightMapDrawer::setBorderRect(const QRectF &borderRect)
{
    m_borderRect = borderRect;
    update();
}


bool HeightMapDrawer::updateData()
{
    updateLinesData();
    updateGridData();
    updteInterpolationData();

    return true;
}


void HeightMapDrawer::updteInterpolationData()
{
    // Check if data is present
    if (!m_data || m_data->count() == 0) {
        m_lines.clear();
        return;
    }

    QColor color;

    // Clear data
    m_lines.clear();

    // Prepare vertex
    VertexData vertex;
    vertex.start = QVector3D(sNan, sNan, sNan);

    // Calculate grid parameters
    int interpolationPointsX = m_data->at(0).count();
    int interpolationPointsY = m_data->count();

    double interpolationStepX = interpolationPointsX > 1 ? m_borderRect.width() / (interpolationPointsX - 1) : 0;
    double interpolationStepY = interpolationPointsY > 1 ? m_borderRect.height() / (interpolationPointsY - 1) : 0;

    // Find min & max values for coloring
    double min = m_data->at(0).at(0);
    double max = min;

    for (int y = 0; y < interpolationPointsY; y++) {
        for (int x = 0; x < interpolationPointsX; x++) {
            min = qMin(min, m_data->at(y).at(x));
            max = qMax(max, m_data->at(y).at(x));
        }
    }

    // Horizontal, vertical lines
    for (int i = 0; i < interpolationPointsY; i++) {
        for (int j = 1; j < interpolationPointsX; j++) {
            if (qIsNaN(m_data->at(i).at(j))) {
                continue;
            }

            // Horizontal lines
            color.setHsvF(0.67 * (max - m_data->at(i).at(j - 1)) / (max - min), 1.0, 1.0);
            vertex.color = color;

            vertex.position = QVector3D(m_borderRect.x() + interpolationStepX * (j - 1), m_borderRect.y() + interpolationStepY * i, m_data->at(i).at(j - 1));
            m_lines.append(vertex);

            color.setHsvF(0.67 * (max - m_data->at(i).at(j)) / (max - min), 1.0, 1.0);
            vertex.color = color;

            vertex.position = QVector3D(m_borderRect.x() + interpolationStepX * j, m_borderRect.y() + interpolationStepY * i, m_data->at(i).at(j));
            m_lines.append(vertex);
        }
    }

    // Vertical lines
    for (int j = 0; j < interpolationPointsX; j++) {
        for (int i = 1; i < interpolationPointsY; i++) {
            if (qIsNaN(m_data->at(i).at(j))) {
                continue;
            }

            color.setHsvF(0.67 * (max - m_data->at(i - 1).at(j)) / (max - min), 1.0, 1.0);
            vertex.color = color;

            vertex.position = QVector3D(m_borderRect.x() + interpolationStepX * j, m_borderRect.y() + interpolationStepY * (i - 1), m_data->at(i - 1).at(j));
            m_lines.append(vertex);

            color.setHsvF(0.67 * (max - m_data->at(i).at(j)) / (max - min), 1.0, 1.0);
            vertex.color = color;

            vertex.position = QVector3D(m_borderRect.x() + interpolationStepX * j, m_borderRect.y() + interpolationStepY * i, m_data->at(i).at(j));
            m_lines.append(vertex);
        }
    }
}

void HeightMapDrawer::updateLinesData()
{
    m_lines = {
        {QVector3D(m_borderRect.x(), m_borderRect.y(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x(), m_borderRect.y() + m_borderRect.height(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x(), m_borderRect.y() + m_borderRect.height(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x() + m_borderRect.width(), m_borderRect.y() + m_borderRect.height(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x() + m_borderRect.width(), m_borderRect.y() + m_borderRect.height(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x() + m_borderRect.width(), m_borderRect.y(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x() + m_borderRect.width(), m_borderRect.y(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
        {QVector3D(m_borderRect.x(), m_borderRect.y(), 0), QColor(1.0, 0.0, 0.0), QVector3D(sNan, sNan, sNan)},
    };
}


void HeightMapDrawer::updateGridData()
{
    // Clear data
    m_lines.clear();
    m_points.clear();

    // Prepare vertex
    VertexData vertex;
    vertex.start = QVector3D(sNan, sNan, sNan);

    // Calculate grid parameters
    int gridPointsX = m_model->columnCount();
    int gridPointsY = m_model->rowCount();

    double gridStepX = gridPointsX > 1 ? m_borderRect.width() / (gridPointsX - 1) : 0;
    double gridStepY = gridPointsY > 1 ? m_borderRect.height() / (gridPointsY - 1) : 0;

    // Probe path / dots
    for (int i = 0; i < gridPointsY; i++) {
        for (int j = 0; j < gridPointsX; j++) {
            if (m_model == NULL || qIsNaN(m_model->data(m_model->index(i, j), Qt::UserRole).toDouble())) {
                vertex.color = QColor(1.0, 0.6, 0.0);
                vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * i, m_zTop);
                m_lines.append(vertex);
                vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * i, m_zBottom);
                m_lines.append(vertex);
            } else {
                vertex.color = QColor(0.0, 0.0, 1.0);
                vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * i, m_model->data(m_model->index(i, j), Qt::UserRole).toDouble());
                m_points.append(vertex);
            }
        }
    }

    // Horizontal, vertical  grid lines
    vertex.color = QColor(0.0, 0.0, 1.0);

    for (int i = 0; i < gridPointsY; i++) {
        for (int j = 1; j < gridPointsX; j++) {
            if (qIsNaN(m_model->data(m_model->index(i, j), Qt::UserRole).toDouble())) {
                continue;
            }

            // horizontal lines
            vertex.position = QVector3D(m_borderRect.x() + gridStepX * (j - 1), m_borderRect.y() + gridStepY * i, m_model->data(m_model->index(i, j - 1), Qt::UserRole).toDouble());
            m_lines.append(vertex);

            vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * i, m_model->data(m_model->index(i, j), Qt::UserRole).toDouble());
            m_lines.append(vertex);
        }
    }

    // Vertical grid lines
    vertex.color = QColor(0.0, 0.0, 1.0);

    for (int j = 0; j < gridPointsX; j++) {
        for (int i = 1; i < gridPointsY; i++) {
            if (qIsNaN(m_model->data(m_model->index(i, j), Qt::UserRole).toDouble())) {
                continue;
            }

            vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * (i - 1), m_model->data(m_model->index(i - 1, j), Qt::UserRole).toDouble());
            m_lines.append(vertex);

            vertex.position = QVector3D(m_borderRect.x() + gridStepX * j, m_borderRect.y() + gridStepY * i, m_model->data(m_model->index(i, j), Qt::UserRole).toDouble());
            m_lines.append(vertex);
        }
    }
}

QVector<QVector<double> > *HeightMapDrawer::data() const
{
    return m_data;
}

void HeightMapDrawer::setData(QVector<QVector<double> > *data)
{
    m_data = data;
    update();
}


QRectF HeightMapDrawer::gridSize() const
{
    return m_gridSize;
}

void HeightMapDrawer::setGridSize(const QRectF &gridSize)
{
    m_gridSize = gridSize;
    update();
}

double HeightMapDrawer::zTop() const
{
    return m_zTop;
}

void HeightMapDrawer::setZTop(double zTop)
{
    m_zTop = zTop;
    update();
}
double HeightMapDrawer::zBottom() const
{
    return m_zBottom;
}

void HeightMapDrawer::setZBottom(double zBottom)
{
    m_zBottom = zBottom;
    update();
}

QAbstractTableModel *HeightMapDrawer::model() const
{
    return m_model;
}

void HeightMapDrawer::setModel(QAbstractTableModel *model)
{
    m_model = model;
    update();
}
