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

void CuttingCalc::initMaterialList()
{
    materialList = (QVector<materialFeed>()
    << (materialFeed) {
        HARDWOOD,    60.0, 150.0, 10000, 0.035
    }
    << (materialFeed) {
        SOFTWOOD,    80.0, 250.0, 10000, 0.04
    }
    << (materialFeed) {
        PLYWOOD,     80.0, 250.0, 10000, 0.04
    }
    << (materialFeed) {
        MDF,         80.0, 250.0, 10000, 0.04
    }
    << (materialFeed) {
        ACRYLIC,     100.0, 150.0, 10000, 0.035
    }
    << (materialFeed) {
        PHENOLIC,    100.0, 200.0, 10000, 0.035
    }
    << (materialFeed) {
        FIBERGLASS,  100.0, 150.0, 10000, 0.035
    } // polyacril
    << (materialFeed) {
        HARDPLASTIC, 150.0, 350.0, 10000, 0.035
    }
    << (materialFeed) {
        SOFTPLASTIC, 200.0, 400.0, 10000, 0.04
    }
    << (materialFeed) {
        BRONZE,      30.0, 60.0, 10000, 0.0085
    }
    << (materialFeed) {
        ALUMINIUM,   70.0, 100.0, 10000, 0.01
    }
    << (materialFeed) {
        COPPER,      50.0, 100.0, 10000, 0.008
    });
}


CuttingCalc::CuttingCalc(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    setStyleSheet(parent->programStyleSheet);

    initMaterialList();
    
    current = parent->cuttedMaterial;

    doubleSpinFeedRate->setReadOnly(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));

    connect(doubleSpinDiameter, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinFlutes, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));
    connect(doubleSpinXY, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinSpindleSpeed, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));
    connect(doubleSpinChipLoad, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));

    connect(comboUnit, SIGNAL(activated(int)), this, SLOT(changeUnit(int)));

    translateDialog();

    emit changeUnit( (parent->unitMm == true) ? 0 : 1);

    emit changeMaterial((int)parent->cuttedMaterial);

    adjustSize();
}


void CuttingCalc::onSave()
{
    parent->unitMm = (scaling == 1.0);
    parent->toolDiameter = doubleSpinDiameter->value();
    parent->toolFlutes = doubleSpinFlutes->value();
    parent->toolRPM = doubleSpinSpindleSpeed->value();

    parent->veloCutting = v;

    parent->cuttedMaterial = current;

    emit accept();
}


void CuttingCalc::changeParameters(void)
{
    QObject* s = sender();

    if (s == doubleSpinXY) {
        disconnect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
        doubleSpinZ->setValue(doubleSpinXY->value() / 3.0);
        connect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    }

    if (s == doubleSpinZ) {
        disconnect(doubleSpinXY, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
        doubleSpinXY->setValue(doubleSpinZ->value() * 3.0);
        connect(doubleSpinXY, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    }

    //     float rpm;
    z = doubleSpinFlutes->value();
    v = doubleSpinXY->value();
    d = doubleSpinDiameter->value();
    rpm = 1000.0 * v / (PI * d);

    if (rpm > materialList.at(current).maxRPM) {
        rpm = materialList.at(current).maxRPM;
    }

    if (s == doubleSpinDiameter) {
        disconnect(doubleSpinChipLoad, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
        fz = d * materialList.at(current).kfz;
        doubleSpinChipLoad->setValue(d * materialList.at(current).kfz);
        connect(doubleSpinChipLoad, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    }

    disconnect(doubleSpinSpindleSpeed, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));
    doubleSpinSpindleSpeed->setValue(rpm);
    connect(doubleSpinSpindleSpeed, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));

    feed = fz * rpm * z;

    doubleSpinFeedRate->setValue(feed);
}


void CuttingCalc::changeMaterial(int n)
{
    if (n >= materialList.size()) {
        return;
    }

    current = (MATERIAL)n;

    disconnect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));

    QString unit;
    float minFeedXY = materialList.at(n).minFeedXY;
    float maxFeedXY = materialList.at(n).maxFeedXY;

    if (scaling == 1.0) {
        unit = QString().sprintf(": %4.1f - %4.1f m/min", minFeedXY, maxFeedXY);
    } else {
        minFeedXY = materialList.at(n).minFeedXY / 3.28084;
        maxFeedXY = materialList.at(n).maxFeedXY / 3.28084;
        unit = QString().sprintf(": %4.1f - %4.1f feet/min", minFeedXY, maxFeedXY);
    }

    labelCuttingRange->setText(translate(_RANGES) + unit);

    float m = (materialList.at(n).maxFeedXY + materialList.at(n).minFeedXY) / 2.0;

    doubleSpinXY->setMinimum(minFeedXY);
    doubleSpinXY->setMaximum(maxFeedXY);
    doubleSpinXY->setValue(m);

    doubleSpinZ->setMinimum(minFeedXY / 3.0);
    doubleSpinZ->setMaximum(maxFeedXY / 3.0);
    doubleSpinZ->setValue(m / 3.0);

    comboMaterial->setCurrentIndex(current);

    connect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));
}


void CuttingCalc::changeUnit(int n)
{
    QString unit;

    if (n == 0) { // mm
        scaling = 1.0;
        unit = QString().sprintf(": %4.1f - %4.1f m/min", materialList.at(n).minFeedXY, materialList.at(n).maxFeedXY);
    } else { // inch
        scaling = 25.4;
        unit = QString().sprintf(": %4.1f - %4.1f feet/min", (materialList.at(n).minFeedXY / 3.28084), (materialList.at(n).maxFeedXY / 3.28084));
    }

    labelCuttingRange->setText(translate(_RANGES) + unit);

    if (parent->toolDiameter == 0) {
        parent->toolDiameter = 3.0;
    }

    if (scaling != 0.0) {
        doubleSpinDiameter->setValue(parent->toolDiameter / scaling);
    }

    doubleSpinFlutes->setValue(parent->toolFlutes);
    doubleSpinSpindleSpeed->setValue(parent->toolRPM);
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



