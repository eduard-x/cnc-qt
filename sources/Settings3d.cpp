/****************************************************************************
 * C++ Implementation:                                                      *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
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
    spinBoxMinX->setValue(parent->grateXmin);
    spinBoxMaxX->setValue(parent->grateXmax);
    spinBoxMinY->setValue(parent->grateYmin);
    spinBoxMaxY->setValue(parent->grateYmax);

    translateDialog();

    adjustSize();
}


void Settings3dDialog::translateDialog()
{
    setWindowTitle(translate(_SETTINGS3D_TITLE));

    groupBoxGrid->setTitle(translate(_DISPLAY_GRID));
    labelBeg->setText(translate(_BEGIN));
    labelEnd->setText(translate(_END));

    checkBoxInstr->setText(translate(_DISPLAY_SPINDLE));
    checkBoxXYZ->setText(translate(_DISPLAY_AXES));
    checkBoxSurface->setText(translate(_DISPLAY_SURFACE));

    groupBoxShowRang->setTitle(translate(_DISPLAY_RANG));

    labelStep->setText(translate(_DISPLAY_SPINDLE));
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
    parent->grateXmin = spinBoxMinX->value();
    parent->grateXmax = spinBoxMaxX->value();
    parent->grateYmin = spinBoxMinY->value();
    parent->grateYmax = spinBoxMaxY->value();

    emit accept();
}