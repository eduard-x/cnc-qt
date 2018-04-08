/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2018 by Eduard Kalinowski                             *
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


#include "includes/sParser.h"
#include "includes/Settings.h"

class Settings;
/******************************************************************************
** SettingsParser
*/


SettingsParser::SettingsParser(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    QStringList ls;

    for (int i = 0; i < 10; i++) {
        ls << QString::number(i);
    }

    comboBoxTimes->addItems(ls);

    spinArcSplitPermm->setValue(Settings::splitsPerMm);

    checkBoxRemove->setChecked(Settings::filterRepeat);
    groupBoxOptimize->setChecked(Settings::optimizeRapidWays);
    groupBoxRepeat->setChecked(Settings::repeatProcessing);
    spinBoxMaxDepth->setValue(Settings::maxAntSearchDepth);

    int n;
    n = comboBoxTimes->findText(QString::number(Settings::repeatTimes));

    if (n >= 0) {
        comboBoxTimes->setCurrentIndex(n);
    } else {
        comboBoxTimes->setCurrentIndex(0);
    }

    ls.removeAt(0);
    comboBoxDepth->addItems(ls);
    n = comboBoxDepth->findText(QString::number(Settings::depthSum));

    if (n >= 0) {
        comboBoxDepth->setCurrentIndex(n);
    } else {
        comboBoxDepth->setCurrentIndex(0);
    }

    translateWidget();

    adjustSize();
}

SettingsParser::~SettingsParser()
{
}
/**
 *
 *
 */
void SettingsParser::getSettings()
{
    Settings::splitsPerMm = spinArcSplitPermm->value();
    Settings::filterRepeat = checkBoxRemove->isChecked();
    Settings::depthSum = comboBoxDepth->currentText().toInt();
    Settings::repeatTimes = comboBoxTimes->currentText().toInt();
    Settings::optimizeRapidWays = groupBoxOptimize->isChecked();
    Settings::repeatProcessing = groupBoxRepeat->isChecked();
    Settings::maxAntSearchDepth = spinBoxMaxDepth->value();
}

/**
 *
 *
 */
void SettingsParser::translateWidget()
{
    checkBoxRemove->setText(translate(ID_REMOVE_REPEAT));

    groupBoxRepeat->setTitle(translate(ID_REPEAT_CODE));
    labelRepeat->setText(translate(ID_NUM_REPEAT));
    labelDepth->setText(translate(ID_DEPTH_SUM));
    groupBoxOptimize->setTitle(translate(ID_OPTIMIZE_RAPID_WAYS));
    labelMaxDepthAnt->setText(translate(ID_MAX_DEPTH_OPTIMIZE));
}

