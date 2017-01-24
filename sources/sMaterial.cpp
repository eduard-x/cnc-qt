/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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


class Settings;
/******************************************************************************
** SettingsMaterial
*/
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

/**
 * TODO
 * @see https://www.onsrud.com/xdoc/FeedSpeeds
 *
 */
void SettingsMaterial::initMaterialList()
{
    materialList = (QVector<materialFeed>()
    << (materialFeed) {
        // material, min Feed, max Feed, max RPM, coeff fz diameter
        HARDWOOD,    60.0, 150.0, 23000, 0.035
    }
    << (materialFeed) {
        SOFTWOOD,    80.0, 250.0, 18000, 0.04
    }
    << (materialFeed) {
        PLYWOOD,     80.0, 250.0, 18000, 0.04
    }
    << (materialFeed) {
        MDF,         80.0, 250.0, 23000, 0.04
    }
    << (materialFeed) {
        ACRYLIC,     100.0, 150.0, 20000, 0.035
    }
    << (materialFeed) {
        PHENOLIC,    100.0, 200.0, 20000, 0.035
    }
    << (materialFeed) {
        FIBERGLASS,  100.0, 150.0, 20000, 0.035
    } // polyacril
    << (materialFeed) {
        HARDPLASTIC, 150.0, 350.0, 18000, 0.035
    }
    << (materialFeed) {
        SOFTPLASTIC, 200.0, 400.0, 20000, 0.04
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



SettingsMaterial::SettingsMaterial(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    //     parent = static_cast<MainWindow*>(p);

    //     graphicsView->setStyleSheet("background: transparent");

    //     setStyleSheet(parent->programStyleSheet);

    // pictures of
    for (int i = 1 ; i < 13; i++) {
        QString name;
        name = QString().sprintf(":/images/frz%02d.png", i);
        frz_png <<  QPixmap(name);
    }

    grph = 0;


    //  parent = static_cast<MainWindow*>(p);

    //     setStyleSheet(parent->programStyleSheet);

    initMaterialList();

    current = Settings::cuttedMaterial;

    doubleSpinFeedRate->setReadOnly(true);

    //     connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    //     connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));

    connect(doubleSpinDiameter, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(spinFlutes, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));
    connect(doubleSpinCutting, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    //     connect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    connect(doubleSpinSpindleSpeed, SIGNAL(valueChanged(int)), this, SLOT(changeParameters(void)));
    connect(doubleSpinChipLoad, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));

    connect(comboUnit, SIGNAL(activated(int)), this, SLOT(changeUnit(int)));

    graphicsView->setStyleSheet("background: transparent");

    //     translateWidget();

    emit changeUnit( (Settings::unitMm == true) ? 0 : 1);

    emit changeMaterial((int)Settings::cuttedMaterial);

    translateWidget();

    emit onChangeTool(0);

    adjustSize();
}


SettingsMaterial::~SettingsMaterial()
{
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



void SettingsMaterial::changeParameters(void)
{
    QObject* s = sender();

    //     if (s == doubleSpinCutting) {
    //         disconnect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    //         doubleSpinZ->setValue(doubleSpinCutting->value() / 3.0);
    //         connect(doubleSpinZ, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    //     }

    //     if (s == doubleSpinZ) {
    //         disconnect(doubleSpinCutting, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    //         doubleSpinCutting->setValue(doubleSpinZ->value() * 3.0);
    //         connect(doubleSpinCutting, SIGNAL(valueChanged(double)), this, SLOT(changeParameters(void)));
    //     }

    //     float rpm;
    z = spinFlutes->value();
    v = doubleSpinCutting->value();
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


void SettingsMaterial::changeMaterial(int n)
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

    labelCuttingRange->setText(translate(ID_RANGES) + unit);

    float m = (materialList.at(n).maxFeedXY + materialList.at(n).minFeedXY) / 2.0;

    doubleSpinCutting->setMinimum(minFeedXY);
    doubleSpinCutting->setMaximum(maxFeedXY);
    doubleSpinCutting->setValue(m);

    //     doubleSpinZ->setMinimum(minFeedXY / 3.0);
    //     doubleSpinZ->setMaximum(maxFeedXY / 3.0);
    //     doubleSpinZ->setValue(m / 3.0);

    comboMaterial->setCurrentIndex(current);

    connect(comboMaterial, SIGNAL(activated(int)), this, SLOT(changeMaterial(int)));
}


void SettingsMaterial::changeUnit(int n)
{
    QString unit;

    if (n == 0) { // mm
        scaling = 1.0;
        unit = QString().sprintf(": %4.1f - %4.1f m/min", materialList.at(n).minFeedXY, materialList.at(n).maxFeedXY);
    } else { // inch
        scaling = 25.4;
        unit = QString().sprintf(": %4.1f - %4.1f feet/min", (materialList.at(n).minFeedXY / 3.28084), (materialList.at(n).maxFeedXY / 3.28084));
    }

    labelCuttingRange->setText(translate(ID_RANGES) + unit);

    if (Settings::toolDiameter == 0) {
        Settings::toolDiameter = 3.0;
    }

    if (scaling != 0.0) {
        doubleSpinDiameter->setValue(Settings::toolDiameter / scaling);
    }

    spinFlutes->setValue(Settings::toolFlutes);
    doubleSpinSpindleSpeed->setValue(Settings::toolRPM);
}


void SettingsMaterial::getSettings()
{
    Settings::unitMm = (scaling == 1.0);
    Settings::toolDiameter = doubleSpinDiameter->value();
    Settings::toolFlutes = spinFlutes->value();
    Settings::toolRPM = doubleSpinSpindleSpeed->value();

    Settings::veloCutting = v;

    Settings::cuttedMaterial = current;
}

void SettingsMaterial::translateWidget()
{
    // tool settings

    labelUnit->setText(translate(ID_MESSURE_UNIT));
    groupMaterial->setTitle(translate(ID_MATERIAL));
    groupTool->setTitle(translate(ID_TOOL));
    labelDiameter->setText(translate(ID_DIAMETER));
    labelFlutes->setText(translate(ID_FLUTES));
    labelCuttingSpeed->setText(translate(ID_CUTTING_SPEED));
    labelCuttingRange->setText(translate(ID_RANGES));
    labelDepth->setText(translate(ID_MAX_DEPTH));
    labelRange->setText(translate(ID_RANGE));

    labelRangeSpindle->setText("");
    labelRangeChipLoad->setText("");

    labelSpendleSpeed->setText(translate(ID_SPINDLE_SPEED));
    labelChipLoad->setText(translate(ID_CHIPLOAD));
    labelFeedRate->setText(translate(ID_FEED_RATE));

    QString units = translate(ID_UNITS);
    QStringList u = units.split("\n");
    comboUnit->addItems(u);

    QString materials = translate(ID_MATERIAL_LIST);
    QStringList m = materials.split("\n");
    comboMaterial->addItems(m);

    //     labelFlutes->setText(translate(_FLUTES));
    //     labelDiameter->setText(translate(_DIAMETER));
    labelShaft->setText(translate(ID_SHAFT));
    labelDiam->setText(translate(ID_DIAMETER));
    labelTool->setText(translate(ID_SELECT_TOOL));

    QString tblText = translate(ID_TOOL_TABLE);
    QStringList tTable = tblText.split("\\");

    toolArray.clear();
    QStringList cmbList;

    foreach (QString s, tTable) {
        QStringList slst = s.split("\t");

        if(slst.count() < 4) {
            continue;
        }

        cmbList << slst.at(2);
        slst[0] = translate(ID_DIAMETER) + ": " + slst.at(0);
        slst[1] = translate(ID_SHAFT) + ": " + slst.at(1);
        slst[3] = slst.at(3).simplified();
        toolArray << slst.toVector();
    }

    labelMaterial->setText(translate(ID_MATERIAL));

    comboBoxTool->addItems(cmbList);
    connect(comboBoxTool, SIGNAL (activated(int)), this, SLOT(onChangeTool(int)));

    // end of tool settings
}

