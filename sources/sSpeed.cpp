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

#include <QtGui>


#include "includes/sSpeed.h"


class Settings;


/******************************************************************************
** SettingsSpeed
*/


SettingsSpeed::SettingsSpeed(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    //     setStyleSheet(parent->programStyleSheet);

    checkSwapX->setChecked(Settings::coord[X].invertDirection);
    checkUseX->setChecked(Settings::coord[X].enabled);
    checkSwapY->setChecked(Settings::coord[Y].invertDirection);
    checkUseY->setChecked(Settings::coord[Y].enabled);
    checkSwapZ->setChecked(Settings::coord[Z].invertDirection);
    checkUseZ->setChecked(Settings::coord[Z].enabled);
    checkSwapA->setChecked(Settings::coord[A].invertDirection);
    checkUseA->setChecked(Settings::coord[A].enabled);

    checkInvStepsX->setChecked(Settings::coord[X].invertPulses);
    checkInvStepsY->setChecked(Settings::coord[Y].invertPulses);
    checkInvStepsZ->setChecked(Settings::coord[Z].invertPulses);
    checkInvStepsA->setChecked(Settings::coord[A].invertPulses);

    numPulseX->setValue(Settings::coord[X].pulsePerMm);
    numPulseY->setValue(Settings::coord[Y].pulsePerMm);
    numPulseZ->setValue(Settings::coord[Z].pulsePerMm);
    numPulseA->setValue(Settings::coord[A].pulsePerMm);

    doubleSpinStartX->setValue(Settings::coord[X].minVeloLimit);
    doubleSpinStartY->setValue(Settings::coord[Y].minVeloLimit);
    doubleSpinStartZ->setValue(Settings::coord[Z].minVeloLimit);
    doubleSpinStartA->setValue(Settings::coord[A].minVeloLimit);

    doubleSpinEndX->setValue(Settings::coord[X].maxVeloLimit);
    doubleSpinEndY->setValue(Settings::coord[Y].maxVeloLimit);
    doubleSpinEndZ->setValue(Settings::coord[Z].maxVeloLimit);
    doubleSpinEndA->setValue(Settings::coord[A].maxVeloLimit);

    doubleSpinAccelX->setValue(Settings::coord[X].acceleration);
    doubleSpinAccelY->setValue(Settings::coord[Y].acceleration);
    doubleSpinAccelZ->setValue(Settings::coord[Z].acceleration);
    doubleSpinAccelA->setValue(Settings::coord[A].acceleration);

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


void SettingsSpeed::getSettings()
{
    Settings::coord[X].pulsePerMm = numPulseX->value();
    Settings::coord[Y].pulsePerMm = numPulseY->value();
    Settings::coord[Z].pulsePerMm = numPulseZ->value();
    Settings::coord[A].pulsePerMm = numPulseA->value();

    Settings::coord[X].minVeloLimit = doubleSpinStartX->value();
    Settings::coord[Y].minVeloLimit = doubleSpinStartY->value();
    Settings::coord[Z].minVeloLimit = doubleSpinStartZ->value();
    Settings::coord[A].minVeloLimit = doubleSpinStartA->value();

    Settings::coord[X].maxVeloLimit = doubleSpinEndX->value();
    Settings::coord[Y].maxVeloLimit = doubleSpinEndY->value();
    Settings::coord[Z].maxVeloLimit = doubleSpinEndZ->value();
    Settings::coord[A].maxVeloLimit = doubleSpinEndA->value();

    Settings::coord[X].acceleration = doubleSpinAccelX->value();
    Settings::coord[Y].acceleration = doubleSpinAccelY->value();
    Settings::coord[Z].acceleration = doubleSpinAccelZ->value();
    Settings::coord[A].acceleration = doubleSpinAccelA->value();

    Settings::coord[X].invertDirection = checkSwapX->isChecked();
    Settings::coord[X].invertPulses = checkInvStepsX->isChecked();
    Settings::coord[X].enabled = checkUseX->isChecked();
    Settings::coord[Y].invertDirection = checkSwapY->isChecked();
    Settings::coord[Y].invertPulses = checkInvStepsY->isChecked();
    Settings::coord[Y].enabled = checkUseY->isChecked();
    Settings::coord[Z].invertDirection = checkSwapZ->isChecked();
    Settings::coord[Z].invertPulses = checkInvStepsZ->isChecked();
    Settings::coord[A].invertDirection = checkSwapA->isChecked();
    Settings::coord[A].invertPulses = checkInvStepsA->isChecked();
    Settings::coord[Z].enabled = checkUseZ->isChecked();
    Settings::coord[A].enabled = checkUseA->isChecked();



}


void SettingsSpeed::translateWidget()
{
    //     labelInfo->setText(translate(_DEV_SIM_HELP));
    labelUse->setText(translate(_USE));

    labelSwap->setText(translate(_SWAP));

    labelStart->setText(translate(_STARTVELO));
    labelEnd->setText(translate(_ENDVELO));
    labelAcceleration->setText(translate(_ACCELERATION));

}

