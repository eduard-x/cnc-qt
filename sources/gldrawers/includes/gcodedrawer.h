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

#ifndef GCODEDRAWER_H
#define GCODEDRAWER_H

#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QList>
// #include "parser/linesegment.h"
// #include "parser/gcodeviewparse.h"
#include "shaderdrawable.h"

class GcodeDrawer : public ShaderDrawable
{
    public:
        explicit GcodeDrawer();

        void update();
        void update(QList<int> indexes);
        bool updateData();

        //         QVector3D getSizes();
        //         QVector3D getMinimumExtremes();
        //         QVector3D getMaximumExtremes();

        //         void setViewParser(GcodeViewParse* viewParser);
        //         GcodeViewParse* viewParser();

        bool simplify() const;
        void setSimplify(bool simplify);

        double simplifyPrecision() const;
        void setSimplifyPrecision(double simplifyPrecision);

        bool geometryUpdated();

        QColor colorNormal() const;
        void setColorNormal(const QColor &colorNormal);

        QColor colorHighlight() const;
        void setColorHighlight(const QColor &colorHighlight);

        QColor colorZMovement() const;
        void setColorZMovement(const QColor &colorZMovement);

        QColor colorDrawn() const;
        void setColorDrawn(const QColor &colorDrawn);

        QColor colorStart() const;
        void setColorStart(const QColor &colorStart);

        QColor colorEnd() const;
        void setColorEnd(const QColor &colorEnd);

    signals:

    public slots:

    private:
        //         GcodeViewParse *m_viewParser;
        bool m_simplify;
        double m_simplifyPrecision;
        bool m_geometryUpdated;

        QColor m_colorNormal;
        QColor m_colorDrawn;
        QColor m_colorHighlight;
        QColor m_colorZMovement;
        QColor m_colorStart;
        QColor m_colorEnd;

        QList<int> m_indexes;

        //         int getSegmentType(LineSegment *segment);
        //         QColor getSegmentColor(LineSegment *segment);
};

#endif // GCODEDRAWER_H
