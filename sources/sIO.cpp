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

#include <QtGui>


#include "includes/sIO.h"

// class Settings;
/******************************************************************************
** SettingsIO
*/


SettingsIO::SettingsIO(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    //     setStyleSheet(parent->programStyleSheet);



    connect(comboConnect1, SIGNAL(currentIndexChanged ( int )), this, SLOT(onChangeConnector(int)));
    connect(comboConnect2, SIGNAL(currentIndexChanged ( int )), this, SLOT(onChangeConnector(int)));
    connect(comboConnect3, SIGNAL(currentIndexChanged ( int )), this, SLOT(onChangeConnector(int)));
    connect(comboConnect4, SIGNAL(currentIndexChanged ( int )), this, SLOT(onChangeConnector(int)));


    checkXmin->setChecked(Settings::coord[X].useLimitMin);
    checkXplus->setChecked(Settings::coord[X].useLimitMax);
    checkYmin->setChecked(Settings::coord[Y].useLimitMin);
    checkYplus->setChecked(Settings::coord[Y].useLimitMax);
    checkZmin->setChecked(Settings::coord[Z].useLimitMin);
    checkZplus->setChecked(Settings::coord[Z].useLimitMax);
    checkAmin->setChecked(Settings::coord[A].useLimitMin);
    checkAplus->setChecked(Settings::coord[A].useLimitMax);

    checkInvertSwitchXmin->setChecked(Settings::coord[X].invLimitMin);
    checkInvertSwitchXplu->setChecked(Settings::coord[X].invLimitMax);
    checkInvertSwitchYmin->setChecked(Settings::coord[Y].invLimitMin);
    checkInvertSwitchYplu->setChecked(Settings::coord[Y].invLimitMax);
    checkInvertSwitchZmin->setChecked(Settings::coord[Z].invLimitMin);
    checkInvertSwitchZplu->setChecked(Settings::coord[Z].invLimitMax);
    checkInvertSwitchAmin->setChecked(Settings::coord[A].invLimitMin);
    checkInvertSwitchAplu->setChecked(Settings::coord[A].invLimitMax);

    // set the connector names
    comboConnect1->setCurrentIndex( Settings::coord[X].connector);
    comboConnect2->setCurrentIndex( Settings::coord[Y].connector);
    comboConnect3->setCurrentIndex( Settings::coord[Z].connector);
    comboConnect4->setCurrentIndex( Settings::coord[A].connector);

    emit onChangeConnector(0);

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

SettingsIO::~SettingsIO()
{
}

void SettingsIO::getSettings()
{

    Settings::coord[X].useLimitMin = checkXmin->isChecked();
    Settings::coord[X].useLimitMax = checkXplus->isChecked();
    Settings::coord[Y].useLimitMin = checkYmin->isChecked();
    Settings::coord[Y].useLimitMax = checkYplus->isChecked();
    Settings::coord[Z].useLimitMin = checkZmin->isChecked();
    Settings::coord[Z].useLimitMax = checkZplus->isChecked();
    Settings::coord[A].useLimitMin = checkAmin->isChecked();
    Settings::coord[A].useLimitMax = checkAplus->isChecked();

    Settings::coord[X].invLimitMin = checkInvertSwitchXmin->isChecked();
    Settings::coord[X].invLimitMax = checkInvertSwitchXplu->isChecked();
    Settings::coord[Y].invLimitMin = checkInvertSwitchYmin->isChecked();
    Settings::coord[Y].invLimitMax = checkInvertSwitchYplu->isChecked();
    Settings::coord[Z].invLimitMin = checkInvertSwitchZmin->isChecked();
    Settings::coord[Z].invLimitMax = checkInvertSwitchZplu->isChecked();
    Settings::coord[A].invLimitMin = checkInvertSwitchAmin->isChecked();
    Settings::coord[A].invLimitMax = checkInvertSwitchAplu->isChecked();

    Settings::coord[X].connector = comboConnect1->currentIndex();
    Settings::coord[Y].connector = comboConnect2->currentIndex();
    Settings::coord[Z].connector = comboConnect3->currentIndex();
    Settings::coord[A].connector = comboConnect4->currentIndex();
}


/**
 * @brief change connector of axis 0..3 for X..A
 */
void SettingsIO::onChangeConnector(int i)
{
    QComboBox* cmbArr[] = {comboConnect1, comboConnect2, comboConnect3, comboConnect4};
    QComboBox* c = static_cast<QComboBox*>(sender());

}


void SettingsIO::translateWidget()
{
    //     setWindowTitle(translate(_EDITGCODE_TITLE));
    //     checkCorrecture->setText(translate(_CORRECTURE));
    //     groupResize->setTitle(translate(_PROPORTION));
    //     groupOffset->setTitle(translate(_OFFSET_GCODE));
    //     checkBoxZ->setText(translate(_CORRECT_Z));
}

