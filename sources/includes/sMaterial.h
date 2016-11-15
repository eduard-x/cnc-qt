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


#ifndef SMATERIAL_H
#define SMATERIAL_H


#include <QVector>
#include <QString>

#include "MainWindow.h"
#include "ui_sMaterial.h"
#include "Settings.h"


class MainWindow;



// material feed
// MPM = SFM / 3.281
struct materialFeed {
    MATERIAL m;
    float minFeedXY;
    float maxFeedXY;
    int maxRPM;
    float kfz; // koeff fz for diameter
};



class SettingsMaterial : public QWidget, public Ui::sMaterial,  public cTranslator
{
        Q_OBJECT
    public:
        SettingsMaterial(QWidget *parent = 0);
        void getSettings();

    private slots:
        void onChangeTool(int i);
        void changeUnit(int n);
        void changeParameters(void);
        //         void onSave();
        void changeMaterial(int i);


    private:
        void translateWidget();
        void initMaterialList();

    private:
        QVector<QPixmap> frz_png;
        QVector< QVector<QString> > toolArray;
        QVector<materialFeed> materialList;
        QGraphicsScene *grph;
        float scaling;
        float v;
        float d;
        int rpm;
        float feed;
        float fz;
        int z;
        MATERIAL current;
        //         MainWindow* parent;
};


#endif // SMATERIAL_H