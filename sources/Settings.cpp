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

#include <QColorDialog>


#include "includes/mk1Controller.h"
#include "includes/Settings.h"



axis::axis()
{
    acceleration = 50.0;
    actualLimitMax = false;
    actualLimitMin = false;
    enabled = true;
    backlash = 0.0;
    invertDirection = false;
    invertPulses = false;
    invLimitMax = false;
    invLimitMin = false;
    workAreaMax = 100.0;
    workAreaMin = -100.0;
    pulsePerMm = 200;
    actualPosPulses = 0;
    wrong = false;
}


float axis::posMm()
{
    if (pulsePerMm != 0) {
        return (float)(actualPosPulses / (float) pulsePerMm);
    } else {
        return 0.0;
    }
}


int axis::posPulse(float posMm)
{
    return (int)(posMm * (float)pulsePerMm);
}


QChar Settings::toDecimalPoint = '.';
QChar Settings::fromDecimalPoint = ';';
bool Settings::DEMO_DEVICE = false;
int  Settings::splitsPerMm = 10;
float Settings::maxLookaheadAngle = 170.0;

byte Settings::bb14 = 0x0;
byte Settings::bb19 = 0x0;

colorGL Settings::colorSettings[COLOR_LINES];


axis Settings::coord[] = { axis(), axis(), axis(), axis() };


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

    connect(toolSelectColor, SIGNAL (clicked()), this, SLOT(changeColor()));

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

    spinArcSplitPermm->setValue(Settings::splitsPerMm);
    spinBoxLookLines->setValue(Settings::maxLookaheadAngle);

    checkSoftX->setChecked(Settings::coord[X].checkSoftLimits);
    checkSwapX->setChecked(Settings::coord[X].invertDirection);
    checkUseX->setChecked(Settings::coord[X].enabled);
    backlashX->setValue(Settings::coord[X].backlash);
    doubleXmin->setValue(Settings::coord[X].softLimitMin);
    doubleXmax->setValue(Settings::coord[X].softLimitMax);
    doubleRangeMinX->setValue(Settings::coord[X].workAreaMin);
    doubleRangeMaxX->setValue(Settings::coord[X].workAreaMax);
    checkInvStepsX->setChecked(Settings::coord[X].invertPulses);

    checkSoftY->setChecked(Settings::coord[Y].checkSoftLimits);
    checkSwapY->setChecked(Settings::coord[Y].invertDirection);
    checkUseY->setChecked(Settings::coord[Y].enabled);
    backlashY->setValue(Settings::coord[Y].backlash);
    doubleYmin->setValue(Settings::coord[Y].softLimitMin);
    doubleYmax->setValue(Settings::coord[Y].softLimitMax);
    doubleRangeMinY->setValue(Settings::coord[Y].workAreaMin);
    doubleRangeMaxY->setValue(Settings::coord[Y].workAreaMax);
    checkInvStepsY->setChecked(Settings::coord[Y].invertPulses);

    checkSoftZ->setChecked(Settings::coord[Z].checkSoftLimits);
    checkSwapZ->setChecked(Settings::coord[Z].invertDirection);
    checkUseZ->setChecked(Settings::coord[Z].enabled);
    backlashZ->setValue(Settings::coord[Z].backlash);
    doubleZmin->setValue(Settings::coord[Z].softLimitMin);
    doubleZmax->setValue(Settings::coord[Z].softLimitMax);
    doubleRangeMinZ->setValue(Settings::coord[Z].workAreaMin);
    doubleRangeMaxZ->setValue(Settings::coord[Z].workAreaMax);
    checkInvStepsZ->setChecked(Settings::coord[Z].invertPulses);

    checkSoftA->setChecked(Settings::coord[A].checkSoftLimits);
    checkSwapA->setChecked(Settings::coord[A].invertDirection);
    checkUseA->setChecked(Settings::coord[A].enabled);
    backlashA->setValue(Settings::coord[A].backlash);
    doubleAmin->setValue(Settings::coord[A].softLimitMin);
    doubleAmax->setValue(Settings::coord[A].softLimitMax);
    doubleRangeMinA->setValue(Settings::coord[A].workAreaMin);
    doubleRangeMaxA->setValue(Settings::coord[A].workAreaMax);
    checkInvStepsA->setChecked(Settings::coord[A].invertPulses);

    numPulseX->setValue(Settings::coord[X].pulsePerMm);
    numPulseY->setValue(Settings::coord[Y].pulsePerMm);
    numPulseZ->setValue(Settings::coord[Z].pulsePerMm);
    numPulseA->setValue(Settings::coord[A].pulsePerMm);

    doubleSpinStartX->setValue(Settings::coord[X].minVelo);
    doubleSpinStartY->setValue(Settings::coord[Y].minVelo);
    doubleSpinStartZ->setValue(Settings::coord[Z].minVelo);
    doubleSpinStartA->setValue(Settings::coord[A].minVelo);

    doubleSpinEndX->setValue(Settings::coord[X].maxVelo);
    doubleSpinEndY->setValue(Settings::coord[Y].maxVelo);
    doubleSpinEndZ->setValue(Settings::coord[Z].maxVelo);
    doubleSpinEndA->setValue(Settings::coord[A].maxVelo);

    doubleSpinAccelX->setValue(Settings::coord[X].acceleration);
    doubleSpinAccelY->setValue(Settings::coord[Y].acceleration);
    doubleSpinAccelZ->setValue(Settings::coord[Z].acceleration);
    doubleSpinAccelA->setValue(Settings::coord[A].acceleration);

    checkBoxDemoController->setChecked(Settings::DEMO_DEVICE);

    // visualisation settings
    radioButtonLines->setChecked(parent->ShowLines);
    radioButtonPoints->setChecked(parent->ShowPoints);

    checkBoxInstr->setChecked(parent->ShowInstrument);
    groupBoxGrid->setChecked(parent->ShowGrid);
    checkBoxSurface->setChecked(parent->ShowSurface);
    checkBoxXYZ->setChecked(parent->ShowAxes);

    //     checkDisableIfSSH->setChecked(parent->disableIfSSH);
    spinBoxGrid->setValue(parent->GrigStep);

    spinBoxBeginX->setValue(parent->GridXstart);
    spinBoxEndX->setValue(parent->GridXend);
    spinBoxBeginY->setValue(parent->GridYstart);
    spinBoxEndY->setValue(parent->GridYend);

    groupBoxShowRang->setChecked(parent->ShowGrate);
    spinBoxMinX->setValue(Settings::coord[X].softLimitMin);
    spinBoxMaxX->setValue(Settings::coord[X].softLimitMax);
    spinBoxMinY->setValue(Settings::coord[Y].softLimitMin);
    spinBoxMaxY->setValue(Settings::coord[Y].softLimitMax);
    //end of visualisation settings

    translateDialog();

    adjustSize();
}


void SettingsDialog::changeColor()
{
    int num = comboColor->currentIndex();

    if (num < COLOR_LINES) {
        colorGL glc = Settings::colorSettings[num];
        QColor clr(glc.r * 255.0, glc.g * 255.0, glc.b * 255.0, glc.a * 255.0);
        
        clr = QColorDialog::getColor ( clr, this ) ;

        if (clr.isValid()) {
            Settings::colorSettings[num] = (colorGL) {
                clr.red() / (float)255.0, clr.green() / (float)255.0, clr.blue() / (float)255.0, clr.alpha() / (float)255.0
            };
        }
    }
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
    fList << translate(_VISUALISATION);

    listWidget->addItems(fList);

    groupBoxColors->setTitle(translate(_COLORS));

    QStringList colorList = translate(_COLOR_LIST).split("\n");
    comboColor->addItems(colorList);

    int width = listWidget->sizeHintForColumn(0);
    listWidget->setFixedWidth(width + 20);

    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onSelection(QListWidgetItem*)));
    //     connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSelection(QListWidgetItem*)));

    listWidget->item(0)->setSelected(true);

    tabWidget->setCurrentIndex(0);
    //     tabWidget->tabBar()->setFixedHeight(0);
    //     tabWidget->tabBar()->setMaximumWidth(500);
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

    // visualisation translations
    //     checkDisableIfSSH->setText(translate(_DISABLE_VISUALISATION));
    groupBoxGrid->setTitle(translate(_DISPLAY_GRID));
    radioButtonLines->setText(translate(_DISPLAY_LINES));
    radioButtonPoints->setText(translate(_DISPLAY_POINTS));
    labelBeg->setText(translate(_BEGIN));
    labelEnd->setText(translate(_END));

    checkBoxInstr->setText(translate(_DISPLAY_SPINDLE));
    checkBoxXYZ->setText(translate(_DISPLAY_AXES));
    checkBoxSurface->setText(translate(_DISPLAY_SURFACE));

    groupBoxShowRang->setTitle(translate(_DISPLAY_RANG));

    labelStep->setText(translate(_STEP));
    labelMin->setText(translate(_MINIMUM));
    labelMax->setText(translate(_MAXIMUM));
    // end
}


void SettingsDialog::onSelection(QListWidgetItem* it)
{
    int idx = listWidget->currentRow();
    //     qDebug() << idx;
    tabWidget->setCurrentIndex(idx);
}


void SettingsDialog::onSave()
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

    Settings::coord[X].checkSoftLimits = checkSoftX->isChecked();
    Settings::coord[X].invertDirection = checkSwapX->isChecked();
    Settings::coord[X].invertPulses = checkInvStepsX->isChecked();
    Settings::coord[X].enabled = checkUseX->isChecked();
    Settings::coord[X].backlash = backlashX->value();
    Settings::coord[X].softLimitMin = doubleXmin->value();
    Settings::coord[X].softLimitMax = doubleXmax->value();
    Settings::coord[X].workAreaMin = doubleRangeMinX->value();
    Settings::coord[X].workAreaMax = doubleRangeMaxX->value();

    Settings::coord[Y].checkSoftLimits = checkSoftY->isChecked();
    Settings::coord[Y].invertDirection = checkSwapY->isChecked();
    Settings::coord[Y].invertPulses = checkInvStepsY->isChecked();
    Settings::coord[Y].enabled = checkUseY->isChecked();
    Settings::coord[Y].backlash = backlashY->value();
    Settings::coord[Y].softLimitMin = doubleYmin->value();
    Settings::coord[Y].softLimitMax = doubleYmax->value();
    Settings::coord[Y].workAreaMin = doubleRangeMinY->value();
    Settings::coord[Y].workAreaMax = doubleRangeMaxY->value();

    Settings::coord[Z].checkSoftLimits = checkSoftZ->isChecked();
    Settings::coord[Z].invertDirection = checkSwapZ->isChecked();
    Settings::coord[Z].invertPulses = checkInvStepsZ->isChecked();
    Settings::coord[Z].backlash = backlashZ->value();
    Settings::coord[Z].enabled = checkUseZ->isChecked();
    Settings::coord[Z].softLimitMin = doubleZmin->value();
    Settings::coord[Z].softLimitMax = doubleZmax->value();
    Settings::coord[Z].workAreaMin = doubleRangeMinZ->value();
    Settings::coord[Z].workAreaMax = doubleRangeMaxZ->value();

    Settings::coord[A].checkSoftLimits = checkSoftA->isChecked();
    Settings::coord[A].invertDirection = checkSwapA->isChecked();
    Settings::coord[A].invertPulses = checkInvStepsA->isChecked();
    Settings::coord[A].backlash = backlashA->value();
    Settings::coord[A].enabled = checkUseA->isChecked();
    Settings::coord[A].softLimitMin = doubleAmin->value();
    Settings::coord[A].softLimitMax = doubleAmax->value();
    Settings::coord[A].workAreaMin = doubleRangeMinA->value();
    Settings::coord[A].workAreaMax = doubleRangeMaxA->value();

    Settings::coord[X].pulsePerMm = numPulseX->value();
    Settings::coord[Y].pulsePerMm = numPulseY->value();
    Settings::coord[Z].pulsePerMm = numPulseZ->value();
    Settings::coord[A].pulsePerMm = numPulseA->value();

    Settings::coord[X].minVelo = doubleSpinStartX->value();
    Settings::coord[Y].minVelo = doubleSpinStartY->value();
    Settings::coord[Z].minVelo = doubleSpinStartZ->value();
    Settings::coord[A].minVelo = doubleSpinStartA->value();

    Settings::coord[X].maxVelo = doubleSpinEndX->value();
    Settings::coord[Y].maxVelo = doubleSpinEndY->value();
    Settings::coord[Z].maxVelo = doubleSpinEndZ->value();
    Settings::coord[A].maxVelo = doubleSpinEndA->value();

    Settings::coord[X].acceleration = doubleSpinAccelX->value();
    Settings::coord[Y].acceleration = doubleSpinAccelY->value();
    Settings::coord[Z].acceleration = doubleSpinAccelZ->value();
    Settings::coord[A].acceleration = doubleSpinAccelA->value();

    Settings::splitsPerMm = spinArcSplitPermm->value();
    Settings::maxLookaheadAngle = spinBoxLookLines->value();

    Settings::DEMO_DEVICE  = checkBoxDemoController->isChecked();

    // visualisation settings
    parent->ShowInstrument = checkBoxInstr->isChecked();
    parent->ShowGrid = groupBoxGrid->isChecked();
    parent->ShowSurface = checkBoxSurface->isChecked();
    parent->ShowAxes = checkBoxXYZ->isChecked();

    //     parent->disableIfSSH = checkDisableIfSSH->isChecked();
    parent->GrigStep = spinBoxGrid->value();

    parent->GridXstart = spinBoxBeginX->value();
    parent->GridXend = spinBoxEndX->value();
    parent->GridYstart = spinBoxBeginY->value();
    parent->GridYend = spinBoxEndY->value();

    parent->ShowGrate = groupBoxShowRang->isChecked();
    parent->ShowLines = radioButtonLines->isChecked();
    parent->ShowPoints = radioButtonPoints->isChecked();
    Settings::coord[X].softLimitMin = spinBoxMinX->value();
    Settings::coord[X].softLimitMax = spinBoxMaxX->value();
    Settings::coord[Y].softLimitMin = spinBoxMinY->value();
    Settings::coord[Y].softLimitMax = spinBoxMaxY->value();
    // end

    accept();
}

