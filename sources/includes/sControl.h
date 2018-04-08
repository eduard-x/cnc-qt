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


#ifndef SCONTROL_H
#define SCONTROL_H

#include <QGroupBox>
#include <QString>
#include <QToolButton>
#include <QLabel>
#include <QVector>
#include <QLabel>
#include <QKeyEvent>
#include <QMap>


#include "MainWindow.h"
#include "ui_sControl.h"
#include "mk1Controller.h"
#include "SettingsDialog.h"

class mk1Controller;
class MainWindow;


class SettingsControl : public QWidget, public Ui::sControl,  public cTranslator
{
        Q_OBJECT
    public:
        SettingsControl(QWidget *parent = 0);
        ~SettingsControl();
        void getSettings();

        enum Direction { X_minus = 0, X_plus, Y_minus, Y_plus, Z_minus, Z_plus, A_minus, A_plus, X_minus_Y_minus, X_minus_Y_plus, X_plus_Y_plus, X_plus_Y_minus };

    private slots:
        //         void mousePressed();
        void numPressed();
        void curPressed();
        void userPressed();
        void tabChanged(int n);
        void mouseReleased();
        void spinChanged(int num);
        void sliderChanged(int num);
        void onEnableRemote();
        void onRemoteNameChanged(const QString &s);
        void onRemotePortChanged(const QString &s);
        void onConnect();

    private:
        void translateWidget();
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
        QVector< QVector<QGroupBox*> > grpArr;
        QVector<QToolButton*> buttonsNumPad;
        QVector<QToolButton*> buttonsCursor;
        QVector<QToolButton*> buttonsUser;
        QVector<QToolButton*> buttonsJoyPad;
        QVector<QLabel*> labelsUser;
        QVector<QString> strsUser;
        QMap<QString, Qt::Key> userManualKeys;

        int recordKey; // num of element or -1
        //     private:
        mk1Controller* cnc;
};


#endif // SCONTROL_H
