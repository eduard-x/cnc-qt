/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergej Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * Qt developing                                                            *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
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
#include <QPixmap>


#include "includes/ManualControl.h"


/******************************************************************************
** ManualControlDialog
*/

ManualControlDialog::ManualControlDialog(QWidget * p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    setStyleSheet(parent->programStyleSheet);

    buttons = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolButton2 << Ui::ManualControlDialog::toolButton3 <<
               Ui::ManualControlDialog::toolButton4 << Ui::ManualControlDialog::toolButton5 << Ui::ManualControlDialog::toolButton6 <<
               Ui::ManualControlDialog::toolButton7 << Ui::ManualControlDialog::toolButton8 << Ui::ManualControlDialog::toolButton9 <<
               Ui::ManualControlDialog::toolButton10 << Ui::ManualControlDialog::toolButton11);

    labelNumPad->setPixmap(QPixmap(":/images/Numpad1.png"));

    connect(pushButton, SIGNAL(clicked()), this, SLOT(reject()));

    labelInfo->setWordWrap(true);

    spinBox->setRange ( 1, 2000 );

    verticalSlider->setRange ( 1, 2000 );
    verticalSlider->setSingleStep ( 1 );


    connect(spinBox, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));


    for (QVector<QToolButton*>::iterator itB = buttons.begin(); itB != buttons.end(); ++itB) {
        connect((*itB), SIGNAL(pressed()), this, SLOT(mousePressed()));
        connect((*itB), SIGNAL(released()), this, SLOT(mouseReleased()));
    }

    translateDialog();
    //     labelNumPad-> picture of numpad

    adjustSize();
}


void ManualControlDialog::translateDialog()
{
    labelVelocity->setText(translate(_VELOCITY));
    labelInfo->setText(translate(_NUMPAD_HELP));
    groupBox->setTitle(translate(_MOUSE_CONTROL));
}


void ManualControlDialog::spinChanged(int num)
{
    disconnect(spinBox, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n = spinBox->value();
    verticalSlider->setSliderPosition(n);

    connect(spinBox, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::sliderChanged(int num)
{
    disconnect(spinBox, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n =  verticalSlider->tickPosition();
    spinBox->setValue(n);

    connect(spinBox, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::mousePressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());
    int speed = spinBox->value();

    int pos = 0;

    for (QVector<QToolButton*>::iterator itB = buttons.begin(); itB != buttons.end(); ++itB) {
        if ((*itB) == b) {
            break;
        }

        pos++;
    }

    switch (pos) {
        case 0: {
            _cnc.startManualMove("0", "+", "0", "0", speed);
            break;
        }

        case 1: {
            _cnc.startManualMove("0", "-", "0", "0", speed);
            break;
        }

        case 2: {
            _cnc.startManualMove("0", "0", "+", "0", speed);
            break;
        }

        case 3: {
            _cnc.startManualMove("0", "0", "-", "0",  speed);
            break;
        }

        case 4: {
            _cnc.startManualMove("-", "0", "0", "0",  speed);
            break;
        }

        case 5: {
            _cnc.startManualMove("+", "0", "0", "0",  speed);
            break;
        }

        case 6: {
            _cnc.startManualMove("+", "-", "0", "0",  speed);
            break;
        }

        case 7: {
            _cnc.startManualMove("-", "-", "0", "0",  speed);
            break;
        }

        case 8: {
            _cnc.startManualMove("+", "+", "0", "0",  speed);
            break;
        }

        case 9: {
            _cnc.startManualMove("+", "-", "0", "0",  speed);
            break;
        }

        default:
            break;
    }
}


void ManualControlDialog::mouseReleased()
{
    _cnc.stopManualMove();
}


