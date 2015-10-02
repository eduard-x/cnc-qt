/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2015 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
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


#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QVector>
#include <QLabel>
#include <QKeyEvent>

#include "MainWindow.h"
#include "ui_ManualControl.h"
#include "mk1Controller.h"


class MainWindow;



class ManualControlDialog : public QDialog, public Ui::ManualControlDialog,  public cTranslator
{
        Q_OBJECT
    public:
        ManualControlDialog(QWidget *parent = 0);

        enum Direction { X_minus = 0, X_plus, Y_minus, Y_plus, Z_minus, Z_plus, A_minus, A_plus, X_minus_Y_minus, X_minus_Y_plus, X_plus_Y_plus, X_plus_Y_minus };

    private slots:
        //         void mousePressed();
        void numPressed();
        void curPressed();
        void userPressed();
        void closePopUp();
        void mouseReleased();
        void spinChanged(int num);
        void sliderChanged(int num);

    private:
        void translateDialog();
        void pressedCommand(int n);
        void decodeUserDefined(Qt::Key n);
        void checkUserEnteredKey(Qt::Key n);
        void decodeCursor(Qt::Key n);
        void decodeNumPad(Qt::Key n);

        //         void keyPressEvent( QKeyEvent *e );
        //         void keyReleaseEvent( QKeyEvent *e );
        bool eventFilter(QObject *target, QEvent *event);
        //         bool eventFilter(QObject *target, QMouseEvent *event);

    private:
        QVector<QToolButton*> buttonsNumPad;
        QVector<QToolButton*> buttonsCursor;
        QVector<QToolButton*> buttonsUser;
        QVector<QLabel*> labelsUser;
        QVector<QString> strsUser;
        QVector<uKeys> userManualKeys;

        int recordKey; // num of element or -1
        MainWindow* parent;
        mk1Controller* cnc;

};


#endif // MANUALCONTROL_H
