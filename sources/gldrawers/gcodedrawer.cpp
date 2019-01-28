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

#include "gcodedrawer.h"

GcodeDrawer::GcodeDrawer()
{
    m_geometryUpdated = false;
}

void GcodeDrawer::update()
{
    m_indexes.clear();
    m_geometryUpdated = false;
    ShaderDrawable::update();
}

void GcodeDrawer::update(QList<int> indexes)
{
    // Store segments to update
    m_indexes += indexes;
    ShaderDrawable::update();
}

bool GcodeDrawer::updateData()
{
    if (m_indexes.isEmpty()) {
        qDebug() << "updating geometry" << this;
#if 0
        QList<LineSegment*> *list = m_viewParser->getLines();
        VertexData vertex;

        // Create vertices array
        // Clear all vertex data
        m_lines.clear();
        m_points.clear();

        bool drawFirstPoint = true;

        for (int i = 0; i < list->count(); i++) {

            if (qIsNaN(list->at(i)->getEnd().z())) {
                continue;
            }

            // Find first point of toolpath
            if (drawFirstPoint) {

                if (qIsNaN(list->at(i)->getEnd().x()) || qIsNaN(list->at(i)->getEnd().y())) {
                    continue;
                }

                // Draw first toolpath point
                vertex.color = m_colorStart;
                vertex.position = list->at(i)->getEnd();
                vertex.start = QVector3D(sNan, sNan, sNan);
                m_points.append(vertex);

                drawFirstPoint = false;
                continue;
            }

            // Prepare vertices
            if (list->at(i)->isFastTraverse()) {
                vertex.start = list->at(i)->getStart();
            } else {
                vertex.start = QVector3D(sNan, sNan, sNan);
            }

            // Simplify geometry
            int j = i;

            if (m_simplify && i < list->count() - 1) {
                QVector3D start = list->at(i)->getEnd() - list->at(i)->getStart();
                QVector3D next;
                double length = start.length();
                bool straight = false;

                do {
                    list->at(i)->setVertexIndex(m_lines.count()); // Store vertex index
                    i++;

                    if (i < list->count() - 1) {
                        next = list->at(i)->getEnd() - list->at(i)->getStart();
                        length += next.length();
                        straight = start.crossProduct(start.normalized(), next.normalized()).length() < 0.025;
                    }

                    // Split short & straight lines
                } while ((length < m_simplifyPrecision || straight) && i < list->count()
                         && getSegmentType(list->at(i)) == getSegmentType(list->at(j)));

                i--;
            } else {
                list->at(i)->setVertexIndex(m_lines.count()); // Store vertex index
            }

            // Set color
            vertex.color = getSegmentColor(list->at(i));

            // Line start
            vertex.position = list->at(j)->getStart();
            m_lines.append(vertex);

            // Line end
            vertex.position = list->at(i)->getEnd();
            m_lines.append(vertex);

            // Draw last toolpath point
            if (i == list->count() - 1) {
                vertex.color = m_colorEnd;
                vertex.position = list->at(i)->getEnd();
                vertex.start = QVector3D(sNan, sNan, sNan);
                m_points.append(vertex);
            }
        }

#endif
        m_indexes.clear();
        m_geometryUpdated = true;
        return true;
    } else {
        // Update vertices
#if 0
        QList<LineSegment*> list = m_viewParser->getLineSegmentList();
        int vertexIndex;

        // Try to map vbo
        VertexData* data;
        data = (VertexData*)m_vbo.map(QOpenGLBuffer::ReadWrite); // Already binded

        // Data to update
        VertexData *vertices;
        vertices = data ? data : m_lines.data();

        // Prepare colors
        QColor drawnColor = m_colorDrawn;
        QColor highlightColor = m_colorHighlight;

        // Update vertices for each line segment
        foreach (int i, m_indexes) {
            // Update vertex pair
            if (i < 0 || i > list.count() - 1) {
                continue;
            }

            vertexIndex = list[i]->vertexIndex();

            if (vertexIndex >= 0) {
                // Update vertex array
                if (vertices[vertexIndex].color == drawnColor // If vertex of drawn segment
                        && getSegmentColor(list[i]) == highlightColor); // dont highlight
                else {
                    vertices[vertexIndex].color = getSegmentColor(list[i]);
                    vertices[vertexIndex + 1].color = vertices[vertexIndex].color;
                }
            }
        }

        m_indexes.clear();

        if (data) {
            m_vbo.unmap();
            return false; // Update only vao
        } else {
            return true;    // Update full vbo via allocate
        }

#endif
    }
}

// QColor GcodeDrawer::getSegmentColor(LineSegment *segment)
// {
//     if (segment->drawn()) {
//         return m_colorDrawn;    //QVector3D(0.85, 0.85, 0.85);
//     } else if (segment->isHightlight()) {
//         return m_colorHighlight;    //QVector3D(0.57, 0.51, 0.9);
//     } else if (segment->isFastTraverse()) {
//         return m_colorNormal;    // QVector3D(0.0, 0.0, 0.0);
//     } else if (segment->isZMovement()) {
//         return m_colorZMovement;    //QVector3D(1.0, 0.0, 0.0);
//     } else {
//         return m_colorNormal;    //QVector3D(0.0, 0.0, 0.0);
//     }
// }
//
// int GcodeDrawer::getSegmentType(LineSegment* segment)
// {
//     return segment->isFastTraverse() + segment->isZMovement() * 2;
// }

// QVector3D GcodeDrawer::getSizes()
// {
//     QVector3D min = m_viewParser->getMinimumExtremes();
//     QVector3D max = m_viewParser->getMaximumExtremes();
//
//     return QVector3D(max.x() - min.x(), max.y() - min.y(), max.z() - min.z());
// }
//
// QVector3D GcodeDrawer::getMinimumExtremes()
// {
//     return m_viewParser->getMinimumExtremes();
// }
//
// QVector3D GcodeDrawer::getMaximumExtremes()
// {
//     return m_viewParser->getMaximumExtremes();
// }
//
// void GcodeDrawer::setViewParser(GcodeViewParse* viewParser)
// {
//     m_viewParser = viewParser;
// }
//
// GcodeViewParse *GcodeDrawer::viewParser()
// {
//     return m_viewParser;
// }
bool GcodeDrawer::simplify() const
{
    return m_simplify;
}

void GcodeDrawer::setSimplify(bool simplify)
{
    m_simplify = simplify;
}
double GcodeDrawer::simplifyPrecision() const
{
    return m_simplifyPrecision;
}

void GcodeDrawer::setSimplifyPrecision(double simplifyPrecision)
{
    m_simplifyPrecision = simplifyPrecision;
}

bool GcodeDrawer::geometryUpdated()
{
    return m_geometryUpdated;
}
QColor GcodeDrawer::colorNormal() const
{
    return m_colorNormal;
}

void GcodeDrawer::setColorNormal(const QColor &colorNormal)
{
    m_colorNormal = colorNormal;
}

QColor GcodeDrawer::colorHighlight() const
{
    return m_colorHighlight;
}

void GcodeDrawer::setColorHighlight(const QColor &colorHighlight)
{
    m_colorHighlight = colorHighlight;
}
QColor GcodeDrawer::colorZMovement() const
{
    return m_colorZMovement;
}

void GcodeDrawer::setColorZMovement(const QColor &colorZMovement)
{
    m_colorZMovement = colorZMovement;
}

QColor GcodeDrawer::colorDrawn() const
{
    return m_colorDrawn;
}

void GcodeDrawer::setColorDrawn(const QColor &colorDrawn)
{
    m_colorDrawn = colorDrawn;
}
QColor GcodeDrawer::colorStart() const
{
    return m_colorStart;
}

void GcodeDrawer::setColorStart(const QColor &colorStart)
{
    m_colorStart = colorStart;
}
QColor GcodeDrawer::colorEnd() const
{
    return m_colorEnd;
}

void GcodeDrawer::setColorEnd(const QColor &colorEnd)
{
    m_colorEnd = colorEnd;
}









