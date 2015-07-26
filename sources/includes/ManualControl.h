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


#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QVector>

#include "MainWindow.h"
#include "ui_ManualControl.h"
#include "mk1Controller.h"


class MainWindow;


class ManualControlDialog : public QDialog, public Ui::ManualControlDialog,  public cTranslator
{
        Q_OBJECT
    public:
        ManualControlDialog(QWidget *parent = 0);

    private slots:
        void mousePressed();
        void mouseReleased();
        void spinChanged(int num);
        void sliderChanged(int num);

    private:
        void translateDialog();

    private:
        QVector<QToolButton*> buttons;
        MainWindow* parent;
        mk1Controller _cnc;

};


#endif // MANUALCONTROL_H
