/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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


#include "includes/sWorkbench.h"



class Settings;
/******************************************************************************
** SettingsWorkbench
*/


SettingsWorkbench::SettingsWorkbench(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    //     setStyleSheet(parent->programStyleSheet);


    QStringList seqList = (QStringList() << "1" << "2" << "3" << "4");
    comboSeqX->addItems(seqList);
    comboSeqY->addItems(seqList);
    comboSeqZ->addItems(seqList);
    comboSeqA->addItems(seqList);




    //     grpArr.clear();



    checkSoftX->setChecked(Settings::coord[X].checkSoftLimits);


    doubleXmin->setValue(Settings::coord[X].softLimitMin);
    doubleXmax->setValue(Settings::coord[X].softLimitMax);
    doubleRangeMinX->setValue(Settings::coord[X].workAreaMin);
    doubleRangeMaxX->setValue(Settings::coord[X].workAreaMax);

    checkSoftY->setChecked(Settings::coord[Y].checkSoftLimits);

    doubleYmin->setValue(Settings::coord[Y].softLimitMin);
    doubleYmax->setValue(Settings::coord[Y].softLimitMax);
    doubleRangeMinY->setValue(Settings::coord[Y].workAreaMin);
    doubleRangeMaxY->setValue(Settings::coord[Y].workAreaMax);

    checkSoftZ->setChecked(Settings::coord[Z].checkSoftLimits);

    doubleZmin->setValue(Settings::coord[Z].softLimitMin);
    doubleZmax->setValue(Settings::coord[Z].softLimitMax);
    doubleRangeMinZ->setValue(Settings::coord[Z].workAreaMin);
    doubleRangeMaxZ->setValue(Settings::coord[Z].workAreaMax);

    checkSoftA->setChecked(Settings::coord[A].checkSoftLimits);

    doubleAmin->setValue(Settings::coord[A].softLimitMin);
    doubleAmax->setValue(Settings::coord[A].softLimitMax);
    doubleRangeMinA->setValue(Settings::coord[A].workAreaMin);
    doubleRangeMaxA->setValue(Settings::coord[A].workAreaMax);


    checkBoxDemoController->setChecked(Settings::DEMO_DEVICE);


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

SettingsWorkbench::~SettingsWorkbench()
{
}

void SettingsWorkbench::getSettings()
{
    Settings::coord[X].checkSoftLimits = checkSoftX->isChecked();


    Settings::coord[X].softLimitMin = doubleXmin->value();
    Settings::coord[X].softLimitMax = doubleXmax->value();
    Settings::coord[X].workAreaMin = doubleRangeMinX->value();
    Settings::coord[X].workAreaMax = doubleRangeMaxX->value();


    Settings::coord[Y].checkSoftLimits = checkSoftY->isChecked();


    Settings::coord[Y].softLimitMin = doubleYmin->value();
    Settings::coord[Y].softLimitMax = doubleYmax->value();
    Settings::coord[Y].workAreaMin = doubleRangeMinY->value();
    Settings::coord[Y].workAreaMax = doubleRangeMaxY->value();


    Settings::coord[Z].checkSoftLimits = checkSoftZ->isChecked();

    Settings::coord[Z].softLimitMin = doubleZmin->value();
    Settings::coord[Z].softLimitMax = doubleZmax->value();
    Settings::coord[Z].workAreaMin = doubleRangeMinZ->value();
    Settings::coord[Z].workAreaMax = doubleRangeMaxZ->value();

    Settings::coord[A].checkSoftLimits = checkSoftA->isChecked();


    Settings::coord[A].softLimitMin = doubleAmin->value();
    Settings::coord[A].softLimitMax = doubleAmax->value();
    Settings::coord[A].workAreaMin = doubleRangeMinA->value();
    Settings::coord[A].workAreaMax = doubleRangeMaxA->value();

    Settings::DEMO_DEVICE  = checkBoxDemoController->isChecked();
}

void SettingsWorkbench::translateWidget()
{
    labelMin->setText(translate(ID_MIN));
    labelMax->setText(translate(ID_MAX));

    labelSeq->setText(translate(ID_SEQUENCE));
    labelSpeed->setText(translate(ID_SPEED));
    labelPosition->setText(translate(ID_POS));

    checkBoxDemoController->setText(translate(ID_DEV_SIMULATION));


}

#if 0
void SettingsWorkbench::checkedChanged( int state)
{
    bool check = checkCorrecture->isChecked();
    groupOffset->setEnabled(check);
    groupResize->setEnabled(check);
}


void SettingsWorkbench::onSaveChange()
{
    if (checkCorrecture->isChecked()) {
        parent->deltaX = doubleSpinOffsetX->value();
        parent->deltaY = doubleSpinOffsetY->value();
        parent->deltaZ = doubleSpinOffsetZ->value();

        parent->deltaFeed = checkBoxZ->isChecked();

        parent->coeffSizeX = doubleSpinResizeX->value();
        parent->coeffSizeY = doubleSpinResizeY->value();
    }

    emit accept();
}
#endif
