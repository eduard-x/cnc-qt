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

#include <QApplication>
#include <QTextCodec>

#include "includes/MainWindow.h"

int main(int argc, char **argv)
{
    //     Q_INIT_RESOURCE(undo);

    QApplication app(argc, argv);

#if QT_VERSION < 0x050000
    // ask QString in Qt 4 to interpret all char* as UTF-8,
    // like Qt 5 does.
    // 106 is "UTF-8", this is faster than lookup by name
    // [http://www.iana.org/assignments/character-sets/character-sets.xml]
    QTextCodec::setCodecForCStrings(QTextCodec::codecForMib(106));
    // and for reasons I can't understand, I need to do the same again for tr
    // even though that clearly uses C strings as well...
    QTextCodec::setCodecForTr(QTextCodec::codecForMib(106));
#ifdef Q_OS_WIN
    QFile::setDecodingFunction(decodeUtf8);
    QFile::setEncodingFunction(encodeUtf8);
#endif
#else
    // for Win32 and Qt5 we try to set the locale codec to UTF-8.
    // this makes QFile::encodeName() work.
#ifdef Q_OS_WIN
    QTextCodec::setCodecForLocale(QTextCodec::codecForMib(106));
#endif
#endif

    MainWindow win;
    win.resize(800, 600);
    win.show();

    return app.exec();
};
