/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
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

#include <QtGui>
#include <QUrl>
#include <QPixmap>

#include "MainWindow.h"
#include "About.h"
#include "version.h"


/******************************************************************************
** AboutDialog
*/


AboutDialog::AboutDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    setStyleSheet(parent->programStyleSheet);

    translateDialog();

    labelImage->setPixmap(QPixmap(":/images/cnc.png"));

    connect(pushButton, SIGNAL(clicked()), this, SLOT(reject()));

    adjustSize();
}


void AboutDialog::translateDialog()
{
    setWindowTitle(translate(ID_ABOUT_TITLE));
    labelAuthorNET->setText("<a href=\"zheigurov@gmail.com\">C#, Windows developing: S. Zheigurov</a>");
    labelProgAuthor->setText("<a href=\"eduard_kalinowski@yahoo.de\">Qt/C++, Linux developing: E. Kalinowski</a>");
    labelProgName->setText(translate(ID_PROG_NAME) + " v." + QString(CNCMK1QTVERSION));
    labelProgVersion->setText("");

    QString ab = translate(ID_ABOUT_TEXT);

    QString link1 = "http://www.planet-cnc.com";
    QString link2 = "http://www.selenur.ru";
    QString link3 = "http://www.cnc-club.ru/forum/viewtopic.php?f=16&t=7078&p=175365#p175365";
    QString link3_descr = "http://www.cnc-club.ru (forum)";
    QString link4 = "https://github.com/eduard-x/cnc-qt";

    ab.replace("\n", "<br>");
    ab = ab.arg("<a href=\"" + link1 + "\">" + link1 + "</a>")
         .arg("<a href=\"" + link2 + "\">" + link2 + "</a>")
         .arg("<a href=\"" + link3 + "\">" + link3_descr + "</a>")
         .arg("<a href=\"" + link4 + "\">" + link4 + "</a>");

    textInfo->setText(ab);
}

