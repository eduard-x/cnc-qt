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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QColor>
#include <QGraphicsScene>
#include <QVector>
#include <QPixmap>

#include "MainWindow.h"
#include "ui_Settings.h"

#include "Settings.h"

#include "sControl.h"
#include "sMaterial.h"
#include "sWorkbench.h"
#include "sSpeed.h"
#include "sSystem.h"
#include "sParser.h"
#include "sVis.h"
#include "sIO.h"

class MainWindow;

class SettingsParser;
class SettingsControl;
class SettingsMaterial;
class SettingsSystem;
class SettingsWorkbench;
class SettingsSpeed;
class SettingsVis;
class SettingsIO;


class SettingsDialog : public QDialog, public Ui::SettingsDialog,  public cTranslator
{
        Q_OBJECT
    public:
        SettingsDialog(QWidget *parent = 0, int tabNum = 0);


    private slots:
        void onSave();
        void onSelection(QTreeWidgetItem* it, QTreeWidgetItem * ip);

    private:
        void translateDialog();

    private:
        MainWindow* parent;
        mk1Controller *cnc;
        QVector< QVector<QGroupBox*> > grpArr;

        QVector< QVector<QString> > menuArr;

        SettingsIO *sIO;
        SettingsVis *sVis;
        SettingsControl *sControl;
        SettingsSystem *sSystem;
        SettingsMaterial *sMaterial;
        SettingsSpeed *sSpeed;
        SettingsParser *sParser;
        SettingsWorkbench *sWorkbench;
};


#endif // SETTINGSDIALOG_H
