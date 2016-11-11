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


#include "includes/sMaterial.h"


/******************************************************************************
** SettingsMaterial
*/


SettingsMaterial::SettingsMaterial(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    graphicsView->setStyleSheet("background: transparent");

    //     setStyleSheet(parent->programStyleSheet);

    // pictures of
    for (int i = 1 ; i < 13; i++) {
        QString name;
        name = QString().sprintf(":/images/frz%02d.png", i);
        frz_png <<  QPixmap(name);
    }

    grph = 0;



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

    emit onChangeTool(0);

    adjustSize();
}



/**
 * @brief change the selection for color
 */
void SettingsMaterial::onChangeTool(int i)
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


void SettingsMaterial::getSettings()
{
}

void SettingsMaterial::translateWidget()
{
    // tool settings

    labelFlutes->setText(translate(_FLUTES));
    labelDiameter->setText(translate(_DIAMETER));
    labelShaft->setText(translate(_SHAFT));
    labelDiam->setText(translate(_DIAMETER));
    labelTool->setText(translate(_SELECT_TOOL));

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

    labelMaterial->setText(translate(_MATERIAL));

    comboBoxTool->addItems(cmbList);
    connect(comboBoxTool, SIGNAL (activated(int)), this, SLOT(onChangeTool(int)));

    // end of tool settings
}

