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


#include "includes/sVis.h"


class Settings;

/******************************************************************************
** SettingsVis
*/


SettingsVis::SettingsVis(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    //     setStyleSheet(parent->programStyleSheet);


    connect(toolSelectColor, SIGNAL (clicked()), this, SLOT(changeColor()));

    checkBoxTool->setChecked(Settings::ShowInstrument);
    groupBoxGrid->setChecked(Settings::ShowGrid);
    checkBoxSurface->setChecked(Settings::ShowSurface);
    checkBoxXYZ->setChecked(Settings::ShowAxes);

    //     checkDisableIfSSH->setChecked(Settings::disableIfSSH);
    spinBoxGrid->setValue(Settings::GrigStep);

    spinBoxBeginX->setValue(Settings::GridXstart);
    spinBoxEndX->setValue(Settings::GridXend);
    spinBoxBeginY->setValue(Settings::GridYstart);
    spinBoxEndY->setValue(Settings::GridYend);

    groupBoxShowRang->setChecked(Settings::ShowBorder);
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

    // visualisation settings
    radioButtonLines->setChecked(Settings::ShowLines);
    radioButtonPoints->setChecked(Settings::ShowPoints);


    //     connect(checkCorrecture, SIGNAL(stateChanged ( int)), this, SLOT(checkedChanged( int )));
    //
    //     checkCorrecture->setChecked(true);
    //     checkCorrecture->setChecked(false); // toggle
    //
    //     connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSaveChange()));
    //     connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //
    //     doubleSpinOffsetX->setValue(Settings::deltaX);
    //     doubleSpinOffsetY->setValue(Settings::deltaY);
    //     doubleSpinOffsetZ->setValue(Settings::deltaZ);
    //
    //     checkBoxZ->setChecked(Settings::deltaFeed);
    //
    //     doubleSpinResizeX->setValue(Settings::coeffSizeX);
    //     doubleSpinResizeY->setValue(Settings::coeffSizeY);

    //     checkResizeZ->setChecked();

    translateWidget();

    emit onChangeColor(0);

    adjustSize();
}


void SettingsVis::getSettings()
{
    // visualisation settings
    Settings::ShowInstrument = checkBoxTool->isChecked();
    Settings::ShowGrid = groupBoxGrid->isChecked();
    Settings::ShowSurface = checkBoxSurface->isChecked();
    Settings::ShowAxes = checkBoxXYZ->isChecked();



    //     Settings::disableIfSSH = checkDisableIfSSH->isChecked();
    Settings::GrigStep = spinBoxGrid->value();

    Settings::GridXstart = spinBoxBeginX->value();
    Settings::GridXend = spinBoxEndX->value();
    Settings::GridYstart = spinBoxBeginY->value();
    Settings::GridYend = spinBoxEndY->value();

    Settings::ShowBorder = groupBoxShowRang->isChecked();
    Settings::ShowLines = radioButtonLines->isChecked();
    Settings::ShowPoints = radioButtonPoints->isChecked();
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
}


/**
 * @brief after changing of color the color of button to change
 *
 */
void SettingsVis::onChangeColor(int i)
{
    toolSelectColor->setStyleSheet(QString("background-color: %1; ").arg(Settings::colorSettings[i].name()));
}



/**
 *
 */
void SettingsVis::changeColor()
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


void SettingsVis::translateWidget()
{
    // visualisation translations
    QStringList colorList = translate(_COLOR_LIST).split("\n");
    comboColor->addItems(colorList);

    connect(comboColor, SIGNAL(activated(int)), this, SLOT(onChangeColor(int)));

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

#if 0
void SettingsVis::checkedChanged( int state)
{
    bool check = checkCorrecture->isChecked();
    groupOffset->setEnabled(check);
    groupResize->setEnabled(check);
}


void SettingsVis::onSaveChange()
{
    if (checkCorrecture->isChecked()) {
        Settings::deltaX = doubleSpinOffsetX->value();
        Settings::deltaY = doubleSpinOffsetY->value();
        Settings::deltaZ = doubleSpinOffsetZ->value();

        Settings::deltaFeed = checkBoxZ->isChecked();

        Settings::coeffSizeX = doubleSpinResizeX->value();
        Settings::coeffSizeY = doubleSpinResizeY->value();
    }

    emit accept();
}
#endif
