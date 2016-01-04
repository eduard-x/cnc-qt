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
#include <QUrl>


#include "includes/Settings3d.h"


/******************************************************************************
** Settings3dDialog
*/


Settings3dDialog::Settings3dDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    glParent = static_cast<GLWidget*>(parent->scene3d);

    setStyleSheet(parent->programStyleSheet);

    connect(pushButton, SIGNAL(clicked()), this, SLOT(onSave()));

    radioButtonLines->setChecked(parent->ShowLines);
    radioButtonPoints->setChecked(parent->ShowPoints);

    checkBoxInstr->setChecked(parent->ShowInstrument);
    groupBoxGrid->setChecked(parent->ShowGrid);
    checkBoxSurface->setChecked(parent->ShowSurface);
    checkBoxXYZ->setChecked(parent->ShowAxes);

    spinBoxGrid->setValue(parent->GrigStep);

    spinBoxBeginX->setValue(parent->GridXstart);
    spinBoxEndX->setValue(parent->GridXend);
    spinBoxBeginY->setValue(parent->GridYstart);
    spinBoxEndY->setValue(parent->GridYend);

    groupBoxShowRang->setChecked(parent->ShowGrate);
    spinBoxMinX->setValue(parent->cnc->coord[X].softLimitMin);
    spinBoxMaxX->setValue(parent->cnc->coord[X].softLimitMax);
    spinBoxMinY->setValue(parent->cnc->coord[Y].softLimitMin);
    spinBoxMaxY->setValue(parent->cnc->coord[Y].softLimitMax);

    translateDialog();

    adjustSize();
}


void Settings3dDialog::translateDialog()
{
    setWindowTitle(translate(_SETTINGS3D_TITLE));

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
}


void Settings3dDialog::onSave()
{
    parent->ShowInstrument = checkBoxInstr->isChecked();
    parent->ShowGrid = groupBoxGrid->isChecked();
    parent->ShowSurface = checkBoxSurface->isChecked();
    parent->ShowAxes = checkBoxXYZ->isChecked();

    parent->GrigStep = spinBoxGrid->value();

    parent->GridXstart = spinBoxBeginX->value();
    parent->GridXend = spinBoxEndX->value();
    parent->GridYstart = spinBoxBeginY->value();
    parent->GridYend = spinBoxEndY->value();

    parent->ShowGrate = groupBoxShowRang->isChecked();
    parent->ShowLines = radioButtonLines->isChecked();
    parent->ShowPoints = radioButtonPoints->isChecked();
    parent->cnc->coord[X].softLimitMin = spinBoxMinX->value();
    parent->cnc->coord[X].softLimitMax = spinBoxMaxX->value();
    parent->cnc->coord[Y].softLimitMin = spinBoxMinY->value();
    parent->cnc->coord[Y].softLimitMax = spinBoxMaxY->value();

    emit accept();
}