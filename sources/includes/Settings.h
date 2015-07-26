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


#ifndef SETTINGS_H
#define SETTINGS_H

#include "MainWindow.h"
#include "ui_Settings.h"

class cTranslator;
class MainWindow;


class SettingsDialog : public QDialog, public Ui::SettingsDialog,  public cTranslator
{
        Q_OBJECT
    public:
        SettingsDialog(QWidget *parent = 0);

    private slots:
        void onSave();

    private:
        void translateDialog();

    private:
        MainWindow* parent;
        mk1Controller* _cnc;
};


#endif // SETTINGS_H
