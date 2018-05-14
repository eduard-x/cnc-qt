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


#include <QObject>
#include <QRegExp>
#include <QDebug>
#include <QString>

#include <QtCore/qmath.h>

#include "includes/Settings.h"
#include "includes/GCode.h"
#include "includes/MainWindow.h"


#include "includes/GData.h"

#include "parse_gcode.h"

#include "scan_gcode.h"


// is static
QVector<ParserData> GData::dataVector;
QMap<QString, float> GData::dataVaris;

/**
 * @brief constructor
 *
 */
GData::GData()
{
}

/**
 * @brief destructor
 *
 */
GData::~GData()
{
    dataVector.clear();
}


void GData::gcodeInit()
{
    gcode_lineno = 0;
    //      gcode_result = NULL;
    //   gcode_vector = NULL;
    //   gcode_header = NULL;
}



void GData::gcodeDestroy()
{
    //      if (csv_result != NULL) {
    //     // delete associated dataset
    //     delete csv_result;
    //     csv_result = NULL;
    //   }
    //   if (csv_vector != NULL) {
    //     csv_finalize ();
    //     csv_vector = NULL;
    //   }
}


/**
 * @brief read and parse into ParserData list and OpenGL list
 * @see for the optimizations see https://blog.qt.io/blog/2014/06/13/qt-weekly-13-qstringliteral/
 * TODO convert QString to QStringLiteral
 *
 */
bool GData::readGCode(char *indata)
{
    int ret = true;

    dataVector.clear();

    mut.lock();

    gcodeInit();

    /* because the data in already in buffer 'indata' */

    YY_BUFFER_STATE bs = gcode__scan_string(indata);
    gcode__switch_to_buffer(bs);

    if ( gcode_parse() != 0) {
        ret = false;
    }

    gcode_lex_destroy();

    mut.unlock();

    if (!ret) {
        gcodeDestroy();
        return false;
    }

    gcodeDestroy();

    return true;
}


