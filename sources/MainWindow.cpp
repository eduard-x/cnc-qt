/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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


#include <QApplication>
#include <QMainWindow>
#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QVector>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QToolButton>


#include <QtCore/qmath.h>

#include "includes/SettingsDialog.h"
#include "includes/About.h"
#include "includes/Reader.h"
#include "includes/EditGCode.h"
#include "includes/ScanSurface.h"


#include "includes/MainWindow.h"


class GLWidget;

class MainWindow;


/**
 * @brief
 *
 */
void MessageTimerBox::showEvent ( QShowEvent * event )
{
    currentTime = 0;

    if (autoClose) {
        this->startTimer(1000);
    }
}


/**
 * @brief
 *
 */
void MessageTimerBox::setDefaultText(const QString &t)
{
    defaultText = t;
}


/**
 * @brief
 *
 */
void MessageTimerBox::setAutoClose(bool b)
{
    autoClose = b;
}


/**
 * @brief
 *
 */
void MessageTimerBox::setTimeout(int t)
{
    timeout = t;
    QString tx;

    tx = defaultText;
    tx.replace("%d", QString::number(timeout));
    setText(tx);
}


/**
 * @brief
 *
 */
void MessageTimerBox::timerEvent(QTimerEvent *event)
{
    QString t;
    currentTime++;
    t = defaultText;
    t.replace("%d", QString::number(timeout - currentTime));

    setText(t);

    if (currentTime >= timeout) {
        this->done(0);
    }
}


/**
 * @brief
 *
 */
int MessageBox::exec(void* p, const QString &title, const QString &text, int ticon)
{
    int ret;
    QMessageBox* msgBox = NULL;
    MainWindow *parent = static_cast<MainWindow*>(p);


    msgBox = new QMessageBox((QWidget*)parent);
    msgBox->setIcon((QMessageBox::Icon)ticon);
    msgBox->setWindowTitle(title);
    msgBox->setText(text);

    if (ticon == QMessageBox::Question) {
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setButtonText(QMessageBox::Yes, translate(ID_YES));
        msgBox->setButtonText(QMessageBox::No, translate(ID_NO));
    } else {
        msgBox->setStandardButtons(QMessageBox::Yes);
        msgBox->setButtonText(QMessageBox::Yes, translate(ID_OK));
    }

    if (parent->programStyleSheet.length() > 0) {
        msgBox->setStyleSheet(parent->programStyleSheet);
    }

    ret = msgBox->exec();

    delete msgBox;

    return ret;
}


// because of static
int Task::lineCodeStart = -1;
int Task::lineCodeNow = -1;
int Task::lineCodeEnd = -1;
int Task::instrCounter = -1;


/**
 * @brief constructor of main window
 *
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)//, Reader ()
{
    setupUi(this);

    textLog->document()->setMaximumBlockCount(10000);

    setWindowTitle(translate(ID_PROG_NAME));

    currentStatus = Task::Stop;

    Settings::currentAppDir = qApp->applicationDirPath();

    if (Settings::currentAppDir.lastIndexOf("/build") > 0) { // build dir detection
        Settings::currentAppDir.remove("/build" );
    }

    QString n = QString::number(1.01);
    Settings::toDecimalPoint = (n.indexOf(",") > 0) ? ',' : '.';
    Settings::fromDecimalPoint = (Settings::toDecimalPoint == ',') ? '.' : ',';


    // detection of remote connection
    enableOpenGL = false;

    QString d = getenv( "DISPLAY" ); // linux machines only!

    // to disable the OpenGL features, if over ssh
    enableOpenGL = (d.indexOf(QRegExp(":[0-9]")) == 0);

    Settings::currentLang = "English";

    filesMenu = 0;
    filesGroup = 0;
    //
    QFont sysFont = qApp->font();
    sysFont = sysFont;

    Settings::fontSize = sysFont.pointSize();

    reader = new Reader();

    labelTask->setText("");

    int rc = 0;
    context = NULL;
    rc = libusb_init(&context);

    if (rc != 0) {
        qApp->quit();
    }


    readSettings();

    setWindowIcon(QIcon(QPixmap(":/images/icon.png")));

    programStyleSheet = QString().sprintf("font-size: %dpt", Settings::fontSize);

    if ( Settings::fontSize == -1) {
        Settings::fontSize = sysFont.pixelSize();
        programStyleSheet = QString().sprintf("font-size: %dpx", Settings::fontSize);
    }

    if (programStyleSheet.length() > 0) {
        setStyleSheet(programStyleSheet);
    }

    if (readLangDir() == false) { // init from langFiles variable in format "filename:language"
        MessageBox::exec(this, translate(ID_ERR),
                         "Directory with other languages not found\nDefault GUI language is english", QMessageBox::Critical);
    }

    listGCodeWidget->clear();
    listGCodeWidget->setColumnCount(3);

    listGCodeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    actionOpen->setShortcut(QString("Ctrl+O"));
    actionExit->setShortcut(QString("Ctrl+Q"));

    addStatusWidgets();

    sceneCoordinates = NULL;

    //
    mk1 = new mk1Controller();


    drawWorkbench();


    //     enableOpenGL = false;

    // OpenGL area
    if (enableOpenGL == true) {
        scene3d = new GLWidget(this);

        scrollArea->setWidget(scene3d);

        OpenGL_preview->addWidget(scrollArea, 0, 0);

        QPalette palette = statusLabel2->palette();
        //  palette.setColor(statusLabel2->backgroundRole(), Qt::yellow);
        palette.setColor(statusLabel2->foregroundRole(), Qt::darkBlue);
        statusLabel2->setPalette(palette);
        statusLabel2->setText( "OpenGL " + translate(ID_ENABLED));

        // OpenGL is placed in widget
    } else {
        scene3d = 0;

        QPalette palette = statusLabel2->palette();
        //  palette.setColor(statusLabel2->backgroundRole(), Qt::yellow);
        palette.setColor(statusLabel2->foregroundRole(), Qt::red);
        statusLabel2->setPalette(palette);
        statusLabel2->setText( "OpenGL " + translate(ID_DISABLED) );
        tabWidget->removeTab(1);
    }



    Correction = false;
    deltaX = 0;
    deltaY = 0;
    deltaZ = 0;
    coeffSizeX = 1;
    coeffSizeY = 1;
    deltaFeed = false;

    scale = 1;

    addConnections();

    if (getLangTable() == false) {
        MessageBox::exec(this, translate(ID_ERR),
                         "Can't open language file!\nDefault GUI language is english", QMessageBox::Critical);

        Settings::currentLang = "English";
    }

    foreach (QAction* itL, actLangSelect) {
        if ((*itL).text() == Settings::currentLang) {
            (*itL).setChecked(true);
            break;
        }
    }

    QStringList arguments = QCoreApplication::arguments();

    if (arguments.size() > 1) {
        if (arguments.at(1).length() > 0) { // as parameter is file name to load
            QString nm = arguments.at(1);

            if (OpenFile(nm) == false) {
                AddLog("File loading error: " + nm );
                return;
            }
        }
    }

    statusProgress->setRange(0, 100);
    statusProgress->setValue(0);

    translateGUI();

    refreshElementsForms();
};


void MainWindow::drawWorkbench()
{
    //
#if 0
    QPixmap p1 = QPixmap(":/images/workbench.png");

    if (sceneCoordinates != NULL) {
        delete sceneCoordinates;
    }

    sceneCoordinates = new QGraphicsScene();
    QGraphicsPixmapItem *item_p1 = sceneCoordinates->addPixmap(p1);

    QPen penLine;
    penLine.setWidth(5);
    penLine.setBrush(Qt::green);
    penLine.setStyle(Qt::DashLine);

    QPen penEllipse;
    penEllipse.setBrush(Qt::green);
    penEllipse.setWidth(20);

    QFont font;
    font.setPixelSize(20);
    font.setBold(true);


    sceneCoordinates->addLine(QLineF(10.0, 10.0, 10.0, 200.0), penLine);
    sceneCoordinates->addEllipse(QRectF(3.0, 5.0, 20.0, 20.0), penEllipse);
    QGraphicsTextItem *textZ = sceneCoordinates->addText(Settings::coord[Z].invertDirection ? "- Z" : "+Z");
    textZ->setFont(font);
    textZ->setPos(0, 0);


    sceneCoordinates->addLine(QLineF(10.0, 200.0, 180.0, 150.0), penLine);
    sceneCoordinates->addEllipse(QRectF(175.0, 140.0, 20.0, 20.0), penEllipse);

    QGraphicsTextItem *textY = sceneCoordinates->addText(Settings::coord[Y].invertDirection ? "- Y" : "+Y");
    textY->setFont(font);
    textY->setPos(170, 135);

    sceneCoordinates->addLine(QLineF(10.0, 200.0, 90.0, 300.0), penLine);
    sceneCoordinates->addEllipse(QRectF(85.0, 295.0, 20.0, 20.0), penEllipse);

    QGraphicsTextItem *textX = sceneCoordinates->addText(Settings::coord[X].invertDirection ? "- X" : "+X");
    textX->setFont(font);
    textX->setPos(80, 290);


    item_p1->setVisible(true);

    graphicsView->setStyleSheet("background: transparent");

    graphicsView->setScene(sceneCoordinates);
#endif
}


/**
 * @brief try to find the translation file
 *
 */
bool MainWindow::getLangTable()
{
    QString lang = Settings::currentLang;
    QString fileLang = "";

    foreach (const QString iLang, langFiles) {
        int pos = iLang.lastIndexOf(":" + lang);

        if (pos > 0) {
            fileLang = iLang.left(pos);
            break;
        }
    }

    if (fileLang == "") {
        return (false);
    }

    if (QFile::exists(Settings::langDir + "/" + fileLang) == false) {
        MessageBox::exec(this, translate(ID_ERR), "Language file not exists!\n\n"
                         + Settings::langDir + "\n\n" + fileLang, QMessageBox::Warning);
        // not found
        return (false);
    }

    return loadTranslation(Settings::langDir + fileLang);
}


/**
 * @brief build the SIGNAL - SLOT connections
 *
 */
void MainWindow::addConnections()
{
    // connect menu actions
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onOpenFile()));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(onSaveFile()));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(onExit()));

    connect(reader, SIGNAL(logMessage(const QString&)), this, SLOT(logMessage(const QString&)));

    //     connect(actionOpenGL, SIGNAL(triggered()), this, SLOT(on3dSettings()));
    connect(actionProgram, SIGNAL(triggered()), this, SLOT(onSettings()));

    connect(toolCalcVelocity, SIGNAL(clicked()), this, SLOT(onCalcVelocity()));

    //     connect(actionConnect, SIGNAL(triggered()), this, SLOT(onConnect()));
    //     connect(actionDisconnect, SIGNAL(triggered()), this, SLOT(onDisconnect()));
    //     connect(actionConnectDisconnect, SIGNAL(triggered()), this, SLOT(onConnDisconnect()));
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(onSettings()));

    connect(pushManualControl, SIGNAL(clicked()), this, SLOT(onManualControlDialog()));

    connect(toolResetCoorX, SIGNAL(clicked()), this, SLOT(onButtonXtoZero()));
    connect(toolResetCoorY, SIGNAL(clicked()), this, SLOT(onButtonYtoZero()));
    connect(toolResetCoorZ, SIGNAL(clicked()), this, SLOT(onButtonZtoZero()));
    connect(toolResetCoorA, SIGNAL(clicked()), this, SLOT(onButtonAtoZero()));

    connect(pushClean, SIGNAL(clicked()), this, SLOT(onLogClear()));
    connect(actionSpindle, SIGNAL(triggered()), this, SLOT(onSpindel()));
    connect(actionMist, SIGNAL(triggered()), this, SLOT(onMist()));
    connect(actionFluid, SIGNAL(triggered()), this, SLOT(onFluid()));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(onEmergyStop()));
    connect(actionScan, SIGNAL(triggered()), this, SLOT(onScanSurface()));

    connect(toolRun, SIGNAL(clicked()), this, SLOT(onStartTask()));
    connect(toolPause, SIGNAL(clicked()), this, SLOT(onPauseTask()));
    connect(toolStop, SIGNAL(clicked()), this, SLOT(onStopTask()));

    connect(actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    connect(actionAboutQt, SIGNAL(triggered()), this, SLOT(onAboutQt()));

    connect(pushCopyPos, SIGNAL(clicked()), this, SLOT(onCopyPos()));
    connect(pushSetHome, SIGNAL(clicked()), this, SLOT(onSetHome()));
    connect(pushCopyHome, SIGNAL(clicked()), this, SLOT(onCopyHome()));
    // end menu
#if 0
    // workbench
    connect(checkBoxSwapX, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchSwap()));
    connect(checkBoxSwapY, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchSwap()));
    connect(checkBoxSwapZ, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchSwap()));
    connect(checkBoxSwapA, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchSwap()));

    connect(checkBoxLimitsXmin, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsXmax, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsYmin, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsYmax, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsZmin, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsZmax, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsAmin, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    connect(checkBoxLimitsAmax, SIGNAL(clicked()), this, SLOT(onCheckBoxWorkbenchLimits()));
    // end of workbench
#endif
    connect(listGCodeWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onEditGCode(int, int)));
    connect(listGCodeWidget, SIGNAL(cellActivated(int, int)), this, SLOT(onCellSelect(int, int)));

    connect(pushSendSignal, SIGNAL(clicked()), this, SLOT(onSendCommand()));
    connect(toolRunMoving, SIGNAL(clicked()), this, SLOT(onRunToPoint()));

    // connected or disconnected
    //     connect(mk1, SIGNAL(hotplugSignal ()), this, SLOT(onCncHotplug())); // cnc->WasConnected += CncConnect;
    //     connect(mk1, SIGNAL(hotplugDisconnected ()), this, SLOT(onCncHotplug())); // cnc->WasDisconnected += CncDisconnect;

    if (mk1 != 0) {
        connect(mk1, SIGNAL(newDataFromMK1Controller ()), this, SLOT(onCncNewData())); // cnc->NewDataFromController += CncNewData;
        connect(mk1, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message += CncMessage;

        connect(this, SIGNAL(mk1Connected()),      mk1,  SLOT(onDeviceConnected()) );
        connect(this, SIGNAL(mk1Disconnected()),   mk1,  SLOT(onDeviceDisconnected()) );
    }

    // Start timer that will periodicaly check if hardware is connected
    hotplugTimer = new QTimer(this);
    // this signal-slot connection is Qt5
    connect(hotplugTimer, SIGNAL(timeout()), this, SLOT(hotplugTimerTick()) );
    hotplugTimer->start(250); // every 0.25 sec


    refreshGUITimer = new QTimer(this);
    connect(refreshGUITimer, SIGNAL(timeout()), this, SLOT(onRefreshGUITimer()));
    refreshGUITimer->start(500);// every 0.5 sec update


    if (enableOpenGL == true) {

        // 3d buttons
        //         connect(posAngleXm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXm()));
        //         connect(posAngleX, SIGNAL(clicked()), scene3d, SLOT(onPosAngleX())); // reset to 0
        //         connect(scene3d, SIGNAL(rotationChanged()), this, SLOT(getRotation()));
        connect(scene3d, SIGNAL(fpsChanged(int)), this, SLOT(getFPS(int)));
        //         connect(posAngleXp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXp()));

        //         connect(posAngleYm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYm()));
        //         connect(posAngleY, SIGNAL(clicked()), scene3d, SLOT(onPosAngleY())); // reset to 0
        //         connect(scene3d, SIGNAL(yRotationChanged(int)), this, SLOT(getYRotation(int)));
        //         connect(posAngleYp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYp()));

        //         connect(posAngleZm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZm()));
        //         connect(posAngleZ, SIGNAL(clicked()), scene3d, SLOT(onPosAngleZ())); // reset to 0
        //         connect(scene3d, SIGNAL(zRotationChanged(int)), this, SLOT(getZRotation(int)));
        //         connect(posAngleZp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZp()));

        //         connect(scene3d, SIGNAL(scaleChanged(int)), this, SLOT(getScale(int)));

        //         connect(pushDefaultPreview, SIGNAL(clicked()), scene3d, SLOT(onDefaulPreview()));
        // end of 3d buttons

    }

    connect(radioFixX, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixY, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixZ, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));

    connect(actionInfo, SIGNAL(triggered()), this, SLOT(onDeviceInfo()));

    radioFixY->setChecked(true);

    //     onCncHotplug();
}



// Function pools messges from udev
// Linux specific
void MainWindow::hotplugTimerTick()
{
    size_t count = 0;
    static bool mk1_connected = false;

    libusb_device **list = NULL;

    count = libusb_get_device_list(context, &list);

    if(count == 0) {
        mk1_connected = false;
        emit mk1Disconnected();
        return;
    }

    bool mk1_detected = false;

    for (size_t idx = 0; idx < count; ++idx) {
        libusb_device *device = list[idx];
        libusb_device_descriptor desc = {0};

        int rc = libusb_get_device_descriptor(device, &desc);

        if (rc != 0) {
            continue;
        }

        if (desc.idVendor == MK1_VENDOR_ID && desc.idProduct == MK1_PRODUCT_ID) {
            mk1_detected = true;
            //      printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);
        }
    }

    libusb_free_device_list(list, 1);

    if (mk1_detected != mk1_connected) {
        if (mk1_detected == true) {
            AddLog(translate(ID_HOTPLUGED));
            emit mk1Connected();
        } else {
            AddLog(translate(ID_DETACHED));
            emit mk1Disconnected();
        }

        actionInfo->setEnabled(mk1_detected);
        mk1_connected = mk1_detected;

        if (refreshGUITimer->isActive() == false) {
            refreshElementsForms();
        }
    }
}


/**
 * @brief slot for popup window with mk1 device information
 *
 */
void MainWindow::onDeviceInfo()
{
    QDialog *gamatosdialog = new QDialog(this);
    gamatosdialog->setWindowTitle("Device information");
    QLabel *label = new QLabel(mk1->getDescription());
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label);
    gamatosdialog->setLayout(mainLayout);
    gamatosdialog->exec();
}


/**
 * @brief slot for selecting of fixed axis: X, Y, Z. unselect other radiobuttons
 *
 */
void MainWindow::onChangeFix(bool checked)
{
    QRadioButton* b  = static_cast<QRadioButton*>(sender());

    disconnect(radioFixX, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    disconnect(radioFixY, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    disconnect(radioFixZ, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));

    if (b == radioFixX) {
        fixedAxes = FixX;
        radioFixY->setChecked(false);
        radioFixZ->setChecked(false);
    }

    if (b == radioFixY) {
        fixedAxes = FixY;
        radioFixX->setChecked(false);
        radioFixZ->setChecked(false);
    }

    if (b == radioFixZ) {
        fixedAxes = FixZ;
        radioFixX->setChecked(false);
        radioFixY->setChecked(false);
    }

    connect(radioFixX, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixY, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixZ, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
}

/**
 * @brief scan the directory with translations. language files are with .utf extentions
 *
 */
bool MainWindow::readLangDir()
{
    bool found = false;
    QString lngDirName;
    QStringList dirsLang;
    QDir dir;
    dirsLang << "/usr/share/cnc-qt/" << "/usr/local/share/cnc-qt/" << Settings::currentAppDir;

    foreach(const QString entry, dirsLang) {
        lngDirName = entry + "/lang/";

        dir = QDir(lngDirName);

        if (dir.exists() == true) {
            found = true;
            break;
        }

        lngDirName = entry + "/language/";
        dir = QDir(lngDirName);

        if (dir.exists() == true) {
            found = true;
            break;
        }
    }

    if(found == false) {
        return false;
    }

    langFiles.clear();

    QStringList fList = dir.entryList(QStringList("*.utf"));

    found = false;

    langMenu = menuSettings->addMenu(translate(ID_LANGUAGE));

    langGroup = new QActionGroup(this);


    foreach (const QString iL, fList) {
        QFile fLang(lngDirName + iL);

        if (fLang.exists() == false) {
            continue;
        }

        QString iconName;

        if (fLang.open(QIODevice::ReadOnly)) {      //load
            QTextStream stream(&fLang);
            stream.setCodec("UTF-8" );
            QString line, nm;

            int lines = 0;

            while (!stream.atEnd()) {
                line = stream.readLine(); // line of text excluding '\n'
                lines++;

                if (line == "LANGUAGE_NAME") {
                    line = stream.readLine();
                    lines++;
                    nm = line;
                    continue;
                }

                if (line == "LANGUAGE_ISO") {
                    line = stream.readLine();
                    selectedLang = line;
                    lines++;

                    iconName = lngDirName + "flags/" + line + ".png";

                    found = true;

                    langFiles += iL + ":" + nm;
                    QAction *tmpAction = new QAction(nm, actionLanguage);
                    tmpAction->setCheckable(true);

                    if (QFile::exists(iconName) == true) {
                        QFile flIcon(iconName);

                        if (flIcon.size() < 1024 ) { // checking of filesize
                            tmpAction->setIcon(QIcon(iconName));
                        }
                    }

                    langGroup->addAction(tmpAction);
                    langMenu->addAction(tmpAction);

                    if (Settings::currentLang == nm) {
                        tmpAction->setChecked(true);
                    }

                    actLangSelect.push_back(tmpAction);
                    break;
                }

                if (lines > 8) {
                    break;
                }
            }

            fLang.close();

        } else {
            continue;
        }
    }

    connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(setLang(QAction*)));

    return (found);
}



/**
 * @brief update settings on main gui after change
 *
 */
#if 0
void MainWindow::updateSettingsOnGUI()
{
    checkBoxSwapX->setChecked(Settings::coord[X].invertDirection);
    checkBoxSwapY->setChecked(Settings::coord[Y].invertDirection);
    checkBoxSwapZ->setChecked(Settings::coord[Z].invertDirection);
    checkBoxSwapA->setChecked(Settings::coord[A].invertDirection);

    checkBoxLimitsXmin->setChecked(Settings::coord[X].useLimitMin);
    checkBoxLimitsXmax->setChecked(Settings::coord[X].useLimitMax);

    checkBoxLimitsYmin->setChecked(Settings::coord[Y].useLimitMin);
    checkBoxLimitsYmax->setChecked(Settings::coord[Y].useLimitMax);

    checkBoxLimitsZmin->setChecked(Settings::coord[Z].useLimitMin);
    checkBoxLimitsZmax->setChecked(Settings::coord[Z].useLimitMax);

    checkBoxLimitsAmin->setChecked(Settings::coord[A].useLimitMin);
    checkBoxLimitsAmax->setChecked(Settings::coord[A].useLimitMax);
}
#endif

/**
 * @brief save settings of program
 *
 */
void MainWindow::writeSettings()
{
    //     Settings::veloCutting = numVeloSubmission->value();
    //     Settings::veloMoving = numVeloMoving->value();
    Settings::veloManual = numVeloManual->value();

    Settings::progPos = pos();//   QPoint pos = s->value("pos", QPoint(200, 200)).toPoint();
    Settings::progSize = size(); //QSize sz = s->value("size", QSize(730, 440)).toSize();


    if (groupManualControl->isChecked() == false) {
        Settings::currentKeyPad = -1;
    }

    Settings::saveSettings();
}


/**
 * @brief load settings of program
 *
 */
void MainWindow::readSettings()
{
    Settings::readSettings();

    resize(Settings::progSize);
    move(Settings::progPos);

    groupManualControl->setChecked( Settings::currentKeyPad != -1);

    //     numVeloSubmission->setValue(Settings::veloCutting);
    //     numVeloMoving->setValue(Settings::veloMoving);
    numVeloManual->setValue( Settings::veloManual);

    reloadRecentList();
}


/**
 * @brief refresh the recent list after loading of file
 *
 */
void MainWindow::reloadRecentList()
{
    if (filesMenu != 0) {
        delete filesMenu;
    }

    if (filesGroup != 0) {
        disconnect(filesGroup, SIGNAL(triggered(QAction*)), this, SLOT(setFile(QAction*)));
        delete filesGroup;
    }

    actFileSelect.clear();

    if (Settings::lastFiles.count() <= 0) {
        return;
    }

    filesMenu = new QMenu( translate(ID_RECENTFILES)); //insertAction
    QAction *actionRecent = menuFile->insertMenu(actionSave, filesMenu);
    filesGroup = new QActionGroup(this);

    QStringList tmpList;

    foreach (QString iL, Settings::lastFiles) {
        QString fName = iL;
        QFileInfo fRecent(fName);

        fName = fRecent.absoluteFilePath();

        if (fRecent.exists() == false) {
            Settings::lastFiles.removeOne(fName);
            continue;
        }

        if (fName.indexOf(QDir::homePath()) >= 0) {
            fName.replace(QDir::homePath(), QString("~"));
        }

        tmpList << fName;
    }

    tmpList.removeDuplicates();

    foreach (QString iL, tmpList) {
        QAction *tmpAction = new QAction(iL, actionRecent);

        filesGroup->addAction(tmpAction);
        filesMenu->addAction(tmpAction);

        actFileSelect.push_back(tmpAction);
    }

    connect(filesGroup, SIGNAL(triggered(QAction*)), this, SLOT(setFile(QAction*)));
}


//
// dialog for opening of file
//
bool MainWindow::OpenFile(QString &fileName)
{
    QString name;
    QString dir;

    dir = ( Settings::lastDir.length() > 0) ? Settings::lastDir : QDir::homePath();

    if (fileName == "") {
        name = QFileDialog::getOpenFileName ( 0, translate(ID_LOAD_FROM_FILE), dir );

        if (name.length() == 0) {
            return false;
        }
    } else {
        name = fileName;
    }

    if (name.length() > 0) {
        bool f = reader->readFile(name);

        if (f == true) {
            QFileInfo fi(name);
            fileName = fi.absoluteFilePath();

            Settings::lastFiles.insert(0, name);

            reloadRecentList();

            QVector<QString> l = reader->getGoodList();
            fillListWidget(l);

            gCodeData = reader->getGCodeData();



            if (enableOpenGL == true) {
                scene3d->loadFigure();
            }

            //     l = reader->getBadList();
            //
            //     if (l.count() != 0) {
            //         foreach (QString s, l) {
            //             AddLog(s);
            //         }
            //     }

            if (gCodeData.count() > 0) {
                name.replace(QDir::homePath(), QString("~"));
                AddLog("File loaded: " + name );
            }

            Settings::lastDir = QFileInfo(fileName).absoluteDir().absolutePath();
        }

        return f;
    }

    return false;
}

/**
 * @brief select file from recent files list
 *
 */
void MainWindow::setFile(QAction* a)
{
    QString fileStr;
    disconnect(filesGroup, SIGNAL(triggered(QAction*)), this, SLOT(setFile(QAction*)));

    fileStr = a->text();

    fileStr = fileStr.remove("&");
    fileStr = fileStr.replace("~", QDir::homePath());

    if (OpenFile(fileStr) == false) {
        AddLog("File loading error: " + fileStr );
//         return;
    }
     connect(filesGroup, SIGNAL(triggered(QAction*)), this, SLOT(setFile(QAction*)));
}

/*
void MainWindow::change_language(QString language)
{
    if (language == "deutsch") {
        qApp->removeTranslator(&language_en); //die Ursprungssprache wird hergestellt
    }

    if (language == "english") {
        language_en.load("cnc_qt_en"); // Aufruf der .qm Datei wenn sich die Datei direkt im Buildpfad befindet
        qApp->installTranslator(&language_en); //die Englishe Translationsdatei wird geladen
    }
}*/

/**
 * @brief set GUI to selected language and do the translation of all GUI widgets
 *
 */
void MainWindow::setLang(QAction* mnu)
{
    QString lngStr;
    mnu = langGroup->checkedAction();

    lngStr = mnu->text();
    lngStr = lngStr.remove("&");

    Settings::currentLang = lngStr;

    if (getLangTable() == false) {
        qDebug() << "setLang" << false;
    }

    disconnect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(setLang(QAction*)));

    mnu->setChecked(true);

    connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(setLang(QAction*)));

    translateGUI();
}



/**
 * @brief display in status text label the actual FPS
 *
 */
void MainWindow::getFPS(int f)
{
    statusLabel2->setText( "OpenGL, FPS: " + QString::number(f));
}


/**
 * @brief
 *
 */
// void MainWindow::getRotation()
// {
//     displayRotation();
// }

/**
 * @brief
 *
 */
// void MainWindow::getScale(int s)
// {
//     scale = s;
//     displayRotation();
// }





MainWindow::~MainWindow()
{
    //     emit mk1Disconnected();
    //
    //     libusb_exit(context);
    //
    //     hotplugTimer->stop();
};


Task::StatusTask MainWindow::getStatus()
{
    return currentStatus;
}

/**
 * @brief close event of program, "X"
 *
 */
void MainWindow::closeEvent(QCloseEvent* ce)
{
    if (currentStatus != Task::Stop) {
        MessageBox::exec(this, translate(ID_WARN), translate(ID_MSG_FOR_DISABLE), QMessageBox::Critical);
        ce->ignore();
        return;
    }

    int ans;
    ans = MessageBox::exec(this, translate(ID_EXIT), translate(ID_REALLYQUIT), QMessageBox::Question);

    if (ans == QMessageBox::No) {
        ce->ignore();
        return;
    }

    emit mk1Disconnected();

    if (context != 0) {
        libusb_exit(context);
    }

    hotplugTimer->stop();

    disconnect(mk1, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message -= CncMessage;

    writeSettings();

    delete mk1;

    ce->accept();

    QCoreApplication::quit();
}


/**
 * @brief slot from "exit" menu element
 *
 */
void MainWindow::onExit()
{
    if (currentStatus != Task::Stop) {
        MessageBox::exec(this, translate(ID_WARN), translate(ID_MSG_FOR_DISABLE), QMessageBox::Critical);
        return;
    }

    if (MessageBox::exec(this, translate(ID_EXIT), translate(ID_REALLYQUIT), QMessageBox::Question) == QMessageBox::No) {
        //         ce->ignore();
        return;
    }

    emit mk1Disconnected();

    if (context != 0) {
        libusb_exit(context);
    }

    hotplugTimer->stop();

    disconnect(mk1, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message -= CncMessage;

    writeSettings();

    delete mk1;

    QCoreApplication::quit();
}


/**
 * @brief translate the GUI widgets
 *
 */
void MainWindow::translateGUI()
{
    groupB14->setTitle(translate(ID_BYTE_14));
    groupB15->setTitle(translate(ID_BYTE_15));
    groupB19->setTitle(translate(ID_BYTE_19));

    groupBoxMoving->setTitle(translate(ID_MOVING_TOPOINT));
    groupGenerate->setTitle(translate(ID_GEN_SIGNAL));
    groupPosition->setTitle(translate(ID_COORDINATES));
    //     groupBoxExec->setTitle(translate(ID_GCODE_RUNNING));
    groupManualControl->setTitle(translate(ID_MANUAL_CONTROL));
    groupIndicator->setTitle(translate(ID_DISPL_LIMITS));
    groupVelocity->setTitle(translate(ID_VELOCITY));

    toolRunMoving->setText(translate(ID_RUN));

    QStringList m = translate(ID_MATERIAL_LIST).split("\n");

    if (m.count() > 0) {
        if (Settings::cuttedMaterial < m.count()) {
            labelMaterial->setText(m.at(Settings::cuttedMaterial));
        } else {
            labelMaterial->setText(m.at(0));
        }
    }

    pushClean->setText(translate(ID_CLEAN));
    pushSendSignal->setText(translate(ID_GEN_SIGNAL));
    pushManualControl->setText(translate(ID_MANUAL_CONTROL));

    radioButtonOff->setText(translate(ID_OFF));
    radioButtonHz->setText(translate(ID_HZ));
    radioButtonRC->setText(translate(ID_RC));

    labelPWMVelo->setText(translate(ID_VELO_PWM));
    labelPWMCHan->setText(translate(ID_CHAN_PWM));

    if (enableOpenGL == true) {
        tabWidget->setTabText(0, translate(ID_DATA));
        tabWidget->setTabText(1, translate(ID_3D_VIEW));
        //         tabWidget->setTabText(2, translate(ID_WORKBENCH));
        tabWidget->setTabText(2, translate(ID_DIAGNOSTIC));
        tabWidget->setTabText(3, translate(ID_ADDITIONAL));
        tabWidget->setTabText(4, translate(ID_LOG));
    } else {
        tabWidget->setTabText(0, translate(ID_DATA));
        //         tabWidget->setTabText(1, translate(ID_WORKBENCH));
        tabWidget->setTabText(1, translate(ID_DIAGNOSTIC));
        tabWidget->setTabText(2, translate(ID_ADDITIONAL));
        tabWidget->setTabText(3, translate(ID_LOG));
    }

    //     labelSubmission->setText(translate(ID_SUBMISSION));
    //     labelMoving->setText(translate(ID_MOVING));

    checkEnSpindle->setText(translate(ID_ON_SPINDLE));
    //     checkHWLimits->setText(translate(ID_CHECK_HW_LIMITS));
    checkHomeAtStart->setText(translate(ID_GO_HOME_AT_START));
    checkHomeAtEnd->setText(translate(ID_GO_HOME_AT_END));

    labelVelo->setText(translate(ID_VELO));

    //     labelRotat->setText(translate(ID_ROTATION));

    labelMinX->setText(translate(ID_MIN));
    labelMaxX->setText(translate(ID_MAX));

    //
    labelRunFrom->setText(translate(ID_CURRENT_LINE));
    labelNumVelo->setText(translate(ID_VELO));

    menuFile->setTitle(translate(ID_FILE));
    menuSettings->setTitle(translate(ID_SETTINGS));
    menuController->setTitle(translate(ID_CONTROLLER));
    menuHelp->setTitle(translate(ID_HELP));

    actionOpen->setText(translate(ID_OPEN_FILE));

    if (filesMenu != 0) {
        filesMenu->setTitle( translate(ID_RECENTFILES));
    }

    actionSave->setText(translate(ID_SAVE_GCODE));
    actionExit->setText(translate(ID_EXIT));

    actionFluid->setText(translate(ID_COOLANT));
    actionMist->setText(translate(ID_MIST));

    actionProgram->setText(translate(ID_PROGRAM));
    //     actionOpenGL->setText(translate(ID_OPENGL));
    //     actionConnect->setText(translate(ID_CONNECT));
    //     actionDisconnect->setText(translate(ID_DISCONNECT));
    actionAbout->setText(translate(ID_ABOUT));

    listGCodeWidget->setHorizontalHeaderLabels((QStringList() << translate(ID_COMMAND) << translate(ID_INFO) << translate(ID_STATE)));
}


/**
 * @brief slot for task start after "play" button
 *
 */
void MainWindow::onStartTask()
{
    //     if (mainTaskTimer.isActive()) {
    //         return;    //timer is active, task is running
    //     }

    if (!mk1->isConnected()) {
        MessageBox::exec(this, translate(ID_ERR), translate(ID_MSG_NO_CONN), QMessageBox::Critical);
        return;
    }

    if (gCodeData.count() == 0) {
        // no data
        MessageBox::exec(this, translate(ID_ERR), translate(ID_MSG_NO_DATA), QMessageBox::Critical);
        return;
    }

    int beg = 0;
    int end = 0;

    QItemSelectionModel* selectionModel = listGCodeWidget->selectionModel();

    QModelIndexList selected = selectionModel->selectedRows();

    //if nothing was selected, from begin to end of list
    if (selected.count() == 0) {
        end = listGCodeWidget->rowCount() - 1;
    }

    if (selected.count() == 1) { //selected only one line
        QString msg = translate(ID_QUEST_START_FROMLINE);
        beg = selected.first().row();

        int dlgres =  MessageBox::exec(this, translate(ID_START_PROG), msg.arg(QString::number(beg + 1)), QMessageBox::Question);

        if (dlgres == QMessageBox::Cancel) {
            return;
        }

        end = listGCodeWidget->rowCount() - 1;
    }

    if (selected.count() > 1) { //select lines range
        QString msg = translate(ID_QUEST_START_FROMTOLINE);

        beg = selected.first().row();
        end = selected.count() + beg - 1;
        int dlgr =  MessageBox::exec(this, translate(ID_START_PROG),  msg.arg(QString::number(beg + 1 ))
                                     .arg(QString::number(end + 1)), QMessageBox::Question);

        if (dlgr == QMessageBox::Cancel) {
            return;
        }
    }

    // set task ranges
    Task::lineCodeStart = beg;
    Task::lineCodeEnd = end;
    Task::lineCodeNow = Task::lineCodeStart;

    Task::instrCounter = 0;

#if 0
    Task::instructionStart = -1;
    Task::instructionNow = -1;

    foreach (const GCodeData c, gCodeList) {
        if(c.numberLine == Task::lineCodeStart && Task::instructionStart == -1) { // get the first only
            Task::instructionStart = c.numberInstruct;
        }

        if(c.numberLine == Task::lineCodeEnd) { // up to last element of this line
            Task::instructionEnd = c.numberInstruct;
        }

        if(c.numberLine == Task::lineCodeNow && Task::instructionNow == -1) { // get the first only
            Task::instructionNow = c.numberInstruct;
        }
    }

#endif

    qDebug() << "ranges, lines:" << Task::lineCodeStart << Task::lineCodeEnd /*<< "code" << Task::instructionStart << Task::instructionEnd*/ << "size of code" << gCodeData.count();
    QString s = translate(ID_FROM_TO).arg( Task::lineCodeStart + 1).arg( Task::lineCodeEnd + 1);
    labelTask->setText( s );

    statusProgress->setRange(Task::lineCodeStart, Task::lineCodeEnd);

    groupManualControl->setChecked( false ); // disable manual control

    currentStatus = Task::Start;

    runNextCommand();
}


/**
 * @brief  slot for task pause after "pause" button
 * TODO check this
 */
void MainWindow::onPauseTask()
{
    if (currentStatus == Task::Start) {
        return;    //if not started, do not set pause
    }

    if (currentStatus == Task::Working || currentStatus == Task::Paused) {
        currentStatus = (currentStatus == Task::Paused) ? Task::Working : Task::Paused;
    }

    runNextCommand();
}

/**
 * @brief slot for checkboxes of workbench tab
 */
#if 0
void MainWindow::onCheckBoxWorkbenchLimits()
{
    QCheckBox* c  = static_cast<QCheckBox*>(sender());
    bool state = c->isChecked();

    if (c == checkBoxLimitsXmin) {
        Settings::coord[X].useLimitMin = state;
    }

    if (c == checkBoxLimitsXmax) {
        Settings::coord[X].useLimitMax = state;
    }

    if (c == checkBoxLimitsYmin) {
        Settings::coord[Y].useLimitMin = state;
    }

    if (c == checkBoxLimitsYmax) {
        Settings::coord[Y].useLimitMax = state;
    }

    if (c == checkBoxLimitsZmin) {
        Settings::coord[Z].useLimitMin = state;
    }

    if (c == checkBoxLimitsZmax) {
        Settings::coord[Z].useLimitMax = state;
    }

    if (c == checkBoxLimitsAmin) {
        Settings::coord[A].useLimitMin = state;
    }

    if (c == checkBoxLimitsAmax) {
        Settings::coord[A].useLimitMax = state;
    }
}

/**
 * @brief slot for checkboxes of workbench tab
 */
void MainWindow::onCheckBoxWorkbenchSwap()
{
    QCheckBox* c  = static_cast<QCheckBox*>(sender());
    bool state = c->isChecked();

    // swap directions
    if (c == checkBoxSwapX) {
        Settings::coord[X].invertDirection = state;
    }

    if (c == checkBoxSwapY) {
        Settings::coord[Y].invertDirection = state;
    }

    if (c == checkBoxSwapZ) {
        Settings::coord[Z].invertDirection = state;
    }

    if (c == checkBoxSwapX) {
        Settings::coord[A].invertDirection = state;
    }

    drawWorkbench();
}
#endif
/**
 * @brief slot for task stop after "stop" button
 * TODO check this
 */
void MainWindow::onStopTask()
{
    if (currentStatus == Task::Waiting) {
        return;
    }

    currentStatus = Task::Stop;
    Task::instrCounter = 0;
}


/**
 * @brief slot from refresh GUI timer signal
 *
 */
void MainWindow::onRefreshGUITimer()
{
    refreshElementsForms();
}


/**
 * @brief slot from mk1 controller, new data
 *
 */
void MainWindow::onCncNewData()
{
    if (currentStatus != Task::Stop) {
        runNextCommand();
    }
}


/**
 * @brief
 *
 */
void MainWindow::runNextCommand()
{
    // Velocity from main form
#if 0
    int userSpeedG1 = (int)numVeloSubmission->value();
    int userSpeedG0 = (int)numVeloMoving->value();

    if (Task::lineCodeNow > Task::lineCodeEnd) {
        currentStatus = Task::Stop;
        AddLog(translate(ID_END_TASK_AT) + QDateTime().currentDateTime().toString());

        return;
    }

#endif
    GCodeData gcodeNow;

    if (Task::instrCounter < gCodeData.count()) {
        gcodeNow = gCodeData.at(Task::instrCounter);
    } else {
        currentStatus = Task::Stop;
    }


    // Stop
    if (currentStatus == Task::Stop) {
        bool useHome = checkHomeAtEnd->isChecked();

        if (useHome == true) {
            Settings::coord[X].startPos = doubleSpinHomeX->value();
            Settings::coord[Y].startPos = doubleSpinHomeY->value();
            Settings::coord[Z].startPos = doubleSpinHomeZ->value();
            Settings::coord[A].startPos = 0.0;

            mk1->pack9E(0x05);

            int limits[4] = {(int)Settings::coord[X].maxVeloLimit, (int)Settings::coord[Y].maxVeloLimit, (int)Settings::coord[Z].maxVeloLimit, (int)Settings::coord[A].maxVeloLimit};
            mk1->packBF(limits); // set max velocities

            mk1->packC0();

            //moving to the first point axes X and Y
            //TODO: spindle move higher, now 10 mm
            moveParameters mParams;
            mParams.pos[X] = Settings::coord[X].startPos;
            mParams.pos[Y] = Settings::coord[Y].startPos;
            mParams.pos[Z] = Settings::coord[Z].startPos + 10.0;
            mParams.pos[A] = Settings::coord[A].startPos;//, userSpeedG0;
            mParams.speed = 0;//gcodeNow.vectSpeed;
            mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.movingCode;
            mParams.restPulses = 0;//gcodeNow.stepsCounter;
            mParams.numberInstruction = Task::instrCounter++;

            mk1->packCA(&mParams); // move to init position
        }

        //TODO: move spindle up, possible moving to "home" position

        mk1->packFF();

        mk1->pack9D();

        mk1->pack9E(0x02);

        mk1->packFF();

        mk1->packFF();

        mk1->packFF();

        mk1->packFF();

        mk1->packFF();

        AddLog(translate(ID_END_TASK_AT) + QDateTime().currentDateTime().toString());

        return;
    }


    // Start
    if (currentStatus == Task::Start) { // init of controller
        bool useHome = checkHomeAtStart->isChecked();

        if (useHome == true) {
            Settings::coord[X].startPos = doubleSpinHomeX->value();
            Settings::coord[Y].startPos = doubleSpinHomeY->value();
            Settings::coord[Z].startPos = doubleSpinHomeZ->value();
            Settings::coord[A].startPos = 0.0;
        } else {
            Settings::coord[X].startPos = Settings::coord[X].actualPosmm;
            Settings::coord[Y].startPos = Settings::coord[Y].actualPosmm;
            Settings::coord[Z].startPos = Settings::coord[Z].actualPosmm;
            Settings::coord[A].startPos = 0.0;
        }

        AddLog(translate(ID_START_TASK_AT) + QDateTime().currentDateTime().toString());

        mk1->pack9E(0x05);

        int limits[4] = {(int)Settings::coord[X].maxVeloLimit, (int)Settings::coord[Y].maxVeloLimit, (int)Settings::coord[Z].maxVeloLimit, (int)Settings::coord[A].maxVeloLimit};
        mk1->packBF(limits); // set max velocities

        mk1->packC0();

        //moving to the first point axes X and Y
        //TODO: spindle move higher, now 10 mm
        moveParameters mParams;
        mParams.pos[X] = Settings::coord[X].startPos;
        mParams.pos[Y] = Settings::coord[Y].startPos;
        mParams.pos[Z] = Settings::coord[Z].startPos + 10.0;
        mParams.pos[A] = Settings::coord[A].startPos;//, userSpeedG0;
        mParams.speed = 0;//gcodeNow.vectSpeed;
        mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.movingCode;
        mParams.restPulses = 0;//gcodeNow.stepsCounter;
        mParams.numberInstruction = 0;

        mk1->packCA(&mParams); // move to init position

        mParams.pos[X] = gcodeNow.xyz.x();
        mParams.pos[Y] = gcodeNow.xyz.y();
        mParams.pos[Z] = gcodeNow.xyz.z() + 10.0;
        mParams.pos[A] = gcodeNow.abc.x();//, userSpeedG0;
        mParams.speed = gcodeNow.vectSpeed;
        mParams.movingCode = gcodeNow.movingCode;
        mParams.restPulses = gcodeNow.stepsCounter;
        mParams.numberInstruction = Task::instrCounter;

        mk1->packCA(&mParams); // move to init position

        currentStatus = Task::Working;

        return; //after start code
    }


    // Working

    if (currentStatus != Task::Working) {
        return;
    }

    // the task is ready
    //     if (Task::posCodeNow > Task::posCodeEnd) {
    //         currentStatus = Stop;
    //         AddLog(translate(ID_END_TASK_AT) + QDateTime().currentDateTime().toString());
    //
    //         //         mainTaskTimer.stop();
    //         return false;
    //     }


    //TODO: to add in parameter the value
    if (mk1->availableBufferSize() <= 3) {
        return;    // nothing before buffer clean
    }

#if 1

    //TODO: to add in parameter the value
    if (Task::instrCounter > (mk1->numberCompleatedInstructions() + mk1->availableBufferSize())) {
        return;    // don't send more than N commands
    }

#endif

    //     qDebug() << "buff size free: " << mk1->availableBufferSize() - 3 << "current instruction: " << Task::instrCounter << "compleate instructions: " << mk1->numberCompleatedInstructions();

    //command G4 or M0
    if (gcodeNow.pauseMSeconds != -1) {
        if (gcodeNow.pauseMSeconds == 0) { // M0 - waiting command
            currentStatus = Task::Paused;

            //pause before user click
            MessageBox::exec(this, translate(ID_PAUSE), translate(ID_RECIEVED_M0), QMessageBox::Information);
        } else {
            QString msg = translate(ID_PAUSE_G4);
            statusLabel2->setText( msg.arg(QString::number(gcodeNow.pauseMSeconds)));

            QThread().wait(gcodeNow.pauseMSeconds); // pause in msec

            statusLabel2->setText( "" );
        }
    }

    //replace instrument
    if (gcodeNow.changeInstrument) {
        currentStatus = Task::Paused;

        //pause before user click
        QString msg = translate(ID_PAUSE_ACTIVATED);
        MessageBox::exec(this, translate(ID_PAUSE), msg.arg(QString::number(gcodeNow.numberInstrument)).arg(QString::number(gcodeNow.diametr)), QMessageBox::Information);
    }

    int commands = 1;

    if (Task::instrCounter > mk1->numberCompleatedInstructions()) {
        commands = mk1->availableBufferSize() - 3;
        //         commands = (Task::instrCounter - (mk1->numberCompleatedInstructions() - mk1->availableBufferSize()));
    }

    for (int i = 0; i < commands; i++) {
        float pointX = gcodeNow.xyz.x();
        float pointY = gcodeNow.xyz.y();
        float pointZ = gcodeNow.xyz.z();
        float pointA = gcodeNow.abc.x();

        Task::lineCodeNow = gcodeNow.numberLine;

        //moving in G-code
        if (Correction) {
            // proportion
            pointX *= coeffSizeX;
            pointY *= coeffSizeY;

            // move
            pointX += deltaX;
            pointY += deltaY;
            pointZ += deltaZ;

            // surface matrix?
            if (deltaFeed) {
                pointZ += getDeltaZ(pointX, pointY);
            }
        }

        {
            //TODO: additional velocity control manual/automatical
            //     int speed = (gcodeNow.workspeed) ? userSpeedG1 : userSpeedG0;
            moveParameters mParams;
            mParams.pos[X] = pointX;
            mParams.pos[Y] = pointY;
            mParams.pos[Z] = pointZ;
            mParams.pos[A] = pointA;//, userSpeedG0;
            mParams.speed = gcodeNow.vectSpeed;
            mParams.movingCode = gcodeNow.movingCode; //
            mParams.restPulses = gcodeNow.stepsCounter;//

            mParams.numberInstruction = Task::instrCounter++;

            gcodeNow.numberInstruction = mParams.numberInstruction;

            mk1->packCA(&mParams); // move to init position

            if (Task::instrCounter < gCodeData.count()) {
                gcodeNow = gCodeData.at(Task::instrCounter);
            } else {
                currentStatus = Task::Stop;
                break;
            }
        }
    }

    labelRunFrom->setText( translate(ID_CURRENT_LINE) + " " + QString::number(Task::lineCodeNow + 1));
}



/**
 * @brief log output
 *
 */
void MainWindow::AddLog(QString _text)
{
    if (_text == NULL) {
        return;
    }

    textLog->append(_text);
}


/**
 * @brief slot for cleaning of status label
 *
 */
void MainWindow::onCleanStatus()
{
    // clean message
    statusLabel2->setText( "" );
}


/**
 * @brief manual moving, or surface scan
 *
 */
void MainWindow::moveToPoint(bool surfaceScan)
{
    int speed = 0;
    float pos[4];//X, posY, posZ, posA;

    if (mk1->isConnected() == false) {
        return;
    }

    if (surfaceScan == true) {
        speed = 200;

        if (scanPosY == -1 || scanPosX == -1) {
            return;
        }

        pos[X] = surfaceMatrix[scanPosY][scanPosX].pos[X];
        pos[Y] = surfaceMatrix[scanPosY][scanPosX].pos[Y];
        pos[Z] = surfaceMatrix[scanPosY][scanPosX].pos[Z];
        pos[A] = surfaceMatrix[scanPosY][scanPosX].pos[A];
    } else {
        speed = spinMoveVelo->value();

        pos[X] = doubleSpinMoveX->value();
        pos[Y] = doubleSpinMoveY->value();
        pos[Z] = doubleSpinMoveZ->value();
        pos[A] = numAngleGrad->value();
    }

    mk1->pack9E(0x05);
    int limits[4] = {speed, speed, speed, speed};
    mk1->packBF(limits);

    mk1->packC0();

    {
        moveParameters mParams;
        mParams.pos[X] = pos[X];
        mParams.pos[Y] = pos[Y];
        mParams.pos[Z] = pos[Z];
        mParams.pos[A] = pos[A];//, userSpeedG0;
        mParams.speed = speed;
        mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.movingCode;
        mParams.restPulses = 0;//gcodeNow.stepsCounter;
        mParams.numberInstruction = 0;

        mk1->packCA(&mParams);
    }

    mk1->packFF();

    mk1->pack9D();

    mk1->pack9E(0x02);

    mk1->packFF();

    mk1->packFF();

    mk1->packFF();

    mk1->packFF();

    mk1->packFF();
}


/**
 * @brief slot for moving to the point
 *
 */
void MainWindow::onRunToPoint()
{
    if (!mk1->testAllowActions()) {
        return;
    }

    mk1->sendSettings();

    moveToPoint();
}


/**
 * @brief DEBUGGING generator PWM
 *
 */
void MainWindow::SendSignal()
{
    mk1Data::TypeSignal tSign;

    tSign = mk1Data::None;

    if (radioButtonHz->isChecked()) {
        tSign = mk1Data::Hz;
    }

    if (radioButtonRC->isChecked()) {
        tSign = mk1Data::RC;
    }

    if (checkEnSpindle->isChecked()) {
        mk1->spindleON();
    } else {
        mk1->spindleOFF();
    }

    mk1->packB5(checkEnSpindle->isChecked(), (int)spinBoxChann->value(), tSign, (int)spinBoxVelo->value());
}

/**
 * @brief slot for sending of one signal
 *
 */
void MainWindow::onSendCommand()
{
    SendSignal();
}


// void MainWindow::onLikePoint()
// {
//     //TODO: open popup with list of points
// }


/**
 * @brief add in the status bar of three widgets
 *
 */
void MainWindow::addStatusWidgets()
{
    //
    statusLabel1 = new QLabel();
    statusLabel1->setFixedWidth(250);
    statusLabel1->setFixedHeight(17);
    statusbar->addWidget(statusLabel1);

    statusProgress = new QProgressBar();
    statusProgress->setFixedWidth(200);
    statusProgress->setFixedHeight(17);
    statusbar->addWidget(statusProgress);

    statusLabel2 = new QLabel();
    statusLabel2->setFixedWidth(250);
    statusLabel2->setFixedHeight(17);
    statusbar->addPermanentWidget(statusLabel2);
}


/**
 * @brief
 *
 */
void MainWindow::onGeneratorCode()
{
    //     GeneratorCodeDialog *frm = new GeneratorCodeDialog(this);
    //     frm->exec()
}


/**
 * @brief linear interpolation for z coordinate from scan surface matrix
 *
 */
float MainWindow::getDeltaZ(float _x, float _y)
{
    //point to calculate
    coord pResult = {_x, _y, 0.0, 0.0}; //new dobPoint(_x, _y, 0);

    int indexXmin = 0;
    int indexYmin = 0;
    int sizeY = surfaceMatrix.size();

    if (sizeY == 0) {
        return 0.0;
    }

    int sizeX = surfaceMatrix[0].size(); // because of rectangle matrix

    if (sizeX == 0) {
        return 0.0;
    }

    for (int y = 0; y < sizeY - 1; y++) {
        for (int x = 0; x < sizeX - 1; x++) {
            if ((_x > surfaceMatrix[y][0].pos[X]) && (_x < surfaceMatrix[y + 1][0].pos[X]) && (surfaceMatrix[0][x].pos[Y] < _y) && (surfaceMatrix[0][x + 1].pos[Y] > _y)) {
                indexXmin = x;
                indexYmin = y;
            }
        }
    }

    coord p1 = {surfaceMatrix[indexYmin][indexXmin].pos[X], surfaceMatrix[indexYmin][indexXmin].pos[Y], surfaceMatrix[indexYmin][indexXmin].pos[Z], 0.0};
    coord p3 = {surfaceMatrix[indexYmin + 1][indexXmin].pos[X], surfaceMatrix[indexYmin + 1][indexXmin].pos[Y], surfaceMatrix[indexYmin + 1][indexXmin].pos[Z], 0.0};
    coord p2 = {surfaceMatrix[indexYmin][indexXmin + 1].pos[X], surfaceMatrix[indexYmin][indexXmin + 1].pos[Y], surfaceMatrix[indexYmin][indexXmin + 1].pos[Z], 0.0};
    coord p4 = {surfaceMatrix[indexYmin + 1][indexXmin + 1].pos[X], surfaceMatrix[indexYmin + 1][indexXmin + 1].pos[Y], surfaceMatrix[indexYmin + 1][indexXmin + 1].pos[Z], 0.0};

    coord p12 = Geometry::CalcPX(p1, p2, pResult);
    coord p34 = Geometry::CalcPX(p3, p4, pResult);
    coord p1234 = Geometry::CalcPY(p12, p34, pResult);

    return p1234.pos[Z];
}


/**
 * @brief slot for logging signal
 *
 */
void MainWindow::onCncMessage(int n_msg)
{
    textLog->append(QDateTime().currentDateTime().toString() + " - " + translate(n_msg));
}


#if 0
/**
 * @brief slot for signals from usb connector: detach or hotplug of controller
 *
 */
void MainWindow::onCncHotplug()
{
    bool e = mk1->isConnected();

    if (e == true) {
        AddLog(translate(ID_HOTPLUGED));
    } else {
        AddLog(translate(ID_DETACHED));
    }

    actionInfo->setEnabled(e);

    if (refreshGUITimer->isActive() == false) {
        refreshElementsForms();
    }
}
#endif

/**
 * @brief update the widgtes after new data from microcontroller
 *
 */
void  MainWindow::refreshElementsForms()
{
    bool cncConnected = mk1->isConnected();

    groupPosition->setEnabled( cncConnected );
    groupManualControl->setEnabled( cncConnected );

    // set groupVelocity too?

    actionStop->setEnabled( cncConnected );
    actionSpindle->setEnabled( cncConnected );
    actionMist->setEnabled( cncConnected );
    actionFluid->setEnabled( cncConnected );

    labelSpeed->setText( QString::number(mk1->getSpindleMoveSpeed()) + translate(ID_MM_MIN));
    //     statLabelNumInstr->setText( translate(ID_NUM_INSTR) + QString::number(mk1->numberCompleatedInstructions()));

    if (!cncConnected ) {
        QPixmap grayPix = QPixmap(":/images/ball_gray.png");

        maxXLED->setPixmap( grayPix );
        minXLED->setPixmap( grayPix );
        maxYLED->setPixmap( grayPix );
        minYLED->setPixmap( grayPix );
        maxZLED->setPixmap( grayPix );
        minZLED->setPixmap( grayPix );
        maxALED->setPixmap( grayPix );
        minALED->setPixmap( grayPix );

        toolRun->setEnabled( cncConnected );
        toolPause->setEnabled( cncConnected );
        toolStop->setEnabled( cncConnected );

        // DEBUG
        labelB14B0->setPixmap( grayPix );
        labelB14B1->setPixmap( grayPix );
        labelB14B2->setPixmap( grayPix );
        labelB14B3->setPixmap( grayPix );
        labelB14B4->setPixmap( grayPix );
        labelB14B5->setPixmap( grayPix );
        labelB14B6->setPixmap( grayPix );
        labelB14B7->setPixmap( grayPix );

        labelB15B0->setPixmap( grayPix );
        labelB15B1->setPixmap( grayPix );
        labelB15B2->setPixmap( grayPix );
        labelB15B3->setPixmap( grayPix );
        labelB15B4->setPixmap( grayPix );
        labelB15B5->setPixmap( grayPix );
        labelB15B6->setPixmap( grayPix );
        labelB15B7->setPixmap( grayPix );

        labelB16B0->setPixmap( grayPix );
        labelB16B1->setPixmap( grayPix );
        labelB16B2->setPixmap( grayPix );
        labelB16B3->setPixmap( grayPix );
        labelB16B4->setPixmap( grayPix );
        labelB16B5->setPixmap( grayPix );
        labelB16B6->setPixmap( grayPix );
        labelB16B7->setPixmap( grayPix );

        labelB19B0->setPixmap( grayPix );
        labelB19B1->setPixmap( grayPix );
        labelB19B2->setPixmap( grayPix );
        labelB19B3->setPixmap( grayPix );
        labelB19B4->setPixmap( grayPix );
        labelB19B5->setPixmap( grayPix );
        labelB19B6->setPixmap( grayPix );
        labelB19B7->setPixmap( grayPix );

        return;
    }

    switch (currentStatus) {
        case Task::Start: {
            statusLabel1->setText( translate(ID_START_TASK));
            break;
        }

        case Task::Paused: {
            statusLabel1->setText( translate(ID_PAUSE_TASK));
            break;
        }

        case Task::Stop: {
            statusLabel1->setText( translate(ID_STOP_TASK));
            break;
        }

        case Task::Working: {
            statusLabel1->setText( translate(ID_RUN_TASK));
            break;
        }

        case Task::Waiting: {
            statusLabel1->setText( translate(ID_WAIT));
            break;
        }
    }

    numPosX->display( Settings::coord[X].posMm());
    numPosY->display( Settings::coord[Y].posMm());
    numPosZ->display( Settings::coord[Z].posMm());

    lineXpulses->setText(QString::number(Settings::coord[X].actualPosPulses));
    lineYpulses->setText(QString::number(Settings::coord[Y].actualPosPulses));
    lineZpulses->setText(QString::number(Settings::coord[Z].actualPosPulses));
    lineApulses->setText(QString::number(Settings::coord[A].actualPosPulses));
    lineBpulses->setText(QString::number(Settings::coord[B].actualPosPulses));
    lineCpulses->setText(QString::number(Settings::coord[C].actualPosPulses));
    lineUpulses->setText(QString::number(Settings::coord[U].actualPosPulses));
    lineVpulses->setText(QString::number(Settings::coord[V].actualPosPulses));
    lineWpulses->setText(QString::number(Settings::coord[W].actualPosPulses));

    lineInstructions->setText(QString::number(mk1->numberCompleatedInstructions()));

#if 0

    if (mk1->isEmergencyStopOn()) {
        actionStop->setStyleSheet("" );
        QPalette palette = actionStop->palette();
        palette.setColor(actionStop->backgroundRole(), Qt::red);
        palette.setColor(actionStop->foregroundRole(), Qt::white);
        actionStop->setPalette(palette);
        //         actionStop->BackColor = Color.Red;
        //         actionStop->ForeColor = Color.White;
    } else {
        actionStop->BackColor = Color.FromName("Control" );
        actionStop->ForeColor = Color.Black;
    }

    // QLabel* pLabel = new QLabel;
    // pLabel->setStyleSheet("QLabel { background-color : red; color : blue; }" );

    if (mk1->isSpindelOn()) {
        actionSpindle->BackColor = Color.Green;
        actionSpindle->ForeColor = Color.White;
    } else {
        actionSpindle->BackColor = Color.FromName("Control" );
        actionSpindle->ForeColor = Color.Black;
    }

#endif

    QPixmap greenPix = QPixmap(":/images/ball_green.png");
    QPixmap redPix = QPixmap(":/images/ball_red.png");

    // main indicators
    maxXLED->setPixmap( Settings::coord[X].actualLimitMin  ? redPix : greenPix );
    minXLED->setPixmap( Settings::coord[X].actualLimitMax  ? redPix : greenPix );
    maxYLED->setPixmap( Settings::coord[Y].actualLimitMin  ? redPix : greenPix );
    minYLED->setPixmap( Settings::coord[Y].actualLimitMax  ? redPix : greenPix );
    maxZLED->setPixmap( Settings::coord[Z].actualLimitMin  ? redPix : greenPix );
    minZLED->setPixmap( Settings::coord[Z].actualLimitMax  ? redPix : greenPix );
    maxALED->setPixmap( Settings::coord[A].actualLimitMin  ? redPix : greenPix );
    minALED->setPixmap( Settings::coord[A].actualLimitMax  ? redPix : greenPix );

    //***************

    //DEBUG:
    labelB14B0->setPixmap( Settings::bb14 & (1 << 0) ? redPix : greenPix );
    labelB14B1->setPixmap( Settings::bb14 & (1 << 1) ? redPix : greenPix );
    labelB14B2->setPixmap( Settings::bb14 & (1 << 2) ? redPix : greenPix );
    labelB14B3->setPixmap( Settings::bb14 & (1 << 3) ? redPix : greenPix );
    labelB14B4->setPixmap( Settings::bb14 & (1 << 4) ? redPix : greenPix );
    labelB14B5->setPixmap( Settings::bb14 & (1 << 5) ? redPix : greenPix );
    labelB14B6->setPixmap( Settings::bb14 & (1 << 6) ? redPix : greenPix );
    labelB14B7->setPixmap( Settings::bb14 & (1 << 7) ? redPix : greenPix );


    labelB15B0->setPixmap( Settings::coord[X].actualLimitMin  ? redPix : greenPix );
    labelB15B1->setPixmap( Settings::coord[X].actualLimitMax  ? redPix : greenPix );
    labelB15B2->setPixmap( Settings::coord[Y].actualLimitMin  ? redPix : greenPix );
    labelB15B3->setPixmap( Settings::coord[Y].actualLimitMax  ? redPix : greenPix );
    labelB15B4->setPixmap( Settings::coord[Z].actualLimitMin  ? redPix : greenPix );
    labelB15B5->setPixmap( Settings::coord[Z].actualLimitMax  ? redPix : greenPix );
    labelB15B6->setPixmap( Settings::coord[A].actualLimitMin  ? redPix : greenPix );
    labelB15B7->setPixmap( Settings::coord[A].actualLimitMax  ? redPix : greenPix );


    labelB19B0->setPixmap( Settings::bb19 & (1 << 0) ? redPix : greenPix );
    labelB19B1->setPixmap( Settings::bb19 & (1 << 1) ? redPix : greenPix );
    labelB19B2->setPixmap( Settings::bb19 & (1 << 2) ? redPix : greenPix );
    labelB19B3->setPixmap( Settings::bb19 & (1 << 3) ? redPix : greenPix );
    labelB19B4->setPixmap( Settings::bb19 & (1 << 4) ? redPix : greenPix );
    labelB19B5->setPixmap( Settings::bb19 & (1 << 5) ? redPix : greenPix );
    labelB19B6->setPixmap( Settings::bb19 & (1 << 6) ? redPix : greenPix );
    labelB19B7->setPixmap( Settings::bb19 & (1 << 7) ? redPix : greenPix );

    // end debug

    // bttons start/stop/pause of task
    frameExec->setEnabled( cncConnected );

    if (cncConnected) {
#if 0

        if (mainTaskTimer.isActive()) {
            toolRun->setEnabled( false );

            if (currentStatus == Paused) {
                toolStop->setEnabled(false);
                toolPause->setEnabled( true);
            } else {
                toolStop->setEnabled( true);
                toolPause->setEnabled( true );
            }
        } else { //таймер выполнения задания выключен
            toolRun->setEnabled(true);
            toolStop->setEnabled(false);
            toolPause->setEnabled(false);
        }

#endif

        if (currentStatus == Task::Waiting) {
            toolResetCoorX->setEnabled( true );
//             numPosX->setReadOnly( true );

            toolResetCoorY->setEnabled( true );
//             numPosY->setReadOnly( true );

            toolResetCoorZ->setEnabled( true );
//             numPosZ->setReadOnly( true );

            toolResetCoorA->setEnabled( true );
//             numAngleGrad->setReadOnly( true );
        } else {
            toolResetCoorX->setEnabled( false );
//             numPosX->setReadOnly( false );

            toolResetCoorY->setEnabled( false );
//             numPosY->setReadOnly( false );

            toolResetCoorZ->setEnabled( false );
//             numPosZ->setReadOnly( false );

            toolResetCoorA->setEnabled( false );
//             numAngleGrad->setReadOnly( false );
        }

        if (currentStatus == Task::Working) {
            int complectaed = mk1->numberCompleatedInstructions();
            int lineNum = 0;

            // TODO to link with line number
            foreach (const GCodeData v, gCodeData) {
                if (v.numberInstruction > complectaed) {
                    break;
                }

                lineNum = v.numberLine;
            }

            statusProgress->setValue( lineNum );

            //listGkodeForUser.Rows[mk1->NumberComleatedInstructions].Selected = true;
            //TODO: to overwork it, because of resetting of selected ragne
            //listGCodeWidget->currentIndex() = mk1->NumberComleatedInstructions;
            toolRun->setEnabled( false );
            toolStop->setEnabled( true);
            toolPause->setEnabled( true );
        }

        if (currentStatus == Task::Stop) {
            toolRun->setEnabled(true);
            toolStop->setEnabled(false);
            toolPause->setEnabled(false);

            toolResetCoorX->setEnabled( true );
//             numPosX->setReadOnly( true );

            toolResetCoorY->setEnabled( true );
//             numPosY->setReadOnly( true );

            toolResetCoorZ->setEnabled( true );
//             numPosZ->setReadOnly( true );

            toolResetCoorA->setEnabled( true );
//             numAngleGrad->setReadOnly( true );
        }

        if (currentStatus == Task::Paused) {
            toolRun->setEnabled( false );
            toolStop->setEnabled(false);
            toolPause->setEnabled( true);
        }

        //         if (enableOpenGL == true) {
        //             scene3d->Draw();
        //         }
    } else {
        toolRun->setEnabled( cncConnected );
        toolPause->setEnabled( cncConnected );
        toolStop->setEnabled( cncConnected );

        toolResetCoorX->setEnabled( false );
//         numPosX->setReadOnly( false );

        toolResetCoorY->setEnabled( false );
//         numPosY->setReadOnly( false );

        toolResetCoorZ->setEnabled( false );
//         numPosZ->setReadOnly( false );

        toolResetCoorA->setEnabled( false );
//         numAngleGrad->setReadOnly( false );
    }
}


/**
 * @brief slot for popup window for manual moving of device
 *
 */
void MainWindow::onManualControlDialog()
{
    SettingsDialog *sd = new SettingsDialog(this, 3);
    sd->exec();
    delete sd;
    //     ManualControlDialog *mc = new ManualControlDialog(this);
    //     mc->exec();
    //     delete mc;
}


/**
 * @brief fill the table widget with text data
 *
 */
void MainWindow::fillListWidget(QVector<QString> listCode)
{
    listGCodeWidget->clear();
    listGCodeWidget->setRowCount( 0);
    listGCodeWidget->setColumnCount(3);

    QStringList header = (QStringList() << translate(ID_COMMAND) << translate(ID_INFO) << translate(ID_STATE));

    listGCodeWidget->setHorizontalHeaderLabels(header);
    listGCodeWidget->setRowCount( listCode.count() );

    for (int i = 0; i < listCode.count(); i++) {
        QString valueStr = listCode.at(i);

        QTableWidgetItem *newItem = new QTableWidgetItem(valueStr);
        newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable); // set read only

        listGCodeWidget->setItem(i, 0, newItem);

        for (int j = 1; j < 3; j++) { // set other elements read only
            QTableWidgetItem *it = new QTableWidgetItem();
            it->setFlags(it->flags() ^ Qt::ItemIsEditable);
            listGCodeWidget->setItem(i, j, it);
        }
    }

    listGCodeWidget->resizeColumnsToContents();

    Task::lineCodeStart = 0;
    Task::lineCodeEnd = listGCodeWidget->rowCount();

    tabWidget->setCurrentIndex(0);

    statusProgress->setRange(0, listGCodeWidget->rowCount());
    statusProgress->setValue(0);

    fixGCodeList();
}


/**
 * @brief function patches the data list before sending to mk1
 *
 * the data list will be patched dependend from current user settings:
 * speed, steps per mm and other. we need to patch data in case of settings changing
 */
void MainWindow::fixGCodeList()
{
    if (gCodeData.count() < 2) {
        return;
    }

    // grad to rad
    maxLookaheadAngleRad = Settings::maxLookaheadAngle * PI / 180.0;

    // calculate the number of steps in one direction, if exists
    for (int idx = 0; idx < gCodeData.size(); idx++) {
        if (gCodeData[idx].movingCode == RAPID_LINE_CODE) {
            continue;
        }

        int endPos = calculateMinAngleSteps(idx); // and update the pos

        if (endPos >= 1) {
            patchSpeedAndAccelCode(idx, endPos);
            idx = endPos;
            continue;
        }
    }

#if 0

    // now debug
    foreach (const GCodeData d, gCodeList) {
        qDebug() << "line:" << d.numberLine << "accel:" << (hex) << d.movingCode << (dec) << "max coeff:" << d.vectorCoeff << "splits:" <<  d.splits
                 << "steps:" << d.stepsCounter << "vector speed:" << d.vectSpeed << "coords:" << d.X << d.Y << "delta angle:" << d.deltaAngle;
    }

    qDebug() << "max delta angle: " << PI - maxLookaheadAngleRad;
#endif
}


/**
 * @brief function set the vector speed and acceleration code before sending data to controller
 *
 * before sending data to microcontroller we need to calculate the vector speed and acceleration code
 * acceleration codes: ACCELERAT_CODE, DECELERAT_CODE, CONSTSPEED_CODE or FEED_LINE_CODE
 *
 * gCodeData [begPos .. endPos]
 *
 * @param[in] begPos from this position in gcode list
 * @param[in] endPos inclusively end position
 *
 */
void MainWindow::patchSpeedAndAccelCode(int begPos, int endPos)
{
    if (begPos < 1 || begPos >= gCodeData.count() - 1) {
        qDebug() << "wrong position number patchSpeedAndAccelCode()" << begPos;
        return;
    }

    if (begPos == endPos) {
        return;
    }

    int sumSteps = 0;

    float dnewSpdX  = 3600; // 3584?
    float dnewSpdY  = 3600; // 3584?
    float dnewSpdZ  = 3600; // 3584?

    // TODO to calculate this only after settings changing
    if ((Settings::coord[X].maxVeloLimit != 0.0) && (Settings::coord[X].pulsePerMm != 0.0)) {
        dnewSpdX = 7.2e8 / ((float)Settings::coord[X].maxVeloLimit * Settings::coord[X].pulsePerMm);
    }

    if ((Settings::coord[Y].maxVeloLimit != 0.0) && (Settings::coord[Y].pulsePerMm != 0.0)) {
        dnewSpdY = 7.2e8 / ((float)Settings::coord[Y].maxVeloLimit * Settings::coord[Y].pulsePerMm);
    }

    if ((Settings::coord[Z].maxVeloLimit != 0.0) && (Settings::coord[Z].pulsePerMm != 0.0)) {
        dnewSpdZ = 7.2e8 / ((float)Settings::coord[Z].maxVeloLimit * Settings::coord[Z].pulsePerMm);
    }

    switch (gCodeData[begPos].plane) {
        case XY: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {

                float dX = qFabs(gCodeData.at(i - 1).xyz.x() - gCodeData.at(i).xyz.x());
                float dY = qFabs(gCodeData.at(i - 1).xyz.y() - gCodeData.at(i).xyz.y());
                float dH = qSqrt(dX * dX + dY * dY);
                float coeff = 1.0;

                if (dX > dY) {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    // calculation of vect speed
                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeData[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                } else {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeData[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                }

                sumSteps += gCodeData[i].stepsCounter;

                gCodeData[i].vectorCoeff = coeff;
            }

            break;
        }

        case YZ: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dY = qFabs(gCodeData.at(i - 1).xyz.y() - gCodeData.at(i).xyz.y());
                float dZ = qFabs(gCodeData.at(i - 1).xyz.z() - gCodeData.at(i).xyz.z());
                float dH = qSqrt(dZ * dZ + dY * dY);
                float coeff = 1.0;

                if (dY > dZ) {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeData[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                } else {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeData[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                }

                sumSteps += gCodeData[i].stepsCounter;

                gCodeData[i].vectorCoeff = coeff;
            }

            break;
        }

        case ZX: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                float dZ = qFabs(gCodeData.at(i - 1).xyz.z() - gCodeData.at(i).xyz.z());
                float dX = qFabs(gCodeData.at(i - 1).xyz.x() - gCodeData.at(i).xyz.x());
                float dH = qSqrt(dX * dX + dZ * dZ);
                float coeff = 1.0;

                if (dZ > dX) {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeData[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                } else {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    gCodeData[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeData[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                }

                sumSteps += gCodeData[i].stepsCounter;

                gCodeData[i].vectorCoeff = coeff;
            }

            break;
        }

        default: {
            qDebug() << "no plane information: pos " << begPos << "x" << gCodeData[begPos].xyz.x() << "y" << gCodeData[begPos].xyz.y() << "z" << gCodeData[begPos].xyz.z();
        }
    }

    if (sumSteps > 0) {
        // now for steps
        for (int i = begPos; i < endPos; i++) {
            int tmpStps;
            tmpStps = gCodeData[i].stepsCounter;
            gCodeData[i].stepsCounter = sumSteps;
            sumSteps -= tmpStps;
            gCodeData[i].movingCode = CONSTSPEED_CODE;
        }

        gCodeData[begPos].movingCode = ACCELERAT_CODE;
        gCodeData[endPos].movingCode = DECELERAT_CODE;
    }
}


/**
 * @brief function determines, how many steps from actual position the g-code object has to the last point with angle up to maxLookaheadAngleRad
 *
 * the angle maxLookaheadAngle is recommended from 150 to 179 grad. it well be converted to radians
 *
 * @param[in] startPos begin pos of searching
 *
 * @return the end position of polygon with angle difference less than maxLookaheadAngle
 */
int MainWindow::calculateMinAngleSteps(int startPos)
{
    int idx = startPos;

    if (startPos > gCodeData.count() - 1 || startPos < 1) {
        qDebug() << "steps counter bigger than list";
        return -1;
    }

#if 1

    if (gCodeData.at(startPos).splits > 0) { // it's arc, splits inforamtion already calculated
        idx += gCodeData.at(startPos).splits;
        return idx;
    }

#endif

    // or for lines
    for (idx = startPos; idx < gCodeData.count() - 1; idx++) {
#if 1

        if (gCodeData.at(idx).movingCode == ACCELERAT_CODE && gCodeData.at(idx).splits > 0) {
            idx += gCodeData.at(idx).splits;
            return idx;
        }

#endif

        if (gCodeData.at(idx + 1).movingCode == RAPID_LINE_CODE) {
            return idx;
        }

#if 0
        qDebug() << "found diff accel code" << startPos << idx << (hex) << gCodeList.at(idx).movingCode << gCodeList[idx + 1].movingCode
                 << "coordinates" << (dec) << gCodeList.at(idx).X << gCodeList.at(idx).Y << gCodeList[idx + 1].X << gCodeList[idx + 1].Y;
#endif

        float a1 = gCodeData.at(idx).angle;
        float a2 = gCodeData.at(idx + 1).angle;

        gCodeData[idx].deltaAngle = (a1 - a2);

        if (qFabs(gCodeData[idx].deltaAngle) > qFabs(PI - maxLookaheadAngleRad)) {
            break;
        }
    }

#if 0

    if ((idx - startPos) != 0) {
        gCodeList[startPos].splits = idx - startPos;
        qDebug() << "found in pos:" << startPos << ", steps: " << idx - startPos << " from" << gCodeList[startPos].X << gCodeList[startPos].Y  << "to" << gCodeList[idx].X << gCodeList[idx].Y;// << dbg;
    }

#endif

    return idx;
}


/**
 * @brief save the g-code file
 *
 */
void MainWindow::onSaveFile()
{
    //     reader->writeFile();
}


/**
 * @brief open g-code file
 *
 */
void MainWindow::onOpenFile()
{
    QString nm;

    statusProgress->setValue(0);

    if (OpenFile(nm) == false) {
        AddLog("File loading error: " + nm );
        return;
    }
}



/**
 * @brief slot for calling of feed speed calculation pop up window
 *
 */
void MainWindow::onCalcVelocity()
{
    SettingsDialog *sd = new SettingsDialog(this, 5);
    sd->exec();
    delete sd;

    QStringList m = translate(ID_MATERIAL_LIST).split("\n");

    if (Settings::cuttedMaterial < m.count()) {
        labelMaterial->setText(m.at(Settings::cuttedMaterial));
    } else {
        labelMaterial->setText(m.at(0));
    }

    //     numVeloSubmission->setValue(Settings::veloCutting);
#if 0
    CuttingCalc *setfrm = new CuttingCalc(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        writeSettings();
        QStringList m = translate(ID_MATERIAL_LIST).split("\n");

        if (m.count() > 0) {
            if (Settings::cuttedMaterial < m.count()) {
                labelMaterial->setText(m.at(Settings::cuttedMaterial));
            } else {
                labelMaterial->setText(m.at(0));
            }
        }

        numVeloSubmission->setValue(Settings::veloCutting);
    }

#endif
}


/**
 * @brief slot for popup window and saving of settings file
 *
 */
void MainWindow::onSettings()
{
    SettingsDialog *setfrm = new SettingsDialog(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        mk1->sendSettings();
        writeSettings();

        if (enableOpenGL == true) {
            scene3d->initStaticElements();
        }
    }
}


/**
 * @brief slot for copying of actual coordinates into home
 *
 */
void MainWindow::onCopyHome()
{
    doubleSpinHomeX->setValue( numPosX->value() );
    doubleSpinHomeY->setValue( numPosY->value() );
    doubleSpinHomeZ->setValue( numPosZ->value() );
}


/**
 * @brief // TODO
 *
 */
void MainWindow::onSetHome()
{
}


/**
 * @brief slot for copying of actual coordinates into the "move to coordinates group"
 *
 */
void MainWindow::onCopyPos()
{
    doubleSpinMoveX->setValue( numPosX->value() );
    doubleSpinMoveY->setValue( numPosY->value() );
    doubleSpinMoveZ->setValue( numPosZ->value() );
}


/**
 * @brief slot for cleaning of logging widget
 *
 */
void MainWindow::onLogClear()
{
    textLog->clear();
}


/**
 * @brief slot for about popup window
 *
 */
void MainWindow::onAbout()
{
    AboutDialog *dlg = new AboutDialog(this);
    dlg->exec();

    delete dlg;
}


/**
 * @brief slot for enabling/disabling of mist coolant on mk1
 *
 */
void MainWindow::onMist()
{
    if (mk1->isMistOn()) {
        actionMist->setIcon(QIcon(QPixmap(":/images/mist_off.png")));
        mk1->mistOFF();
    } else {
        actionMist->setIcon(QIcon(QPixmap(":/images/mist_on.png")));
        mk1->mistON();
    }
}


/**
 * @brief slot for enabling/disabling of fluid coolant on mk1
 *
 */
void MainWindow::onFluid()
{
    if (mk1->isFluidOn()) {
        actionFluid->setIcon(QIcon(QPixmap(":/images/coolant_off.png")));
        mk1->fluidOFF();
    } else {
        actionFluid->setIcon(QIcon(QPixmap(":/images/coolant_on.png")));
        mk1->fluidON();
    }
}

/**
 * @brief slot for enabling/disabling of spindle on mk1
 *
 */
void MainWindow::onSpindel()
{
    if (mk1->isSpindelOn()) {
        actionSpindle->setIcon(QIcon(QPixmap(":/images/forward_off.png")));
        mk1->spindleOFF();
    } else {
        actionSpindle->setIcon(QIcon(QPixmap(":/images/forward_on.png")));
        mk1->spindleON();
    }
}


/**
 * @brief slot for emergy stop on mk1
 *
 */
void MainWindow::onEmergyStop()
{
    mk1->emergyStop();
}

#if 0
/**
 * @brief  slot for 3d settings of program
 *
 */
void MainWindow::on3dSettings()
{


    if (enableOpenGL == true) {
        // 3d settings
        Settings3dDialog *dlg = new Settings3dDialog(this);
        dlg->exec();

        scene3d->updateGL();

        delete dlg;
    }


}
#endif

/**
 * @brief slot for the popup window of scan surface dialog
 *
 */
void MainWindow::onScanSurface()
{
    Settings::ShowSurface = true;

    ScanSurfaceDialog *dlg = new ScanSurfaceDialog(this);
    dlg->exec();

    delete dlg;
}


/**
 * @brief // TODO
 *
 */
void MainWindow::onCellSelect(int row, int col)
{
}


/**
 * @brief
 *
 */
void MainWindow::onEditGCode(int row, int col)
{
    EditGCodeDialog *dlg = new EditGCodeDialog(this);

    dlg->exec();

    delete dlg;
}


/**
 * @brief slot for resetting of "x" coordinate
 *
 */
void MainWindow::onButtonXtoZero()
{
    mk1->deviceNewPosition(0, Settings::coord[Y].actualPosPulses, Settings::coord[Z].actualPosPulses, Settings::coord[A].actualPosPulses);
    numPosX->display(0.0);
}


/**
 * @brief slot for resetting of "y" coordinate
 *
 */
void MainWindow::onButtonYtoZero()
{
    mk1->deviceNewPosition(Settings::coord[X].actualPosPulses, 0, Settings::coord[Z].actualPosPulses, Settings::coord[A].actualPosPulses);
    numPosY->display(0.0);
}


/**
 * @brief slot for resetting of "z" coordinate
 *
 */
void MainWindow::onButtonZtoZero()
{
    mk1->deviceNewPosition(Settings::coord[X].actualPosPulses, Settings::coord[Y].actualPosPulses, 0, Settings::coord[A].actualPosPulses);
    numPosZ->display(0.0);
}


/**
 * @brief slot for log messages from other objects
 *
 */
void MainWindow::logMessage(const QString &msg)
{
    AddLog(msg);
}

/**
 * @brief slot for resetting of "a" angle
 *
 */
void MainWindow::onButtonAtoZero()
{
    mk1->deviceNewPosition(Settings::coord[X].actualPosPulses, Settings::coord[Y].actualPosPulses, Settings::coord[Z].actualPosPulses, 0);
    numAngleGrad->display(0.0);
}


/**
 * @brief slot for popup window
 *
 */
void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
