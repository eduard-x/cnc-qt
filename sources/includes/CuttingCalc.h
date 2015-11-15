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


#ifndef CUTTINGCALC_H
#define CUTTINGCALC_H

#include <QVector>
#include <QString>

#include "MainWindow.h"
#include "ui_CuttingCalc.h"



// material feed
// MPM = SFM / 3.281
struct materialFeed {
    MATERIAL m;
    float minFeedXY;
    float maxFeedXY;
    float minFeedZ;
    float maxFeedZ;
};

// class MainWindow;


class CuttingCalc : public QDialog, public Ui::CuttingCalcDialog,  public cTranslator
{
        Q_OBJECT
    public:
        CuttingCalc(QWidget *parent = 0);

    private slots:
        void checkedChanged(int state);
        void changeUnit(int n);
        void changeParameters(void);
        void onSave();
        void changeMaterial(int i);

    private:
        void translateDialog();
        static materialFeed materialList[];

    private:
        MainWindow* parent;
        float scaling;
};


#endif // EDITCODE_H