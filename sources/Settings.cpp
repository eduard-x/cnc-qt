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

int Settings::pointSize = 1;
int Settings::lineWidth = 3;
bool Settings::smoothMoving = false;
bool Settings::showTraverse = false;
bool Settings::showWorkbench = false;
bool Settings::filterRepeat = true;

byte Settings::bb14 = 0x0;
byte Settings::bb19 = 0x0;

QColor Settings::colorSettings[COLOR_LINES];


axis Settings::coord[] = { axis(), axis(), axis(), axis() };


/******************************************************************************
** SettingsDialog
*/


SettingsDialog::SettingsDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    cnc = parent->mk1;

    // pictures of
    for (int i = 1 ; i < 13; i++) {
        QString name;
        name = QString().sprintf(":/images/frz%02d.png", i);
        frz_png <<  QPixmap(name);
    }

    graphicsView->setStyleSheet("background: transparent");

    setStyleSheet(parent->programStyleSheet);

    QStringList seqList = (QStringList() << "1" << "2" << "3" << "4");
    comboSeqX->addItems(seqList);
    comboSeqY->addItems(seqList);
    comboSeqZ->addItems(seqList);
    comboSeqA->addItems(seqList);

    grpArr.clear();
    grpArr << (QVector <QGroupBox*>() << groupRanges << groupHome << groupSoftwareLimits); // workbench
    grpArr << (QVector <QGroupBox*>() << groupSpeed); // speed
    grpArr << (QVector <QGroupBox*>() << groupHardwareLimits << groupOutput << groupJog << groupExtPin); // I/O
    grpArr << (QVector <QGroupBox*>() << groupBacklash << groupLookahead); // system
    grpArr << (QVector <QGroupBox*>() << groupBoxArc); // parser
    grpArr << (QVector <QGroupBox*>() << groupTool << groupMaterial << groupCalc); // tool
    grpArr << (QVector <QGroupBox*>() << groupViewing << groupBoxColors << groupBoxGrid << groupBoxShowRang); // 3d
    grpArr << (QVector <QGroupBox*>() << groupRemote << groupKeyboard << groupJoypad); // control

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

    checkBoxDemoController->setChecked(Settings::DEMO_DEVICE);

    checkBoxRemove->setChecked(Settings::filterRepeat);
    // visualisation settings
    radioButtonLines->setChecked(parent->ShowLines);
    radioButtonPoints->setChecked(parent->ShowPoints);

    checkBoxTool->setChecked(parent->ShowInstrument);
    groupBoxGrid->setChecked(parent->ShowGrid);
    checkBoxSurface->setChecked(parent->ShowSurface);
    checkBoxXYZ->setChecked(parent->ShowAxes);

    //     checkDisableIfSSH->setChecked(parent->disableIfSSH);
    spinBoxGrid->setValue(parent->GrigStep);

    spinBoxBeginX->setValue(parent->GridXstart);
    spinBoxEndX->setValue(parent->GridXend);
    spinBoxBeginY->setValue(parent->GridYstart);
    spinBoxEndY->setValue(parent->GridYend);

    groupBoxShowRang->setChecked(parent->ShowBorder);
    spinBoxMinX->setValue(Settings::coord[X].softLimitMin);
    spinBoxMaxX->setValue(Settings::coord[X].softLimitMax);
    spinBoxMinY->setValue(Settings::coord[Y].softLimitMin);
    spinBoxMaxY->setValue(Settings::coord[Y].softLimitMax);

    //     checkBoxCommand->setValue(Settings::
    checkBoxWorkbench->setChecked(Settings::showWorkbench);
    checkBoxTraverse->setChecked(Settings::showTraverse);
    checkBoxSmooth->setChecked(Settings::smoothMoving);
    spinBoxPointSize->setValue(Settings::pointSize);
    spinBoxLineWidth->setValue(Settings::lineWidth);
    //end of visualisation settings

    grph = 0;

    translateDialog();

    emit onChangeColor(0);

    emit onChangeTool(0);

    adjustSize();
}


/**
 *
 */
void SettingsDialog::onChangeTool(int i)
{
    if (i >= frz_png.count()) {
        qDebug() << "picture vector is too small";
        return;
    }

    if (grph != NULL) {
        delete grph;
    }

    grph = new QGraphicsScene();
    QGraphicsPixmapItem *item_p = grph->addPixmap(frz_png.at(i));

    item_p->setVisible(true);

    graphicsView->setScene(grph);

    textBrowser->setText(toolArray[i][3]);

    labelDiam->setText(toolArray[i][0]);
    labelShaft->setText(toolArray[i][1]);
}


/**
 *
 */
void SettingsDialog::changeColor()
{
    int num = comboColor->currentIndex();

    if (num < COLOR_LINES) {
        QColor glc = Settings::colorSettings[num];
        QColor clr = glc;// (glc.r * 255.0, glc.g * 255.0, glc.b * 255.0, glc.a * 255.0);

        clr = QColorDialog::getColor ( clr, this ) ;

        if (clr.isValid()) {
            Settings::colorSettings[num] = clr;

            emit onChangeColor(num);
        }
    }
}


void SettingsDialog::translateDialog()
{
    setWindowTitle(translate(_SETTINGS_TITLE));
//     groupRanges->setTitle(translate(_LIMITS));
    checkBoxDemoController->setText(translate(_DEV_SIMULATION));
    //     labelInfo->setText(translate(_DEV_SIM_HELP));
    labelUse->setText(translate(_USE));
    labelMin->setText(translate(_MIN));
    labelMax->setText(translate(_MAX));
    labelSwap->setText(translate(_SWAP));
    labelSeq->setText(translate(_SEQUENCE));
    labelSpeed->setText(translate(_SPEED));
    labelPosition->setText(translate(_POS));

//     groupLookahead->setTitle(translate(_LOOKAHEAD));

    labelFlutes->setText(translate(_FLUTES));
    labelDiameter->setText(translate(_DIAMETER));
    labelShaft->setText(translate(_SHAFT));
    labelDiam->setText(translate(_DIAMETER));
    labelTool->setText(translate(_SELECT_TOOL));

//     groupBoxArc->setTitle(translate(_ARC_SPLITTING));


    // tool settings

    QString tblText = translate(_TOOL_TABLE);
    QStringList tTable = tblText.split("\\");

    toolArray.clear();
    QStringList cmbList;

    foreach (QString s, tTable) {
        QStringList slst = s.split("\t");

        if(slst.count() < 4) {
            continue;
        }

        cmbList << slst.at(2);
        slst[0] = translate(_DIAMETER) + ": " + slst.at(0);
        slst[1] = translate(_SHAFT) + ": " + slst.at(1);
        slst[3] = slst.at(3).simplified();
        toolArray << slst.toVector();
    }

    comboBoxTool->addItems(cmbList);
    connect(comboBoxTool, SIGNAL (activated(int)), this, SLOT(onChangeTool(int)));

    // end of tool settings

    //    menu items

    QStringList menuList;
    menuList << translate(_WORKBENCH);
    menuList << translate(_SPEED);
    menuList << translate(_IO);
    menuList << translate(_SYSTEM);
    menuList << translate(_PARSER);
    menuList << translate(_WORK_TOOL);
    menuList << translate(_VISUALISATION);
    menuList << translate(_CONTROL);


    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;

    foreach (QString s, menuList) {
        QStringList sub = s.split(";");
        QVector <QString> v = sub.toVector();
        menuArr << v;
        QTreeWidgetItem *m = new QTreeWidgetItem(treeWidget, QStringList(sub.at(0)));
        items.append(m);

        if (sub.count() > 1) {
            sub.removeAt(0);

            foreach (QString ssub, sub) {
                items.append(new QTreeWidgetItem(m, QStringList(ssub)));
            }

            m->setExpanded(true);
        }
    }

    treeWidget->header()->close();

    for (int i = 0; i < menuArr.count(); i++) {
        if ((grpArr.at(i).count() == menuArr.at(i).count() -1)) {
            for (int j = 0; j < menuArr.at(i).count() -1; j++) {
                ((QGroupBox*)(grpArr.at(i).at(j)))->setTitle(menuArr.at(i).at(j+1));
            }
        }
    }

//       grpArr << (QVector <QGroupBox*>() << groupRanges << groupHome << groupSoftwareLimits); // workbench
//     grpArr << (QVector <QGroupBox*>() << groupSpeed); // speed
//     grpArr << (QVector <QGroupBox*>() << groupHardwareLimits << groupOutput << groupJog << groupExtPin); // I/O
//     grpArr << (QVector <QGroupBox*>() << groupBacklash << groupLookahead); // system
//     grpArr << (QVector <QGroupBox*>() << groupBoxArc); // parser
//     grpArr << (QVector <QGroupBox*>() << groupTool << groupMaterial << groupCalc); // tool
//     grpArr << (QVector <QGroupBox*>() << groupViewing << groupBoxColors << groupBoxGrid << groupBoxShowRang); // 3d
//     grpArr << (QVector <QGroupBox*>() << groupRemote << groupKeyboard << groupJoypad); // control
    int width = treeWidget->sizeHint().width();
    treeWidget->setFixedWidth(width);

    connect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));

    treeWidget->setCurrentItem( items.at(0));
    // end of menu items

//     groupBoxColors->setTitle(translate(_COLORS));

    checkBoxRemove->setText(translate(_REMOVE_REPEAT));

    QStringList colorList = translate(_COLOR_LIST).split("\n");
    comboColor->addItems(colorList);

    connect(comboColor, SIGNAL(activated(int)), this, SLOT(onChangeColor(int)));

    tabWidget->setStyleSheet("QTabBar::tab { height: 0px; width: 0px; border: 0px solid #333; }" );


    labelStart->setText(translate(_STARTVELO));
    labelEnd->setText(translate(_ENDVELO));
    labelAcceleration->setText(translate(_ACCELERATION));

    labelMaterial->setText(translate(_MATERIAL));

    QList<QAbstractButton*> l = buttonBox->buttons();
    QStringList strl = (QStringList() << translate(_SET) << translate(_CANCEL));

    for(int i = 0; i < l.count(); i++) {
        l[i]->setText(strl.at(i));
    }

    // visualisation translations
    //     checkDisableIfSSH->setText(translate(_DISABLE_VISUALISATION));
//     groupBoxGrid->setTitle(translate(_DISPLAY_GRID));
    radioButtonLines->setText(translate(_DISPLAY_LINES));
    radioButtonPoints->setText(translate(_DISPLAY_POINTS));
    labelBeg->setText(translate(_BEGIN));
    labelEnd->setText(translate(_END));

    checkBoxTool->setText(translate(_DISPLAY_SPINDLE));
    checkBoxXYZ->setText(translate(_DISPLAY_AXES));
    checkBoxSurface->setText(translate(_DISPLAY_SURFACE));

    checkBoxCommand->setText(translate(_DISPLAY_COMMAND));
    checkBoxWorkbench->setText(translate(_DISPLAY_WORKBENCH));
    checkBoxTraverse->setText(translate(_DISPLAY_TRAVERSE));
    checkBoxSmooth->setText(translate(_SMOOTH_MOVING));
    labelPoint->setText(translate(_POINT_SIZE));
    labelLine->setText(translate(_LINE_WIDTH));

//     groupBoxShowRang->setTitle(translate(_DISPLAY_RANG));

    labelStep->setText(translate(_STEP));
    labelMin->setText(translate(_MINIMUM));
    labelMax->setText(translate(_MAXIMUM));
    // end
}


void SettingsDialog::onSelection(QTreeWidgetItem* it, QTreeWidgetItem * ip)
{
    QString mainText;
    QString childText;

    disconnect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));

    if (it->parent() != NULL) {
        mainText =  it->parent()->text(0);
        childText = it->text(0);
//         qDebug() << mainText << childText;
    } else {
        mainText = it->text(0);
//         qDebug() << mainText;
    }

    for (int idxRow = 0; idxRow < menuArr.count(); idxRow++) {
        if (menuArr.at(idxRow).at(0) == mainText) { // selected element was found
            tabWidget->setCurrentIndex(idxRow);

            // display selected widget
            if (childText.length() > 0) {
                for (int idxCol = 1; idxCol < menuArr.at(idxRow).count(); ++idxCol) {
                    // check the sizing of arrays
                    if ((grpArr.count() == menuArr.count()) && (grpArr.at(idxRow).count() == menuArr.at(idxRow).count() - 1)) {
                        if (childText == menuArr.at(idxRow).at(idxCol)) {
                            grpArr.at(idxRow).at(idxCol - 1)->setHidden(false);
                        } else {
                            grpArr.at(idxRow).at(idxCol - 1)->setHidden(true);
                        }
                    }
                }
            } else { // display all widgets
                for (int idxCol = 1; idxCol < menuArr.at(idxRow).count(); ++idxCol) {
                    // check the sizing of arrays
                    if ((grpArr.count() == menuArr.count()) && (grpArr.at(idxRow).count() == menuArr.at(idxRow).count() - 1)) {
                        grpArr.at(idxRow).at(idxCol - 1)->setHidden(false);
                    }
                }
            }

            break;
        }
    }

    connect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));
}


void SettingsDialog::onChangeColor(int i)
{
    toolSelectColor->setStyleSheet(QString("background-color: %1; ").arg(Settings::colorSettings[i].name()));
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

    Settings::splitsPerMm = spinArcSplitPermm->value();
    Settings::maxLookaheadAngle = spinBoxLookLines->value();

    Settings::DEMO_DEVICE  = checkBoxDemoController->isChecked();

    // visualisation settings
    parent->ShowInstrument = checkBoxTool->isChecked();
    parent->ShowGrid = groupBoxGrid->isChecked();
    parent->ShowSurface = checkBoxSurface->isChecked();
    parent->ShowAxes = checkBoxXYZ->isChecked();

    Settings::filterRepeat = checkBoxRemove->isChecked();

    //     parent->disableIfSSH = checkDisableIfSSH->isChecked();
    parent->GrigStep = spinBoxGrid->value();

    parent->GridXstart = spinBoxBeginX->value();
    parent->GridXend = spinBoxEndX->value();
    parent->GridYstart = spinBoxBeginY->value();
    parent->GridYend = spinBoxEndY->value();

    parent->ShowBorder = groupBoxShowRang->isChecked();
    parent->ShowLines = radioButtonLines->isChecked();
    parent->ShowPoints = radioButtonPoints->isChecked();
    Settings::coord[X].softLimitMin = spinBoxMinX->value();
    Settings::coord[X].softLimitMax = spinBoxMaxX->value();
    Settings::coord[Y].softLimitMin = spinBoxMinY->value();
    Settings::coord[Y].softLimitMax = spinBoxMaxY->value();

    //     checkBoxCommand->setValue(Settings::
    Settings::showWorkbench = checkBoxWorkbench->isChecked();
    Settings::showTraverse = checkBoxTraverse->isChecked();
    Settings::smoothMoving = checkBoxSmooth->isChecked();
    Settings::pointSize = spinBoxPointSize->value();
    Settings::lineWidth = spinBoxLineWidth->value();

    // end

    accept();
}

