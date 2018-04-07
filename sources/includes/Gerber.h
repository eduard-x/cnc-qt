/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
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


#ifndef GERBER_H
#define GERBER_H

#include <QString>
#include <QMutex>
#include <QList>
#include <QObject>
#include <QVector>
#include <QVector3D>

#include "GData.h"

// #include "Reader.h"

// #include <deque>
// #include <utility>
// #include "vec.h"

/* Externalize variables used by the scanner and parser. */



class Gerber : public QObject
{
        Q_OBJECT
    public:
        explicit Gerber(); // constructor
        ~Gerber(); // destructor

        QVector<GCodeData> *dataVector();
        bool readGCode(char *indata);

    private:
        void gerberInit();
        void gerberDestroy();


    signals:
        void logMessage(const QString &l);

    public:
        static QVector<GCodeData> gCodeVector;
        QMutex mut;
};


#endif // GERBER_H
