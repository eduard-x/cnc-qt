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


#include "includes/CuttingCalc.h"


/******************************************************************************
** CuttingCalcDialog
*/

//
// calculation of cutting deep:
// max cutting deep = diameter * 4
// source http://forum.jdpaintrus.ru/topic/40-vybor-frez-dlia-raznykh-materialov-vybor-shpindel/


//
// http://www.gravman.ru/speed.htm
// http://www.gravman.ru/realspeed.htm
// http://agranat-avia.ru/raschet-rezhimov-frezerovaniya
// tables: http://www.pdsspindles.com/engineering-speeds
// tables: http://www.harveytool.com/cms/GeneralMachiningGuidelines_17.aspx
// https://en.wikipedia.org/wiki/Speeds_and_feeds
// http://www.cncroutershop.com/us_en/calculate-feeds

//
// data for 3mm diameter
// z feed is the 1/3 from xy feed
//
materialFeed CuttingCalc::materialList[] = {
    {HARDWOOD,    150.0, 350.0},
    {SOFTWOOD, 300.0, 500.0},
    {PLYWOOD, 300.0, 400.0},
    {MDF, 0.1, 0.18},
    {ACRYLIC,     0.003, 0.005},
    {PHENOLIC,    0.004, 0.005},
    {FIBERGLASS,  0.003, 0.005},
    {HARDPLASTIC, 150.0, 300.0},
    {SOFTPLASTIC, 300.0, 400.0},
    {BRONZE,     100.0, 120.0},
    {ALUMINIUM,    80.0, 100.0},
    {COPPER,     24.0, 45.0}
};


CuttingCalc::CuttingCalc(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    setStyleSheet(parent->programStyleSheet);

    doubleSpinFeedRate->setReadOnly(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));

    connect(doubleSpinDiameter, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinFlutes, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinXY, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinSpindleSpeed, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinChipLoad, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));

    connect(comboUnit, SIGNAL(activated(int)), this, SLOT(changeUnit(int)));

    translateDialog();

    emit changeUnit( (parent->unitMm == true)?0:1);

    emit changeMaterial(parent->cuttedMaterial);

    adjustSize();
}


void CuttingCalc::onSave()
{
    parent->unitMm = (scaling == 1.0);

    emit accept();
}


void CuttingCalc::changeParameters(void)
{
}


void CuttingCalc::changeMaterial(int n)
{
    
    emit changeParameters();
}


void CuttingCalc::changeUnit(int n)
{
    if (n == 0) { // mm
        scaling = 1.0;
    } else { // inch
        scaling = 25.4;
    }
}


void CuttingCalc::translateDialog()
{
    setWindowTitle(translate(_CALC_TITLE));

    labelUnit->setText(translate(_MESSURE_UNIT));
    groupMaterial->setTitle(translate(_MATERIAL));
    groupTool->setTitle(translate(_TOOL));
    labelDiameter->setText(translate(_DIAMETER));
    labelFlutes->setText(translate(_FLUTES));
    labelCuttingSpeed->setText(translate(_CUTTING_SPEED));
    labelCuttingRange->setText(translate(_RANGES));
    labelDepth->setText(translate(_MAX_DEPTH));
    labelRange->setText(translate(_RANGE));

    labelRangeSpindle->setText("");
    labelRangeChipLoad->setText("");

    labelSpendleSpeed->setText(translate(_SPINDLE_SPEED));
    labelChipLoad->setText(translate(_CHIPLOAD));
    labelFeedRate->setText(translate(_FEED_RATE));

    QString units = translate(_UNITS);
    QStringList u = units.split("\n");
    comboUnit->addItems(u);

    QString materials = translate(_MATERIAL_LIST);
    QStringList m = materials.split("\n");
    comboMaterial->addItems(m);
}


void CuttingCalc::checkedChanged( int state)
{
    //     bool check = checkCorrecture->isChecked();
    //     groupOffset->setEnabled(check);
    //     groupResize->setEnabled(check);
}


