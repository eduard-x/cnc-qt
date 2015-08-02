/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, developing                                           *
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

#include "includes/MainWindow.h"
#include "includes/ManualControl.h"


/******************************************************************************
** ManualControlDialog
*/

ManualControlDialog::ManualControlDialog(QWidget * p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    cnc = parent->cnc;

    setStyleSheet(parent->programStyleSheet);

    buttonsNumPad = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolNum0 << Ui::ManualControlDialog::toolNum1 <<
               Ui::ManualControlDialog::toolNum2 << Ui::ManualControlDialog::toolNum3 << Ui::ManualControlDialog::toolNum4 <<
               Ui::ManualControlDialog::toolNum5 << Ui::ManualControlDialog::toolNum6 << Ui::ManualControlDialog::toolNum7 <<
               Ui::ManualControlDialog::toolNum8 << Ui::ManualControlDialog::toolNum9 << Ui::ManualControlDialog::toolNumDel <<
               Ui::ManualControlDialog::toolNumDiv << Ui::ManualControlDialog::toolNumEnter << Ui::ManualControlDialog::toolNumMinus <<
               Ui::ManualControlDialog::toolNumMult << Ui::ManualControlDialog::toolNumPlus);

    buttonsControl = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolCurDel << Ui::ManualControlDialog::toolCurDown <<
               Ui::ManualControlDialog::toolCurEnd << Ui::ManualControlDialog::toolCurHome << Ui::ManualControlDialog::toolCurInsert <<
               Ui::ManualControlDialog::toolCurLeft << Ui::ManualControlDialog::toolCurPageDn << Ui::ManualControlDialog::toolCurPageUp <<
               Ui::ManualControlDialog::toolCurRight << Ui::ManualControlDialog::toolCurUp);
     
    buttonsMouse = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolButton2 << Ui::ManualControlDialog::toolButton3 <<
               Ui::ManualControlDialog::toolButton4 << Ui::ManualControlDialog::toolButton5 << Ui::ManualControlDialog::toolButton6 <<
               Ui::ManualControlDialog::toolButton7 << Ui::ManualControlDialog::toolButton8 << Ui::ManualControlDialog::toolButton9 <<
               Ui::ManualControlDialog::toolButton10 << Ui::ManualControlDialog::toolButton11 << Ui::ManualControlDialog::toolButton12 <<
               Ui::ManualControlDialog::toolButton13);
     
    connect(pushButton, SIGNAL(clicked()), this, SLOT(reject()));
   

    labelNumpad->setWordWrap(true);
    labelCursor->setWordWrap(true);

    spinBoxVelo->setRange ( 1, 2000 );

    verticalSlider->setRange ( 1, 2000 );
    verticalSlider->setSingleStep ( 1 );

    changePad(parent->currentKeyPad);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));


    for (QVector<QToolButton*>::iterator itB = buttonsNumPad.begin(); itB != buttonsNumPad.end(); ++itB) {
        connect((*itB), SIGNAL(pressed()), this, SLOT(numPressed()));
        connect((*itB), SIGNAL(released()), this, SLOT(numPressed()));
    }
    
    for (QVector<QToolButton*>::iterator itB = buttonsControl.begin(); itB != buttonsControl.end(); ++itB) {
        connect((*itB), SIGNAL(pressed()), this, SLOT(curPressed()));
        connect((*itB), SIGNAL(released()), this, SLOT(curPressed()));
    }
    
    for (QVector<QToolButton*>::iterator itB = buttonsMouse.begin(); itB != buttonsMouse.end(); ++itB) {
        connect((*itB), SIGNAL(pressed()), this, SLOT(mousePressed()));
        connect((*itB), SIGNAL(released()), this, SLOT(mouseReleased()));
    }
    
    translateDialog();
    //     labelNumPad-> picture of numpad

    adjustSize();
}


void ManualControlDialog::mousePressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());
  
    int pos = 0;
    int decode[] ={Y_plus, Y_minus, Z_minus, Z_plus, X_minus, X_plus, X_minus_Y_plus, X_minus_Y_minus, X_plus_Y_plus, X_plus_Y_minus, A_minus, A_plus, -1 };

    for (QVector<QToolButton*>::iterator itB = buttonsMouse.begin(); itB != buttonsMouse.end(); ++itB) {
        if ((*itB) == b) {
            break;
        }

        pos++;
    }

    pressedCommand(decode[pos]);
}


void ManualControlDialog::numPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());
  
    int pos = 0;
    int decode[] ={-1, -1, X_minus, -1, Y_minus, -1, Y_plus, A_minus, X_plus, A_plus, -1, -1, -1, Z_plus, -1, Z_minus, -1, -1};

    for (QVector<QToolButton*>::iterator itB = buttonsNumPad.begin(); itB != buttonsNumPad.end(); ++itB) {
        if ((*itB) == b) {
            break;
        }

        pos++;
    }

    pressedCommand(decode[pos]);
}


void ManualControlDialog::curPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());
  
    int pos = 0;
    int decode[] ={A_minus, Y_minus, Z_minus, Z_plus, -1, X_minus, A_plus, -1, X_plus, Y_plus, -1};

    for (QVector<QToolButton*>::iterator itB = buttonsControl.begin(); itB != buttonsControl.end(); ++itB) {
        if ((*itB) == b) {
            break;
        }

        pos++;
    }

    pressedCommand(decode[pos]);
}


void ManualControlDialog::changePad(int n)
{
//     tabWidget->setCurrentIndex(n);
//     switch (n) {
//         case NumPad: {
// //             labelNumPad->setPixmap(QPixmap(":/images/numpad.png"));
//             labelInfo->setText(translate(_NUMPAD_HELP));
//             break;
//         }
// 
//         case ControlPad: {
//             labelNumPad->setPixmap(QPixmap(":/images/controlpad.png"));
//             labelInfo->setText(translate(_CONTROLPAD_HELP));
//             break;
//         }
//         
//         case MouseControl: {
//             labelNumPad->setPixmap(QPixmap(":/images/controlpad.png"));
//             labelInfo->setText(translate(_CONTROLPAD_HELP));
//             break;
//         }
// 
//         default: {
//             break;
//         }
//     }
}


void ManualControlDialog::translateDialog()
{
    labelVelocity->setText(translate(_VELOCITY));
//     groupBox->setTitle(translate(_MOUSE_CONTROL));
}


void ManualControlDialog::spinChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n = spinBoxVelo->value();
    verticalSlider->setSliderPosition(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::sliderChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n =  verticalSlider->tickPosition();
    spinBoxVelo->setValue(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(verticalSlider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::pressedCommand(int num)
{
    if (num == -1){
        return;
    }
    
    int speed = spinBoxVelo->value();

     switch (num) {
        case Y_plus: { // y+
            cnc->startManualMove("0", "+", "0", "0", speed);
            break;
        }

        case Y_minus: { // y-
            cnc->startManualMove("0", "-", "0", "0", speed);
            break;
        }

        case Z_plus: { // z+
            cnc->startManualMove("0", "0", "+", "0", speed);
            break;
        }

        case Z_minus: { // z-
            cnc->startManualMove("0", "0", "-", "0",  speed);
            break;
        }

        case X_minus: { // x-
            cnc->startManualMove("-", "0", "0", "0",  speed);
            break;
        }

        case X_plus: { // x+
            cnc->startManualMove("+", "0", "0", "0",  speed);
            break;
        }

        case X_plus_Y_minus: {
            cnc->startManualMove("+", "-", "0", "0",  speed);
            break;
        }

        case X_minus_Y_minus: {
            cnc->startManualMove("-", "-", "0", "0",  speed);
            break;
        }

        case X_plus_Y_plus: {
            cnc->startManualMove("+", "+", "0", "0",  speed);
            break;
        }

        case X_minus_Y_plus: {
            cnc->startManualMove("-", "+", "0", "0",  speed);
            break;
        }
        
        case A_minus: {
            cnc->startManualMove("0", "0", "0", "-",  speed);
            break;
        }
        
        case A_plus: {
            cnc->startManualMove("0", "0", "0", "+",  speed);
            break;
        }
        
        default:
            break;
    }
}



void ManualControlDialog::mouseReleased()
{
    cnc->stopManualMove();
}


