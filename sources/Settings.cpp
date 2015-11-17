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
// #include <QUrl>

#include "includes/mk1Controller.h"
#include "includes/Settings.h"


/******************************************************************************
** SettingsDialog
*/


SettingsDialog::SettingsDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    setStyleSheet(parent->programStyleSheet);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    numPulseX->setValue(DeviceInfo::AxesX_PulsePerMm);
    numPulseY->setValue(DeviceInfo::AxesY_PulsePerMm);
    numPulseZ->setValue(DeviceInfo::AxesZ_PulsePerMm);
    numPulseA->setValue(DeviceInfo::AxesA_PulsePerMm);

    doubleSpinStartX->setValue(DeviceInfo::AxesX_StartVelo);
    doubleSpinStartY->setValue(DeviceInfo::AxesY_StartVelo);
    doubleSpinStartZ->setValue(DeviceInfo::AxesZ_StartVelo);
    doubleSpinStartA->setValue(DeviceInfo::AxesA_StartVelo);

    doubleSpinEndX->setValue(DeviceInfo::AxesX_EndVelo);
    doubleSpinEndY->setValue(DeviceInfo::AxesY_EndVelo);
    doubleSpinEndZ->setValue(DeviceInfo::AxesZ_EndVelo);
    doubleSpinEndA->setValue(DeviceInfo::AxesA_EndVelo);

    doubleSpinAccelX->setValue(DeviceInfo::AxesX_Acceleration);
    doubleSpinAccelY->setValue(DeviceInfo::AxesY_Acceleration);
    doubleSpinAccelZ->setValue(DeviceInfo::AxesZ_Acceleration);
    doubleSpinAccelA->setValue(DeviceInfo::AxesA_Acceleration);

    checkBoxDemoController->setChecked(DeviceInfo::DEMO_DEVICE);

    translateDialog();

    adjustSize();
}


void SettingsDialog::translateDialog()
{
    setWindowTitle(translate(_SETTINGS_TITLE));
    groupBox1->setTitle(translate(_PULSES_PER_MM));
    checkBoxDemoController->setText(translate(_DEV_SIMULATION));
    labelInfo->setText(translate(_DEV_SIM_HELP));
}


void SettingsDialog::onSave()
{
    DeviceInfo::AxesX_PulsePerMm = numPulseX->value();
    DeviceInfo::AxesY_PulsePerMm = numPulseY->value();
    DeviceInfo::AxesZ_PulsePerMm = numPulseZ->value();
    DeviceInfo::AxesA_PulsePerMm = numPulseA->value();

    DeviceInfo::AxesX_StartVelo = doubleSpinStartX->value();
    DeviceInfo::AxesY_StartVelo = doubleSpinStartY->value();
    DeviceInfo::AxesZ_StartVelo = doubleSpinStartZ->value();
    DeviceInfo::AxesA_StartVelo = doubleSpinStartA->value();

    DeviceInfo::AxesX_EndVelo = doubleSpinEndX->value();
    DeviceInfo::AxesY_EndVelo = doubleSpinEndY->value();
    DeviceInfo::AxesZ_EndVelo = doubleSpinEndZ->value();
    DeviceInfo::AxesA_EndVelo = doubleSpinEndA->value();

    DeviceInfo::AxesX_Acceleration = doubleSpinAccelX->value();
    DeviceInfo::AxesY_Acceleration = doubleSpinAccelY->value();
    DeviceInfo::AxesZ_Acceleration = doubleSpinAccelZ->value();
    DeviceInfo::AxesA_Acceleration = doubleSpinAccelA->value();

    DeviceInfo::DEMO_DEVICE      = checkBoxDemoController->isChecked();

    accept();
}

