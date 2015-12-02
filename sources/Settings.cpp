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

    cnc = parent->cnc;

    setStyleSheet(parent->programStyleSheet);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    numPulseX->setValue(cnc->coord[X].pulsePerMm);
    numPulseY->setValue(cnc->coord[Y].pulsePerMm);
    numPulseZ->setValue(cnc->coord[Z].pulsePerMm);
    numPulseA->setValue(cnc->coord[A].pulsePerMm);

    doubleSpinStartX->setValue(cnc->coord[X].minVelo);
    doubleSpinStartY->setValue(cnc->coord[Y].minVelo);
    doubleSpinStartZ->setValue(cnc->coord[Z].minVelo);
    doubleSpinStartA->setValue(cnc->coord[A].minVelo);

    doubleSpinEndX->setValue(cnc->coord[X].maxVelo);
    doubleSpinEndY->setValue(cnc->coord[Y].maxVelo);
    doubleSpinEndZ->setValue(cnc->coord[Z].maxVelo);
    doubleSpinEndA->setValue(cnc->coord[A].maxVelo);

    doubleSpinAccelX->setValue(cnc->coord[X].acceleration);
    doubleSpinAccelY->setValue(cnc->coord[Y].acceleration);
    doubleSpinAccelZ->setValue(cnc->coord[Z].acceleration);
    doubleSpinAccelA->setValue(cnc->coord[A].acceleration);

    checkBoxDemoController->setChecked(cnc->DEMO_DEVICE);

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
    cnc->coord[X].pulsePerMm = numPulseX->value();
    cnc->coord[Y].pulsePerMm = numPulseY->value();
    cnc->coord[Z].pulsePerMm = numPulseZ->value();
    cnc->coord[A].pulsePerMm = numPulseA->value();

    cnc->coord[X].minVelo = doubleSpinStartX->value();
    cnc->coord[Y].minVelo = doubleSpinStartY->value();
    cnc->coord[Z].minVelo = doubleSpinStartZ->value();
    cnc->coord[A].minVelo = doubleSpinStartA->value();

    cnc->coord[X].maxVelo = doubleSpinEndX->value();
    cnc->coord[Y].maxVelo = doubleSpinEndY->value();
    cnc->coord[Z].maxVelo = doubleSpinEndZ->value();
    cnc->coord[A].maxVelo = doubleSpinEndA->value();

    cnc->coord[X].acceleration = doubleSpinAccelX->value();
    cnc->coord[Y].acceleration = doubleSpinAccelY->value();
    cnc->coord[Z].acceleration = doubleSpinAccelZ->value();
    cnc->coord[A].acceleration = doubleSpinAccelA->value();

    cnc->DEMO_DEVICE  = checkBoxDemoController->isChecked();

    accept();
}

