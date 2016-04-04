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

#include <QtGui>
#include <QPixmap>
#include <QDebug>

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

    recordKey = -1;

    setStyleSheet(parent->programStyleSheet);

    buttonsNumPad = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolNum0 << Ui::ManualControlDialog::toolNum1 <<
                     Ui::ManualControlDialog::toolNum2 << Ui::ManualControlDialog::toolNum3 << Ui::ManualControlDialog::toolNum4 <<
                     Ui::ManualControlDialog::toolNum5 << Ui::ManualControlDialog::toolNum6 << Ui::ManualControlDialog::toolNum7 <<
                     Ui::ManualControlDialog::toolNum8 << Ui::ManualControlDialog::toolNum9 << Ui::ManualControlDialog::toolNumDel <<
                     Ui::ManualControlDialog::toolNumDiv << Ui::ManualControlDialog::toolNumEnter << Ui::ManualControlDialog::toolNumMinus <<
                     Ui::ManualControlDialog::toolNumMult << Ui::ManualControlDialog::toolNumPlus);

    buttonsCursor = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolCurDel << Ui::ManualControlDialog::toolCurDown <<
                     Ui::ManualControlDialog::toolCurEnd << Ui::ManualControlDialog::toolCurHome << Ui::ManualControlDialog::toolCurInsert <<
                     Ui::ManualControlDialog::toolCurLeft << Ui::ManualControlDialog::toolCurPageDn << Ui::ManualControlDialog::toolCurPageUp <<
                     Ui::ManualControlDialog::toolCurRight << Ui::ManualControlDialog::toolCurUp);

    buttonsUser = (QVector<QToolButton*>() << Ui::ManualControlDialog::toolAplus << Ui::ManualControlDialog::toolAminus <<
                   Ui::ManualControlDialog::toolZplus << Ui::ManualControlDialog::toolZminus <<
                   Ui::ManualControlDialog::toolYplus << Ui::ManualControlDialog::toolYminus <<
                   Ui::ManualControlDialog::toolXplus << Ui::ManualControlDialog::toolXminus);

    labelsUser = (QVector<QLabel*>() << Ui::ManualControlDialog::labelAp << Ui::ManualControlDialog::labelAm <<
                  Ui::ManualControlDialog::labelZp << Ui::ManualControlDialog::labelZm <<
                  Ui::ManualControlDialog::labelYp << Ui::ManualControlDialog::labelYm <<
                  Ui::ManualControlDialog::labelXp << Ui::ManualControlDialog::labelXm);

    strsUser = (QVector<QString>() << "+A" << "-A" << "+Z" << "-Z" << "+Y" << "-Y" << "+X" << "-X");


    connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(closePopUp()));
    connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

    if (parent->currentKeyPad == NoManuaControl) {
        tabWidget->setCurrentIndex(0);
    } else {
        tabWidget->setCurrentIndex(parent->currentKeyPad);
    }

    labelNumpad->setWordWrap(true);
    labelCursor->setWordWrap(true);
    labelUserDefined->setWordWrap(true);

    userManualKeys.clear();

    for (int i = 0; i < parent->userKeys.count(); i++) { // copy
        userManualKeys << parent->userKeys.at(i);
    }

    spinBoxVelo->setRange ( 1, 1000 );

    slider->setRange ( 1, 1000 );
    slider->setSingleStep ( 1 );

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    spinBoxVelo->setValue(parent->veloManual);

    foreach (QToolButton* itB, buttonsNumPad) {
        (*itB).setFocusPolicy(Qt::NoFocus);
        connect(&(*itB), SIGNAL(pressed()), this, SLOT(numPressed()));
        connect(&(*itB), SIGNAL(released()), this, SLOT(numPressed()));
    }

    foreach (QToolButton* itB, buttonsCursor) {
        (*itB).setFocusPolicy(Qt::NoFocus);
        connect(&(*itB), SIGNAL(pressed()), this, SLOT(curPressed()));
        connect(&(*itB), SIGNAL(released()), this, SLOT(curPressed()));
    }

    foreach (QToolButton* itB, buttonsUser) {
        (*itB).setFocusPolicy(Qt::NoFocus);
        connect(&(*itB), SIGNAL(pressed()), this, SLOT(userPressed()));
        connect(&(*itB), SIGNAL(released()), this, SLOT(userPressed()));
    }

    this->installEventFilter(this);
    this->setFocus();

    translateDialog();

    adjustSize();
}


void ManualControlDialog::closePopUp()
{
    int idx =  tabWidget->currentIndex();
    parent->currentKeyPad = idx;
    parent->numVeloManual->setValue(spinBoxVelo->value());

    if (idx == 2) {
        // copy of local user key
        for (int i = 0; i < parent->userKeys.count(); i++) { // copy
            parent->userKeys[i] = userManualKeys.at(i);
        }
    }

    accept();
}


bool ManualControlDialog::eventFilter(QObject *target, QEvent *event)
{
    int currentMode = tabWidget->currentIndex();
    int evType = event->type();

    if (evType == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (currentMode == NumPad) {
            decodeNumPad((Qt::Key)keyEvent->key());
        }

        if (currentMode == CursorPad) {
            decodeCursor((Qt::Key)keyEvent->key());
        }

        if (currentMode == UserDefined) {
            if (recordKey != -1) { // already pressed to define
                // check the entered
                checkUserEnteredKey((Qt::Key)keyEvent->key());
                recordKey = -1;
            } else { // send
                decodeUserDefined((Qt::Key)keyEvent->key());
            }
        }

        event->setAccepted(true);
        return true;
    }

    return QDialog::eventFilter(target, event);
}


void ManualControlDialog::decodeCursor(Qt::Key n)
{
    switch (n) {
        case Qt::Key_Home: {
            pressedCommand(Z_plus);
            break;
        }

        case Qt::Key_End: {
            pressedCommand(Z_minus);
            break;
        }

        case Qt::Key_Left: {
            pressedCommand(X_minus);
            break;
        }

        case Qt::Key_Up: {
            pressedCommand(Y_plus);
            break;
        }

        case Qt::Key_Right: {
            pressedCommand(X_plus);
            break;
        }

        case Qt::Key_Down: {
            pressedCommand(Y_minus);
            break;
        }

        case Qt::Key_Delete: {
            pressedCommand(A_minus);
            break;
        }

        case Qt::Key_PageDown: {
            pressedCommand(A_plus);
            break;
        }

        default:
            break;
    }
}


void ManualControlDialog::decodeNumPad(Qt::Key n)
{
    switch (n) {
        case Qt::Key_division: {
            pressedCommand(A_minus);
            break;
        }

        case Qt::Key_multiply: {
            pressedCommand(A_plus);
            break;
        }

        case Qt::Key_Minus: {
            pressedCommand(Z_plus);
            break;
        }

        case Qt::Key_Plus: {
            pressedCommand(Z_minus);
            break;
        }

        case Qt::Key_7:
        case Qt::Key_Home: {
            pressedCommand(X_minus_Y_plus);
            break;
        }

        case Qt::Key_1:
        case Qt::Key_End: {
            pressedCommand(X_minus_Y_minus);
            break;
        }

        case Qt::Key_4:
        case Qt::Key_Left: {
            pressedCommand(X_minus);
            break;
        }

        case Qt::Key_8:
        case Qt::Key_Up: {
            pressedCommand(Y_plus);
            break;
        }

        case Qt::Key_6:
        case Qt::Key_Right: {
            pressedCommand(X_plus);
            break;
        }

        case Qt::Key_2:
        case Qt::Key_Down: {
            pressedCommand(Y_minus);
            break;
        }

        case Qt::Key_9:
        case Qt::Key_PageUp: {
            pressedCommand(X_plus_Y_plus);
            break;
        }

        case Qt::Key_3:
        case Qt::Key_PageDown: {
            pressedCommand(X_plus_Y_minus);
            break;
        }

        default:
            break;
    }
}


void ManualControlDialog::checkUserEnteredKey(Qt::Key n)
{
    if ((recordKey == -1) || (recordKey > (userManualKeys.count() - 1))) {
        return;
    }

    bool found = false;
    qDebug() << "checkUserEnteredKey" << recordKey;

    for(int i = 0; i < userManualKeys.count(); i++) {
        if ( userManualKeys.at(i).code == n) { // exists
            found = true;
            break;
        }
    }

    if (found == false) {
        userManualKeys[recordKey].code = n;
    }

    labelsUser.at(recordKey)->setText("'" + QKeySequence(userManualKeys[recordKey].code).toString() + "'\t\t" + strsUser.at(recordKey));
}


void ManualControlDialog::decodeUserDefined(Qt::Key n)
{
    for(int i = 0; i < userManualKeys.count(); ++i) {
        if (n !=  userManualKeys.at(i).code) {
            continue;
        }

        if ( userManualKeys.at(i).name == "UserZplus") {
            pressedCommand(Z_plus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserZminus") {
            pressedCommand(Z_minus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserXminus") {
            pressedCommand(X_minus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserYplus") {
            pressedCommand(Y_plus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserXplus") {
            pressedCommand(X_plus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserYminus") {
            pressedCommand(Y_minus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserAminus") {
            pressedCommand(A_minus);
            return;
        }

        if ( userManualKeys.at(i).name == "UserAplus") {
            pressedCommand(A_plus);
            return;
        }
    }
}


void ManualControlDialog::numPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());

    int decode[] = { -1, X_minus_Y_minus, Y_minus, X_plus_Y_minus, X_minus, -1, X_plus, X_minus_Y_plus, Y_plus, X_plus_Y_plus, -1, A_minus, -1, Z_plus, A_plus, Z_minus, -1, -1};

    for (int i = 0; i < buttonsNumPad.count(); ++i) {
        if (buttonsNumPad.at(i) == b) {
            pressedCommand(decode[i]);
            break;
        }
    }
}


// not for sending to hardware, only for redifinition of buttons
void ManualControlDialog::userPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());

    int decode[] = { A_plus, A_minus, Z_plus, Z_minus, Y_plus, Y_minus, X_plus, X_minus, -1};

    for (int i = 0; i < buttonsUser.count(); ++i) {
        if (buttonsUser.at(i) == b) {
            //             pressedCommand(decode[i]);
            labelsUser.at(i)->setText("<" + translate(_PRESS_BUTTON) + ">\t" + strsUser.at(i));
            recordKey = i;

            break;
        }
    }
}


void ManualControlDialog::curPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());

    int decode[] = {A_minus, Y_minus, Z_minus, Z_plus, -1, X_minus, A_plus, -1, X_plus, Y_plus, -1};

    for (int i = 0; i < buttonsCursor.count(); ++i) {
        if (buttonsCursor.at(i) == b) {
            pressedCommand(decode[i]);
            break;
        }
    }
}


void ManualControlDialog::translateDialog()
{
    labelVelocity->setText(translate(_VELOCITY));
    labelNumpad->setText(translate(_NUMPAD_HELP));
    labelCursor->setText(translate(_CONTROLPAD_HELP));

    for (int i = 0; i < labelsUser.count(); i++) {
        labelsUser.at(i)->setText("'" + QKeySequence(userManualKeys.at(i).code).toString() + "'\t\t" + strsUser.at(i));
    }

    labelUserDefined->setText(translate(_USEDEF_TEXT));
}


void ManualControlDialog::spinChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n = spinBoxVelo->value();
    slider->setSliderPosition(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::sliderChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n =  slider->sliderPosition();
    spinBoxVelo->setValue(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void ManualControlDialog::pressedCommand(int num)
{
    if (num == -1) {
        return;
    }

    QString x, y, z, a;

    x = y = z = a = "0";

    int speed = spinBoxVelo->value();

    switch (num) {
        case Y_plus: { // y+
            y = "+";
            break;
        }

        case Y_minus: { // y-
            y = "-";
            break;
        }

        case Z_plus: { // z+
            z = "+";
            break;
        }

        case Z_minus: { // z-
            z = "-";
            break;
        }

        case X_minus: { // x-
            x = "-";
            break;
        }

        case X_plus: { // x+
            x = "+";
            break;
        }

        case X_plus_Y_minus: {
            y = "-";
            x = "+";
            break;
        }

        case X_minus_Y_minus: {
            y = "-";
            x = "-";
            break;
        }

        case X_plus_Y_plus: {
            y = "+";
            x = "+";
            break;
        }

        case X_minus_Y_plus: {
            y = "+";
            x = "-";
            break;
        }

        case A_minus: {
            a = "-";
            break;
        }

        case A_plus: {
            a = "+";
            break;
        }

        default:
            return;
    }

    cnc->startManualMove(x, y, z, a,  speed);
    //     cnc->stopManualMove();
}



void ManualControlDialog::mouseReleased()
{
    qDebug() << "mouse released";
    cnc->stopManualMove();
}


