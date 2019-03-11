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


#include "sSystem.h"
#include "Settings.h"

// class Settings;
/******************************************************************************
** SettingsSystem
*/


SettingsSystem::SettingsSystem(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    backlashX->setValue(Settings::coord[X].backlash);
    backlashY->setValue(Settings::coord[Y].backlash);
    backlashZ->setValue(Settings::coord[Z].backlash);
    backlashA->setValue(Settings::coord[A].backlash);

    spinBoxLookLines->setValue(Settings::maxLookaheadAngle);



    //     connect(checkCorrecture, SIGNAL(stateChanged ( int)), this, SLOT(checkedChanged( int )));
    //
    //     checkCorrecture->setChecked(true);
    //     checkCorrecture->setChecked(false); // toggle
    //
    //     connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSaveChange()));
    //     connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //
    //     doubleSpinOffsetX->setValue(parent->deltaX);
    //     doubleSpinOffsetY->setValue(parent->deltaY);
    //     doubleSpinOffsetZ->setValue(parent->deltaZ);
    //
    //     checkBoxZ->setChecked(parent->deltaFeed);
    //
    //     doubleSpinResizeX->setValue(parent->coeffSizeX);
    //     doubleSpinResizeY->setValue(parent->coeffSizeY);

    //     checkResizeZ->setChecked();

    translateWidget();

    adjustSize();
}

SettingsSystem::~SettingsSystem()
{
}

void SettingsSystem::getSettings()
{
    Settings::coord[X].backlash = backlashX->value();
    Settings::coord[Y].backlash = backlashY->value();
    Settings::coord[Z].backlash = backlashZ->value();
    Settings::coord[A].backlash = backlashA->value();

    Settings::maxLookaheadAngle = spinBoxLookLines->value();
}


void SettingsSystem::translateWidget()
{
    //     setWindowTitle(translate(_EDITGCODE_TITLE));
    //     checkCorrecture->setText(translate(_CORRECTURE));
    //     groupResize->setTitle(translate(_PROPORTION));
    //     groupOffset->setTitle(translate(_OFFSET_GCODE));
    //     checkBoxZ->setText(translate(_CORRECT_Z));
}

