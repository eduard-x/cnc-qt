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

    numPulseX->setValue(parent->cnc->AxesX_PulsePerMm);
    numPulseY->setValue(parent->cnc->AxesY_PulsePerMm);
    numPulseZ->setValue(parent->cnc->AxesZ_PulsePerMm);
    numPulseA->setValue(parent->cnc->AxesA_PulsePerMm);

    doubleSpinStartX->setValue(parent->cnc->AxesX_StartVelo);
    doubleSpinStartY->setValue(parent->cnc->AxesY_StartVelo);
    doubleSpinStartZ->setValue(parent->cnc->AxesZ_StartVelo);
    doubleSpinStartA->setValue(parent->cnc->AxesA_StartVelo);

    doubleSpinEndX->setValue(parent->cnc->AxesX_EndVelo);
    doubleSpinEndY->setValue(parent->cnc->AxesY_EndVelo);
    doubleSpinEndZ->setValue(parent->cnc->AxesZ_EndVelo);
    doubleSpinEndA->setValue(parent->cnc->AxesA_EndVelo);

    doubleSpinAccelX->setValue(parent->cnc->AxesX_Acceleration);
    doubleSpinAccelY->setValue(parent->cnc->AxesY_Acceleration);
    doubleSpinAccelZ->setValue(parent->cnc->AxesZ_Acceleration);
    doubleSpinAccelA->setValue(parent->cnc->AxesA_Acceleration);

    checkBoxDemoController->setChecked(parent->cnc->DEMO_DEVICE);

    translateDialog();

    adjustSize();
}


void SettingsDialog::translateDialog()
{
    setWindowTitle(translate(_SETTINGS_TITLE));
    groupBoxImpulses->setTitle(translate(_PULSES_PER_MM));
    checkBoxDemoController->setText(translate(_DEV_SIMULATION));
    labelInfo->setText(translate(_DEV_SIM_HELP));

    labelStart->setText(translate(_STARTVELO));
    labelEnd->setText(translate(_ENDVELO));
    labelAcceleration->setText(translate(_ACCELERATION));

    QList<QAbstractButton*> l = buttonBox->buttons();
    QStringList strl = (QStringList() << translate(_OK) << translate(_CANCEL));

    for(int i = 0; i < l.count(); i++) {
        l[i]->setText(strl.at(i));
    }
}


void SettingsDialog::onSave()
{
    parent->cnc->AxesX_PulsePerMm = numPulseX->value();
    parent->cnc->AxesY_PulsePerMm = numPulseY->value();
    parent->cnc->AxesZ_PulsePerMm = numPulseZ->value();
    parent->cnc->AxesA_PulsePerMm = numPulseA->value();

    parent->cnc->AxesX_StartVelo = doubleSpinStartX->value();
    parent->cnc->AxesY_StartVelo = doubleSpinStartY->value();
    parent->cnc->AxesZ_StartVelo = doubleSpinStartZ->value();
    parent->cnc->AxesA_StartVelo = doubleSpinStartA->value();

    parent->cnc->AxesX_EndVelo = doubleSpinEndX->value();
    parent->cnc->AxesY_EndVelo = doubleSpinEndY->value();
    parent->cnc->AxesZ_EndVelo = doubleSpinEndZ->value();
    parent->cnc->AxesA_EndVelo = doubleSpinEndA->value();

    parent->cnc->AxesX_Acceleration = doubleSpinAccelX->value();
    parent->cnc->AxesY_Acceleration = doubleSpinAccelY->value();
    parent->cnc->AxesZ_Acceleration = doubleSpinAccelZ->value();
    parent->cnc->AxesA_Acceleration = doubleSpinAccelA->value();

    parent->cnc->DEMO_DEVICE  = checkBoxDemoController->isChecked();

    accept();
}

