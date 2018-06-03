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

#include <QtGui>

#include "sControl.h"

class Settings;


/******************************************************************************
** EditGCodeDialog
*/


SettingsControl::SettingsControl(QWidget *p)
    : QWidget(p)
{
    setupUi(this);

    recordKey = -1;
    cnc = NULL;

    //     setStyleSheet(parent->programStyleSheet);
    // only in the 'frame' widget
    frame->setStyleSheet("#frame QToolButton {"
                         "border: 2px solid #555;"
                         "border-radius: 25px;"
                         "padding: 5px;"
                         "background: qradialgradient(cx: 0.3, cy: -0.4,"
                         "fx: 0.3, fy: -0.4,"
                         "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                         "min-width: 30px;"
                         "}"

                         "#frame QToolButton:hover {"
                         "background: qradialgradient(cx: 0.3, cy: -0.4,"
                         "fx: 0.3, fy: -0.4,"
                         "radius: 1.35, stop: 0 #fff, stop: 1 #bbb);"
                         "}"

                         "#frame QToolButton:pressed {"
                         "background: qradialgradient(cx: 0.4, cy: -0.1,"
                         "fx: 0.4, fy: -0.1,"
                         "radius: 1.35, stop: 0 #fff, stop: 1 #ddd);"
                         "}");



    buttonsNumPad = (QVector<QToolButton*>() << Ui::sControl::toolNum0 << Ui::sControl::toolNum1 <<
                     Ui::sControl::toolNum2 << Ui::sControl::toolNum3 << Ui::sControl::toolNum4 <<
                     Ui::sControl::toolNum5 << Ui::sControl::toolNum6 << Ui::sControl::toolNum7 <<
                     Ui::sControl::toolNum8 << Ui::sControl::toolNum9 << Ui::sControl::toolNumDel <<
                     Ui::sControl::toolNumDiv << Ui::sControl::toolNumEnter << Ui::sControl::toolNumMinus <<
                     Ui::sControl::toolNumMult << Ui::sControl::toolNumPlus);

    buttonsCursor = (QVector<QToolButton*>() << Ui::sControl::toolCurDel << Ui::sControl::toolCurDown <<
                     Ui::sControl::toolCurEnd << Ui::sControl::toolCurHome << Ui::sControl::toolCurInsert <<
                     Ui::sControl::toolCurLeft << Ui::sControl::toolCurPageDn << Ui::sControl::toolCurPageUp <<
                     Ui::sControl::toolCurRight << Ui::sControl::toolCurUp);

    buttonsUser = (QVector<QToolButton*>() << Ui::sControl::toolAplus << Ui::sControl::toolAminus <<
                   Ui::sControl::toolZplus << Ui::sControl::toolZminus <<
                   Ui::sControl::toolYplus << Ui::sControl::toolYminus <<
                   Ui::sControl::toolXplus << Ui::sControl::toolXminus);

    buttonsJoyPad = (QVector<QToolButton*>() << Ui::sControl::toolButtonA << Ui::sControl::toolButtonB << Ui::sControl::toolButtonC << Ui::sControl::toolButtonD);
    //     QRect *rect = new QRect(10,10,50,50);
    //     QRegion* region = new QRegion(*rect, QRegion::Ellipse);
    QStringList tool = {"Y", "X", "B", "A"};

    int n = 0;

    for (QVector<QToolButton*>::iterator ib = buttonsJoyPad.begin(); ib != buttonsJoyPad.end(); ++ib) {
        (*ib)->setAttribute(Qt::WA_TranslucentBackground);
        (*ib)->setFixedHeight(50);
        (*ib)->setFixedWidth(50);
        (*ib)->setText(tool.at(n));
        ++n;
        //         QRegion* region = new QRegion(*(new QRect((*ib)->x()+5,(*ib)->y()+5,45,45)),QRegion::Ellipse);
        //         (*ib)->setMask(*region);
        // //         (*ib)->setStyleSheet("border: 2px solid #8f8f91; border-radius: 20px");
        //         (*ib)->setStyleSheet("background-color: white;"
        //                             "border-style: solid;"
        //                             "border-width:1px;"
        //                             "border-radius:25px;"
        //                             "border-color: red;"
        //                             "max-width:50px;"
        //                             "max-height:50px;"
        //                             "min-width:50px;"
        //                             "min-height:50px;");
    }

    labelsUser = (QVector<QLabel*>() << Ui::sControl::labelAp << Ui::sControl::labelAm <<
                  Ui::sControl::labelZp << Ui::sControl::labelZm <<
                  Ui::sControl::labelYp << Ui::sControl::labelYm <<
                  Ui::sControl::labelXp << Ui::sControl::labelXm);

    strsUser = (QVector<QString>() << "+A" << "-A" << "+Z" << "-Z" << "+Y" << "-Y" << "+X" << "-X");


    QStringList distList = (QStringList() << "1x (0.1mm)" << "2x (0.2mm)" << "5x (0.5mm)" << "10x (1mm)" << "20x (2mm)" << "50x (5mm)" << "100x (10mm)");
    comboDistance->addItems(distList);

    //     connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(closePopUp()));
    //     connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

    if (Settings::currentKeyPad == NoManuaControl) {
        tabWidgetManual->setCurrentIndex(0);
    } else {
        tabWidgetManual->setCurrentIndex(Settings::currentKeyPad);
    }

    labelNumpad->setWordWrap(true);
    labelCursor->setWordWrap(true);
    labelUserDefined->setWordWrap(true);

    userManualKeys.clear();
    userManualKeys = Settings::userKeys;
    //     for (int i = 0; i < Settings::userKeys.count(); i++) { // copy
    //         userManualKeys << Settings::userKeys.at(i);
    //     }

    spinBoxVelo->setRange ( 1, 1000 );

    slider->setRange ( 1, 1000 );
    slider->setSingleStep ( 1 );

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    spinBoxVelo->setValue(Settings::veloManual);
    checkBoxOpenGL->setChecked(Settings::disableOpenGL);

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

    connect(checkBoxUSBIP, SIGNAL(clicked()), this, SLOT(onEnableRemote()));
    connect(comboBoxIP, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onRemoteNameChanged(const QString &)));
    connect(lineEditPort, SIGNAL(textChanged(const QString &)), this, SLOT(onRemotePortChanged(const QString &)));
    connect(pushConnect, SIGNAL(clicked()), this, SLOT(onConnect()));

    checkBoxOpenGL->setChecked(Settings::disableOpenGL);
    checkBoxUSBIP->setChecked(Settings::enableRemote );
    comboBoxIP->setCurrentText(Settings::remoteName);
    lineEditPort->setText(QString::number(Settings::remotePort));

    tabWidgetManual->setStyleSheet("QTabBar::tab { height: 0px; width: 0px; border: 0px solid #333; }" );

    translateWidget();

    adjustSize();
}


SettingsControl::~SettingsControl()
{
}


void SettingsControl::getSettings()
{
    int idx =  tabWidgetManual->currentIndex();
    Settings::currentKeyPad = idx;
    //     parent->numVeloManual->setValue(spinBoxVelo->value());

    Settings::disableOpenGL = checkBoxOpenGL->isChecked();
    Settings::remoteName = comboBoxIP->currentText();
    Settings::remotePort = lineEditPort->text().toInt();
    Settings::enableRemote = checkBoxUSBIP->isChecked();


    if (idx == 2) {
        // copy of local user key
        Settings::userKeys = userManualKeys;
        //         for (int i = 0; i < Settings::userKeys.count(); i++) { // copy
        //             Settings::userKeys[i] = userManualKeys.at(i);
        //         }
    }
}


void SettingsControl::onConnect()
{
}

void SettingsControl::onEnableRemote()
{
}


void SettingsControl::onRemoteNameChanged(const QString &s)
{
}


void SettingsControl::onRemotePortChanged(const QString &s)
{
}


bool SettingsControl::eventFilter(QObject *target, QEvent *event)
{
    int currentMode = tabWidgetManual->currentIndex();
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

    return QWidget::eventFilter(target, event);
}


void SettingsControl::decodeCursor(Qt::Key n)
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


void SettingsControl::decodeNumPad(Qt::Key n)
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


void SettingsControl::checkUserEnteredKey(Qt::Key n)
{
    if ((recordKey == -1) || (recordKey > (userManualKeys.count() - 1))) {
        return;
    }

    //     bool found = false;
    //     qDebug() << "checkUserEnteredKey" << recordKey;

    //     foreach (QMapIterator<QString, Qt::Key> imap, userManualKeys){
    QMapIterator<QString, Qt::Key> imap(userManualKeys);

    while (imap.hasNext()) {
        imap.next();

        if (userManualKeys[imap.key()] == n) {
            //             found = true;
            labelsUser.at(recordKey)->setText("'" + QKeySequence(imap.value()).toString() + "'\t\t" + strsUser.at(recordKey));
            return;
        }
    }

    //     for(int i = 0; i < userManualKeys.count(); i++) {
    //         if ( userManualKeys.at(i).code == n) { // exists
    //             found = true;
    //             break;
    //         }
    //     }

    //     if (found == false) {
    //     if (imap.hasNext() == false){
    qDebug() << "fehler in checkUserEnteredKey " << recordKey << "nicht gefunden" << n;
    //         return;
    //         userManualKeys[recordKey].code = n;
    //     }


}


void SettingsControl::decodeUserDefined(Qt::Key n)
{
    QMapIterator<QString, Qt::Key> imap(userManualKeys);

    while (imap.hasNext()) {
        imap.next();

        //     for(int i = 0; i < userManualKeys.count(); ++i) {
        if (n !=  imap.value()) {
            continue;
        }

        if ( imap.key() == "UserZplus") {
            pressedCommand(Z_plus);
            return;
        }

        if (imap.key() == "UserZminus") {
            pressedCommand(Z_minus);
            return;
        }

        if (imap.key() == "UserXminus") {
            pressedCommand(X_minus);
            return;
        }

        if (imap.key() == "UserYplus") {
            pressedCommand(Y_plus);
            return;
        }

        if (imap.key() == "UserXplus") {
            pressedCommand(X_plus);
            return;
        }

        if (imap.key() == "UserYminus") {
            pressedCommand(Y_minus);
            return;
        }

        if (imap.key() == "UserAminus") {
            pressedCommand(A_minus);
            return;
        }

        if (imap.key() == "UserAplus") {
            pressedCommand(A_plus);
            return;
        }
    }
}


void SettingsControl::numPressed()
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
void SettingsControl::userPressed()
{
    QToolButton* b  = static_cast<QToolButton*>(sender());

    //     int decode[] = { A_plus, A_minus, Z_plus, Z_minus, Y_plus, Y_minus, X_plus, X_minus, -1};

    for (int i = 0; i < buttonsUser.count(); ++i) {
        if (buttonsUser.at(i) == b) {
            //             pressedCommand(decode[i]);
            labelsUser.at(i)->setText("<" + translate(ID_PRESS_BUTTON) + ">\t" + strsUser.at(i));
            recordKey = i;

            break;
        }
    }
}


void SettingsControl::curPressed()
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


void SettingsControl::spinChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n = spinBoxVelo->value();
    slider->setSliderPosition(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void SettingsControl::sliderChanged(int num)
{
    disconnect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    disconnect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));

    int n =  slider->sliderPosition();
    spinBoxVelo->setValue(n);

    connect(spinBoxVelo, SIGNAL(valueChanged ( int)), this, SLOT(spinChanged(int)));
    connect(slider, SIGNAL(valueChanged ( int)), this, SLOT(sliderChanged(int)));
}


void SettingsControl::pressedCommand(int num)
{
    if (num == -1) {
        return;
    }

    QString x, y, z, a;

    x = y = z = a = "0";

    int speed = spinBoxVelo->value();

    int pulses = 0;

    switch (num) {
        case Y_plus: { // y+
            y = "+";
            pulses = Settings::coord[Y].pulsePerMm;
            break;
        }

        case Y_minus: { // y-
            y = "-";
            pulses = Settings::coord[Y].pulsePerMm;
            break;
        }

        case Z_plus: { // z+
            z = "+";
            pulses = Settings::coord[Z].pulsePerMm;
            break;
        }

        case Z_minus: { // z-
            z = "-";
            pulses = Settings::coord[Z].pulsePerMm;
            break;
        }

        case X_minus: { // x-
            x = "-";
            pulses = Settings::coord[X].pulsePerMm;
            break;
        }

        case X_plus: { // x+
            x = "+";
            pulses = Settings::coord[X].pulsePerMm;
            break;
        }

        case X_plus_Y_minus: {
            y = "-";
            x = "+";
            pulses = Settings::coord[X].pulsePerMm;
            pulses += Settings::coord[Y].pulsePerMm;
            pulses >>= 2;
            break;
        }

        case X_minus_Y_minus: {
            y = "-";
            x = "-";
            pulses = Settings::coord[X].pulsePerMm;
            pulses += Settings::coord[Y].pulsePerMm;
            pulses >>= 2;
            break;
        }

        case X_plus_Y_plus: {
            y = "+";
            x = "+";
            pulses = Settings::coord[X].pulsePerMm;
            pulses += Settings::coord[Y].pulsePerMm;
            pulses >>= 2;
            break;
        }

        case X_minus_Y_plus: {
            y = "+";
            x = "-";
            pulses = Settings::coord[X].pulsePerMm;
            pulses += Settings::coord[Y].pulsePerMm;
            pulses >>= 2;
            break;
        }

        case A_minus: {
            pulses = Settings::coord[A].pulsePerMm;
            a = "-";
            break;
        }

        case A_plus: {
            pulses = Settings::coord[A].pulsePerMm;
            a = "+";
            break;
        }

        default:
            return;
    }

    //
    int n = comboDistance->currentIndex();

    switch (n) {
        case 0:
            pulses *= 0.1;
            break;

        case 1:
            pulses *= 0.2;
            break;

        case 2:
            pulses *= 0.5;
            break;

        case 3:
            // multiplicator 1
            break;

        case 4:
            pulses *= 2;
            break;

        case 5:
            pulses *= 5;
            break;

        case 6:
            pulses *= 10;
            break;

        default:
            break;
    }

    if (pulses > 0) {
        //         cnc->startManualMove(x, y, z, a,  speed, pulses);
    }

    //     cnc->stopManualMove();
}



void SettingsControl::mouseReleased()
{
    qDebug() << "mouse released";
    //     cnc->stopManualMove();
}

void SettingsControl::tabChanged(int n)
{
    tabWidgetManual->setCurrentIndex(n);
}


void SettingsControl::translateWidget()
{
    labelVelocity->setText(translate(ID_VELOCITY));
    labelNumpad->setText(translate(ID_NUMPAD_HELP));
    labelCursor->setText(translate(ID_CONTROLPAD_HELP));
    labelDistance->setText(translate(ID_STEP_DISTANCE));

    pushConnect->setText(translate(ID_CONNECT));
    labelPort->setText(translate(ID_PORT));
    checkBoxUSBIP->setText(translate(ID_REMOTE_NAME));
    groupRemote->setTitle(translate(ID_REMOTE_CONNECTION));

    checkBoxOpenGL->setText(translate(ID_DISABLE_VISUALISATION));

    QStringList lcontrl;
    lcontrl << translate(ID_NUMPAD) << translate(ID_CONTROLPAD) << translate(ID_USER_DEFINED) << translate(ID_JOYPAD);
    comboBoxControl->addItems(lcontrl);

    connect(comboBoxControl, SIGNAL(activated ( int )), this, SLOT(tabChanged(int)));

    emit tabChanged(0);

    labelSelectKeyboard->setText(translate(ID_SELECT_CONTROL));

    QMapIterator<QString, Qt::Key> imap(userManualKeys);
    //     while (imap.hasNext()) {

    for (int i = 0; i < labelsUser.count(); i++) {
        if (imap.hasNext() == false) {
            break;
        }

        imap.next();
        labelsUser.at(i)->setText("'" + QKeySequence(imap.value()).toString() + "'\t\t" + strsUser.at(i));
    }

    labelUserDefined->setText(translate(ID_USEDEF_TEXT));
}


