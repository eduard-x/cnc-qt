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

    QStringList seqList = (QStringList() << "1" << "2" << "3" << "4");
    comboSeqX->addItems(seqList);
    comboSeqY->addItems(seqList);
    comboSeqZ->addItems(seqList);
    comboSeqA->addItems(seqList);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    checkXmin->setChecked(cnc->coord[X].useLimitMin);
    checkXplus->setChecked(cnc->coord[X].useLimitMax);
    checkYmin->setChecked(cnc->coord[Y].useLimitMin);
    checkYplus->setChecked(cnc->coord[Y].useLimitMax);
    checkZmin->setChecked(cnc->coord[Z].useLimitMin);
    checkZplus->setChecked(cnc->coord[Z].useLimitMax);
    checkAmin->setChecked(cnc->coord[A].useLimitMin);
    checkAplus->setChecked(cnc->coord[A].useLimitMax);

    checkInvertSwitchXmin->setChecked(cnc->coord[X].invLimitMin);
    checkInvertSwitchXplu->setChecked(cnc->coord[X].invLimitMax);
    checkInvertSwitchYmin->setChecked(cnc->coord[Y].invLimitMin);
    checkInvertSwitchYplu->setChecked(cnc->coord[Y].invLimitMax);
    checkInvertSwitchZmin->setChecked(cnc->coord[Z].invLimitMin);
    checkInvertSwitchZplu->setChecked(cnc->coord[Z].invLimitMax);
    checkInvertSwitchAmin->setChecked(cnc->coord[A].invLimitMin);
    checkInvertSwitchAplu->setChecked(cnc->coord[A].invLimitMax);

    spinBoxLookLines->setValue(splitsPerMm);
    spinBoxLookLines->setValue(maxLookaheadAngle);

    checkSoftX->setChecked(cnc->coord[X].checkSoftLimits);
    checkSwapX->setChecked(cnc->coord[X].invertDirection);
    checkUseX->setChecked(cnc->coord[X].enabled);
    backlashX->setValue(cnc->coord[X].backlash);
    doubleXmin->setValue(cnc->coord[X].softLimitMin);
    doubleXmax->setValue(cnc->coord[X].softLimitMax);
    doubleRangeMinX->setValue(cnc->coord[X].workAreaMin);
    doubleRangeMaxX->setValue(cnc->coord[X].workAreaMax);
    checkInvStepsX->setChecked(cnc->coord[X].invertPulses);

    checkSoftY->setChecked(cnc->coord[Y].checkSoftLimits);
    checkSwapY->setChecked(cnc->coord[Y].invertDirection);
    checkUseY->setChecked(cnc->coord[Y].enabled);
    backlashY->setValue(cnc->coord[Y].backlash);
    doubleYmin->setValue(cnc->coord[Y].softLimitMin);
    doubleYmax->setValue(cnc->coord[Y].softLimitMax);
    doubleRangeMinY->setValue(cnc->coord[Y].workAreaMin);
    doubleRangeMaxY->setValue(cnc->coord[Y].workAreaMax);
    checkInvStepsY->setChecked(cnc->coord[Y].invertPulses);

    checkSoftZ->setChecked(cnc->coord[Z].checkSoftLimits);
    checkSwapZ->setChecked(cnc->coord[Z].invertDirection);
    checkUseZ->setChecked(cnc->coord[Z].enabled);
    backlashZ->setValue(cnc->coord[Z].backlash);
    doubleZmin->setValue(cnc->coord[Z].softLimitMin);
    doubleZmax->setValue(cnc->coord[Z].softLimitMax);
    doubleRangeMinZ->setValue(cnc->coord[Z].workAreaMin);
    doubleRangeMaxZ->setValue(cnc->coord[Z].workAreaMax);
    checkInvStepsZ->setChecked(cnc->coord[Z].invertPulses);

    checkSoftA->setChecked(cnc->coord[A].checkSoftLimits);
    checkSwapA->setChecked(cnc->coord[A].invertDirection);
    checkUseA->setChecked(cnc->coord[A].enabled);
    backlashA->setValue(cnc->coord[A].backlash);
    doubleAmin->setValue(cnc->coord[A].softLimitMin);
    doubleAmax->setValue(cnc->coord[A].softLimitMax);
    doubleRangeMinA->setValue(cnc->coord[A].workAreaMin);
    doubleRangeMaxA->setValue(cnc->coord[A].workAreaMax);
    checkInvStepsA->setChecked(cnc->coord[A].invertPulses);

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
    groupRanges->setTitle(translate(_LIMITS));
    checkBoxDemoController->setText(translate(_DEV_SIMULATION));
    labelInfo->setText(translate(_DEV_SIM_HELP));
    labelUse->setText(translate(_USE));
    labelMin->setText(translate(_MIN));
    labelMax->setText(translate(_MAX));
    labelSwap->setText(translate(_SWAP));
    labelSeq->setText(translate(_SEQUENCE));
    labelSpeed->setText(translate(_SPEED));
    labelPosition->setText(translate(_POS));

    QStringList fList;
    fList << translate(_WORKTABLE);
    fList << translate(_SPEED);
    fList << translate(_LIMITS);
    fList << translate(_PARKING);
    fList << translate(_IO);
    fList << translate(_TOOL);
    fList << translate(_LOOKAHEAD);
    fList << translate(_ARC_SPLITTING);

    listWidget->addItems(fList);

    int width = listWidget->sizeHintForColumn(0);
    listWidget->setFixedWidth(width + 20);

    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onSelection(QListWidgetItem*)));
    //     connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSelection(QListWidgetItem*)));

    listWidget->item(0)->setSelected(true);

    tabWidget->setCurrentIndex(0);
    tabWidget->tabBar()->setFixedHeight(0);
    tabWidget->tabBar()->setMaximumWidth(500);
    tabWidget->setStyleSheet("QTabBar::tab { height: 0px; width: 0px; border: 0px solid #333; }" );

    //     tabWidget->setTabText(0, translate(_WORKTABLE));
    //     tabWidget->setTabText(1, translate(_VELO));
    //     tabWidget->setTabText(2, translate(_LIMITS));
    //     tabWidget->setTabText(3, translate(_PARKING));
    //     tabWidget->setTabText(4, translate(_TOOL));

    labelStart->setText(translate(_STARTVELO));
    labelEnd->setText(translate(_ENDVELO));
    labelAcceleration->setText(translate(_ACCELERATION));

    QList<QAbstractButton*> l = buttonBox->buttons();
    QStringList strl = (QStringList() << translate(_SET) << translate(_CANCEL));

    for(int i = 0; i < l.count(); i++) {
        l[i]->setText(strl.at(i));
    }
}


void SettingsDialog::onSelection(QListWidgetItem* it)
{
    int idx = listWidget->currentRow();
    //     qDebug() << idx;
    tabWidget->setCurrentIndex(idx);
}


void SettingsDialog::onSave()
{
    cnc->coord[X].useLimitMin = checkXmin->isChecked();
    cnc->coord[X].useLimitMax = checkXplus->isChecked();
    cnc->coord[Y].useLimitMin = checkYmin->isChecked();
    cnc->coord[Y].useLimitMax = checkYplus->isChecked();
    cnc->coord[Z].useLimitMin = checkZmin->isChecked();
    cnc->coord[Z].useLimitMax = checkZplus->isChecked();
    cnc->coord[A].useLimitMin = checkAmin->isChecked();
    cnc->coord[A].useLimitMax = checkAplus->isChecked();

    cnc->coord[X].invLimitMin = checkInvertSwitchXmin->isChecked();
    cnc->coord[X].invLimitMax = checkInvertSwitchXplu->isChecked();
    cnc->coord[Y].invLimitMin = checkInvertSwitchYmin->isChecked();
    cnc->coord[Y].invLimitMax = checkInvertSwitchYplu->isChecked();
    cnc->coord[Z].invLimitMin = checkInvertSwitchZmin->isChecked();
    cnc->coord[Z].invLimitMax = checkInvertSwitchZplu->isChecked();
    cnc->coord[A].invLimitMin = checkInvertSwitchAmin->isChecked();
    cnc->coord[A].invLimitMax = checkInvertSwitchAplu->isChecked();

    cnc->coord[X].checkSoftLimits = checkSoftX->isChecked();
    cnc->coord[X].invertDirection = checkSwapX->isChecked();
    cnc->coord[X].invertPulses = checkInvStepsX->isChecked();
    cnc->coord[X].enabled = checkUseX->isChecked();
    cnc->coord[X].backlash = backlashX->value();
    cnc->coord[X].softLimitMin = doubleXmin->value();
    cnc->coord[X].softLimitMax = doubleXmax->value();
    cnc->coord[X].workAreaMin = doubleRangeMinX->value();
    cnc->coord[X].workAreaMax = doubleRangeMaxX->value();

    cnc->coord[Y].checkSoftLimits = checkSoftY->isChecked();
    cnc->coord[Y].invertDirection = checkSwapY->isChecked();
    cnc->coord[Y].invertPulses = checkInvStepsY->isChecked();
    cnc->coord[Y].enabled = checkUseY->isChecked();
    cnc->coord[Y].backlash = backlashY->value();
    cnc->coord[Y].softLimitMin = doubleYmin->value();
    cnc->coord[Y].softLimitMax = doubleYmax->value();
    cnc->coord[Y].workAreaMin = doubleRangeMinY->value();
    cnc->coord[Y].workAreaMax = doubleRangeMaxY->value();

    cnc->coord[Z].checkSoftLimits = checkSoftZ->isChecked();
    cnc->coord[Z].invertDirection = checkSwapZ->isChecked();
    cnc->coord[Z].invertPulses = checkInvStepsZ->isChecked();
    cnc->coord[Z].backlash = backlashZ->value();
    cnc->coord[Z].enabled = checkUseZ->isChecked();
    cnc->coord[Z].softLimitMin = doubleZmin->value();
    cnc->coord[Z].softLimitMax = doubleZmax->value();
    cnc->coord[Z].workAreaMin = doubleRangeMinZ->value();
    cnc->coord[Z].workAreaMax = doubleRangeMaxZ->value();

    cnc->coord[A].checkSoftLimits = checkSoftA->isChecked();
    cnc->coord[A].invertDirection = checkSwapA->isChecked();
    cnc->coord[A].invertPulses = checkInvStepsA->isChecked();
    cnc->coord[A].backlash = backlashA->value();
    cnc->coord[A].enabled = checkUseA->isChecked();
    cnc->coord[A].softLimitMin = doubleAmin->value();
    cnc->coord[A].softLimitMax = doubleAmax->value();
    cnc->coord[A].workAreaMin = doubleRangeMinA->value();
    cnc->coord[A].workAreaMax = doubleRangeMaxA->value();

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

