/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * Qt developing                                                            *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
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


#include "includes/Settings.h"


/******************************************************************************
** SettingsDialog
*/


SettingsDialog::SettingsDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    _cnc = parent->cnc;

    setStyleSheet(parent->programStyleSheet);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    numPulseX->setValue(DeviceInfo::AxesX_PulsePerMm);
    numPulseY->setValue(DeviceInfo::AxesY_PulsePerMm);
    numPulseZ->setValue(DeviceInfo::AxesZ_PulsePerMm);

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
    DeviceInfo::DEMO_DEVICE      = checkBoxDemoController->isChecked();

    accept();
}

