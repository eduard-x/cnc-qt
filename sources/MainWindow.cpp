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


#include "includes/Settings.h"
#include "includes/About.h"
#include "includes/Reader.h"
#include "includes/CuttingCalc.h"
#include "includes/EditGCode.h"
#include "includes/ManualControl.h"
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
        msgBox->setButtonText(QMessageBox::Yes, translate(_YES));
        msgBox->setButtonText(QMessageBox::No, translate(_NO));
    } else {
        msgBox->setStandardButtons(QMessageBox::Yes);
        msgBox->setButtonText(QMessageBox::Yes, translate(_OK));
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

    setWindowTitle(translate(_PROG_NAME));

    currentStatus = Task::Stop;

    axisNames = "XYZA";

    currentAppDir = qApp->applicationDirPath();

    if (currentAppDir.lastIndexOf("/build") > 0) { // build dir detection
        currentAppDir.remove("/build" );
    }

    QString n = QString::number(1.01);
    Settings::toDecimalPoint = (n.indexOf(",") > 0) ? ',' : '.';
    Settings::fromDecimalPoint = (Settings::toDecimalPoint == ',') ? '.' : ',';


    // detection of remote connection
    enableOpenGL = false;

    QString d = getenv( "DISPLAY" ); // linux machines only!

    // to disable the OpenGL features, if over ssh
    enableOpenGL = (d.indexOf(QRegExp(":[0-9]")) == 0);

    currentLang = "English";

    filesMenu = 0;
    filesGroup = 0;
    //
    QFont sysFont = qApp->font();
    sysFont = sysFont;

    fontSize = sysFont.pointSize();

    reader = new Reader(this);

    userKeys = {
        { "UserAplus", Qt::Key_multiply },
        { "UserAminus", Qt::Key_division },
        { "UserZplus", Qt::Key_Home },
        { "UserZminus", Qt::Key_End },
        { "UserYplus", Qt::Key_Up },
        { "UserYminus", Qt::Key_Down },
        { "UserXplus", Qt::Key_Right },
        { "UserXminus", Qt::Key_Left }
    };

    labelTask->setText("");

    readSettings();

    setWindowIcon(QIcon(QPixmap(":/images/icon.png")));

    programStyleSheet = QString().sprintf("font-size: %dpt", fontSize);

    if ( fontSize == -1) {
        fontSize = sysFont.pixelSize();
        programStyleSheet = QString().sprintf("font-size: %dpx", fontSize);
    }

    if (programStyleSheet.length() > 0) {
        setStyleSheet(programStyleSheet);
    }

    if (readLangDir() == false) { // init from langFiles variable in format "filename:language"
        MessageBox::exec(this, translate(_ERR),
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
        statusLabel2->setText( "OpenGL " + translate(_ENABLED));

        // OpenGL is placed in widget
    } else {
        scene3d = 0;

        QPalette palette = statusLabel2->palette();
        //  palette.setColor(statusLabel2->backgroundRole(), Qt::yellow);
        palette.setColor(statusLabel2->foregroundRole(), Qt::red);
        statusLabel2->setPalette(palette);
        statusLabel2->setText( "OpenGL " + translate(_DISABLED) );
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
        MessageBox::exec(this, translate(_ERR),
                         "Can't open language file!\nDefault GUI language is english", QMessageBox::Critical);

        currentLang = "English";
    }

    foreach (QAction* itL, actLangSelect) {
        if ((*itL).text() == currentLang) {
            (*itL).setChecked(true);
            break;
        }
    }

    QStringList arguments = QCoreApplication::arguments();

    if (arguments.size() > 1) {
        if (arguments.at(1).length() > 0) { // as parameter is file name to load
            QString nm = arguments.at(1);

            if (reader->OpenFile(nm) == true) {

                lastFiles.insert(0, nm);
                lastFiles.removeDuplicates();

                reloadRecentList();

                QVector<QString> l = reader->getGoodList();
                fillListWidget(l);

                gCodeData = reader->getGCodeData();

                if (enableOpenGL == true) {
                    scene3d->loadFigure();
                }

                //                 l = reader->getBadList();
                //
                //                 if (l.count() != 0) {
                //                     foreach (QString s, l) {
                //                         AddLog(s);
                //                     }
                //                 }

                if (gCodeData.count() > 0) {
                    nm.replace(QDir::homePath(), QString("~"));
                    AddLog("File loaded: " + nm);
                }
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

    graphicsView->setScene(sceneCoordinates);
}


/**
 * @brief try to find the translation file
 *
 */
bool MainWindow::getLangTable()
{
    QString lang = currentLang;
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

    if (QFile::exists(langDir + "/" + fileLang) == false) {
        MessageBox::exec(this, translate(_ERR), "Language file not exists!\n\n"
                         + langDir + "\n\n" + fileLang, QMessageBox::Warning);
        // not found
        return (false);
    }

    return loadTranslation(langDir + fileLang);
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


    connect(listGCodeWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onEditGCode(int, int)));
    connect(listGCodeWidget, SIGNAL(cellActivated(int, int)), this, SLOT(onCellSelect(int, int)));

    connect(pushSendSignal, SIGNAL(clicked()), this, SLOT(onSendCommand()));
    connect(toolRunMoving, SIGNAL(clicked()), this, SLOT(onRunToPoint()));

    // connected or disconnected
    connect(mk1, SIGNAL(hotplugSignal ()), this, SLOT(onCncHotplug())); // cnc->WasConnected += CncConnect;
    //     connect(mk1, SIGNAL(hotplugDisconnected ()), this, SLOT(onCncHotplug())); // cnc->WasDisconnected += CncDisconnect;

    connect(mk1, SIGNAL(newDataFromMK1Controller ()), this, SLOT(onCncNewData())); // cnc->NewDataFromController += CncNewData;
    connect(mk1, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message += CncMessage;

    connect(&mainGUITimer, SIGNAL(timeout()), this, SLOT(onRefreshGUITimer()));
    mainGUITimer.start(500);// every 0.5 sec update


    if (enableOpenGL == true) {

        // 3d buttons
        connect(posAngleXm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXm()));
        connect(posAngleX, SIGNAL(clicked()), scene3d, SLOT(onPosAngleX())); // reset to 0
        connect(scene3d, SIGNAL(rotationChanged()), this, SLOT(getRotation()));
        connect(scene3d, SIGNAL(fpsChanged(int)), this, SLOT(getFPS(int)));
        connect(posAngleXp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXp()));

        connect(posAngleYm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYm()));
        connect(posAngleY, SIGNAL(clicked()), scene3d, SLOT(onPosAngleY())); // reset to 0
        //         connect(scene3d, SIGNAL(yRotationChanged(int)), this, SLOT(getYRotation(int)));
        connect(posAngleYp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYp()));

        connect(posAngleZm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZm()));
        connect(posAngleZ, SIGNAL(clicked()), scene3d, SLOT(onPosAngleZ())); // reset to 0
        //         connect(scene3d, SIGNAL(zRotationChanged(int)), this, SLOT(getZRotation(int)));
        connect(posAngleZp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZp()));

        connect(scene3d, SIGNAL(scaleChanged(int)), this, SLOT(getScale(int)));

        connect(pushDefaultPreview, SIGNAL(clicked()), scene3d, SLOT(onDefaulPreview()));
        // end of 3d buttons

    }

    connect(radioFixX, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixY, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));
    connect(radioFixZ, SIGNAL(toggled(bool)), this, SLOT(onChangeFix(bool)));

    connect(actionInfo, SIGNAL(triggered()), this, SLOT(onDeviceInfo()));

    radioFixY->setChecked(true);

    onCncHotplug();
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
    dirsLang << "/usr/share/cnc-qt/" << "/usr/local/share/cnc-qt/" << currentAppDir;

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

    langMenu = menuSettings->addMenu(translate(_LANGUAGE));

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

                    if (currentLang == nm) {
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
 * @brief get the system locale for selection of language, if exists
 *
 */
QString MainWindow::getLocaleString()
{
    QString res;
    QLocale lSys = QLocale::system();

    switch (lSys.language()) {
        case QLocale::C:
            res = "English";
            break;

        case QLocale::German:
            res = "Deutsch";
            break;

        case QLocale::Russian:
            res = "Russian";
            break;

        default:
            res = "English";
            break;
    }

    return res;
}


/**
 * @brief update settings on main gui after change
 *
 */
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


/**
 * @brief save settings of program
 *
 */
void MainWindow::writeSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );

    //     s->beginGroup("General");

    s->setValue("pos", pos());
    s->setValue("size", size());
    //     s->setValue("WorkDir", currentWorkDir);
    s->setValue("LANGUAGE", currentLang);
    s->setValue("LASTDIR", reader->lastDir);

    s->setValue("VelocityCutting", numVeloSubmission->value());
    s->setValue("VelocityMoving", numVeloMoving->value());
    s->setValue("VelocityManual", numVeloManual->value());

    s->setValue("SplitArcPerMM", Settings::splitsPerMm);
    s->setValue("LookaheadAngle", Settings::maxLookaheadAngle);

    s->setValue("UnitMM", unitMm);
    s->setValue("ToolDiameter", toolDiameter);
    s->setValue("ToolFlutes", toolFlutes);
    s->setValue("ToolRPM", toolRPM);

    s->setValue("FilterRepeatData", Settings::filterRepeat);

    s->setValue("CuttedMaterial", cuttedMaterial);

    foreach (uKeys k, userKeys) {
        s->setValue(k.name, (quint32)k.code);
    }

    if (groupManualControl->isChecked() == false) {
        currentKeyPad = -1;
    }

    s->setValue("KeyControl", (int)currentKeyPad);
    //         s->setValue("LASTPROJ", currentProject);
    //     s->setValue("FontSize", fontSize);
    //     s->setValue("GUIFont", sysFont);
    // qDebug() << "writeGUISettings";

    lastFiles.removeDuplicates();

    int i = 0;

    foreach (QString l, lastFiles) {
        if (i > 9) { // max last dirs
            break;
        }

        s->setValue("LASTFILE" + QString::number(i), l);
        i++;
    }

    // opengl settings
    if (enableOpenGL == true) {
        s->beginGroup("OpenGL");

        s->setValue("ShowLines", ShowLines);
        s->setValue("ShowPoints", ShowPoints);

        s->setValue("ShowInstrument", ShowInstrument);
        s->setValue("ShowGrid", ShowGrid);
        s->setValue("ShowSurface", ShowSurface);
        s->setValue("ShowAxes", ShowAxes);

        //         s->setValue("DisableOpenGL", disableIfSSH);

        s->setValue("GrigStep", (int)GrigStep);

        s->setValue("GridXstart", (int)GridXstart);
        s->setValue("GridXend", (int)GridXend);
        s->setValue("GridYstart", (int)GridYstart);
        s->setValue("GridYend", (int)GridYend);

        s->setValue("ShowGrate", (bool)ShowBorder); // grenzen

        s->setValue("PosX", (int)PosX); //
        s->setValue("PosY", (int)PosY); //
        s->setValue("PosZ", (int)PosZ); //

        s->setValue("AngleX", (int)PosAngleX); //
        s->setValue("AngleY", (int)PosAngleY); //
        s->setValue("AngleZ", (int)PosAngleZ); //

        s->setValue("Zoom", (int)PosZoom); //


        s->setValue("Color_X", (QColor)Settings::colorSettings[COLOR_X]);
        s->setValue("Color_Y", (QColor)Settings::colorSettings[COLOR_Y]);
        s->setValue("Color_Z", (QColor)Settings::colorSettings[COLOR_Z]);
        s->setValue("Color_BG", (QColor)Settings::colorSettings[COLOR_BACKGROUND]);
        s->setValue("Color_Tool", (QColor)Settings::colorSettings[COLOR_TOOL]);
        s->setValue("Color_WB", (QColor)Settings::colorSettings[COLOR_WORKBENCH]);
        s->setValue("Color_Traverse", (QColor)Settings::colorSettings[COLOR_TRAVERSE]);
        s->setValue("Color_Rapid", (QColor)Settings::colorSettings[COLOR_RAPID]);
        s->setValue("Color_Work", (QColor)Settings::colorSettings[COLOR_WORK]);
        s->setValue("Color_Grid", (QColor)Settings::colorSettings[COLOR_GRID]);
        s->setValue("Color_Border", (QColor)Settings::colorSettings[COLOR_BORDER]);
        s->setValue("Color_Surface", (QColor)Settings::colorSettings[COLOR_SURFACE]);
        s->setValue("Color_Connect", (QColor)Settings::colorSettings[COLOR_CONNECTION]);

        s->setValue("LineWidth", (int)Settings::lineWidth);
        s->setValue("PointSize", (int)Settings::pointSize);
        s->setValue("SmoothMoving", (bool)Settings::smoothMoving);
        s->setValue("ShowTraverse", (bool)Settings::showTraverse);
        s->setValue("ShowWorkbench", (bool)Settings::showWorkbench);

        s->endGroup();
    }

    s->beginGroup("mk1");

    for (int c = 0; c < axisNames.length(); c++) {
        s->setValue("Pulse" + QString( axisNames.at(c)), Settings::coord[c].pulsePerMm);
        s->setValue("Accel" + QString( axisNames.at(c)), (double)Settings::coord[c].acceleration);
        s->setValue("StartVelo" + QString( axisNames.at(c)), (double)Settings::coord[c].minVeloLimit);
        s->setValue("EndVelo" + QString( axisNames.at(c)), (double)Settings::coord[c].maxVeloLimit);

        //
        s->setValue("Backlash" + QString( axisNames.at(c)), (double)Settings::coord[c].backlash);
        s->setValue("InvDirection" + QString( axisNames.at(c)), (bool)Settings::coord[c].invertDirection);
        s->setValue("InvPulses" + QString( axisNames.at(c)), (bool)Settings::coord[c].invertPulses);
        s->setValue("InvLimitMax" + QString( axisNames.at(c)), (bool)Settings::coord[c].invLimitMax);
        s->setValue("InvLimitMin" + QString( axisNames.at(c)), (bool)Settings::coord[c].invLimitMin);
        s->setValue("WorkAreaMin" + QString( axisNames.at(c)), (double)Settings::coord[c].workAreaMin);
        s->setValue("WorkAreaMax" + QString( axisNames.at(c)), (double)Settings::coord[c].workAreaMax);
        s->setValue("Enabled" + QString( axisNames.at(c)), (bool)Settings::coord[c].enabled);
        //

        s->setValue("HardLimitMin" + QString( axisNames.at(c)), (bool)Settings::coord[c].useLimitMin);
        s->setValue("HardLimitMax" + QString( axisNames.at(c)), (bool)Settings::coord[c].useLimitMax);

        s->setValue("SoftLimit" + QString( axisNames.at(c)), (bool)Settings::coord[c].checkSoftLimits);
        s->setValue("SoftMin" + QString( axisNames.at(c)), (double)Settings::coord[c].softLimitMin);
        s->setValue("SoftMax" + QString( axisNames.at(c)), (double)Settings::coord[c].softLimitMax);

        s->setValue("Home" + QString( axisNames.at(c)), (double)Settings::coord[c].home);
    }

    s->endGroup();

    s->sync();

    updateSettingsOnGUI();

    delete s;
}


/**
 * @brief load settings of program
 *
 */
void MainWindow::readSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );

    //     s->beginGroup("General");

    QPoint pos = s->value("pos", QPoint(200, 200)).toPoint();
    QSize size = s->value("size", QSize(840, 640)).toSize();
    resize(size);
    move(pos);

    accelerationCutting = s->value("AccelerationCutting", 15).toInt();
    minVelo = s->value("MinVelocity", 20).toInt();
    maxVelo = s->value("MaxVelocity", 400).toInt();

    veloCutting = s->value("VelocityCutting", 200).toInt();
    veloMoving = s->value("VelocityMoving", 500).toInt();
    veloManual = s->value("VelocityManual", 400).toInt();
    currentKeyPad = s->value("KeyControl", -1).toInt();

    unitMm = s->value("UnitMM", 1.0).toBool();
    Settings::splitsPerMm =   s->value("SplitArcPerMM", 10).toInt();
    Settings::maxLookaheadAngle = s->value("LookaheadAngle", 170.0).toFloat();
    cuttedMaterial = (MATERIAL)s->value("CuttedMaterial", 0).toInt();

    toolDiameter = s->value("ToolDiameter", 3.0).toFloat();
    toolFlutes = s->value("ToolFlutes", 2).toInt();
    toolRPM = s->value("ToolRPM", 10000).toInt();

    Settings::filterRepeat = s->value("FilterRepeatData", true).toBool();

    foreach(uKeys k, userKeys) {
        k.code = (Qt::Key)s->value(k.name, (quint32)k.code).toUInt();
    }

    groupManualControl->setChecked(currentKeyPad != -1);

    numVeloSubmission->setValue(veloCutting);
    numVeloMoving->setValue(veloMoving);
    numVeloManual->setValue(veloManual);

    QString l;
    l = getLocaleString();

    currentLang = s->value("LANGUAGE", l).toString();

    reader->lastDir = s->value("LASTDIR", "").toString();

    sysFont = sysFont.toString();

    int sz = sysFont.pointSize();

    if ( sz == -1) {
        sz = sysFont.pixelSize();
    }

    fontSize = sz;
    lastFiles.clear();

    for (int i = 0; i < 10; i++) {
        QString d = s->value("LASTFILE" + QString::number(i)).toString();
        QFile fl;

        if (d.length() == 0) {
            break;
        }

        if (fl.exists(d) == true) {
            lastFiles << d;
        }
    }

    lastFiles.removeDuplicates();

    reloadRecentList();

    QDir dir;
    QStringList dirsLang;
    dirsLang << "/usr/share/cnc-qt/" << "/usr/local/share/cnc-qt/" << currentAppDir;

    foreach(QString entry, dirsLang) {
        helpDir = entry + "/help/";

        dir = QDir(helpDir);

        if (dir.exists() == true) {
            break;
        } else {
            helpDir = "";
        }
    }

    foreach(QString entry, dirsLang) {
        langDir = entry + "/lang/";

        dir = QDir(langDir);

        if (dir.exists() == true) {
            break;
        } else {
            langDir = "";
        }
    }

    //       s->endGroup();

    if (enableOpenGL == true) {
        // opengl settings
        s->beginGroup("OpenGL");

        ShowLines = s->value("ShowLines", false).toBool();
        ShowPoints = s->value("ShowPoints", true).toBool();

        ShowInstrument = s->value("ShowInstrument", true).toBool();
        ShowGrid = s->value("ShowGrid", true).toBool();
        ShowSurface = s->value("ShowSurface", false).toBool();
        ShowAxes = s->value("ShowAxes", true).toBool();

        //         disableIfSSH =  s->value("DisableOpenGL", false).toBool();

        GrigStep = s->value("GrigStep", 10).toInt();

        GridXstart = s->value("GridXstart", -100).toInt();
        GridXend = s->value("GridXend", 100).toInt();
        GridYstart = s->value("GridYstart", -100).toInt();
        GridYend = s->value("GridYend", 100).toInt();

        ShowBorder = s->value("ShowGrate", true).toBool(); // grenzen

        PosX = s->value("PosX", -96 ).toInt(); //
        PosY = s->value("PosY", -64 ).toInt(); //
        PosZ = s->value("PosZ", -300 ).toInt(); //

        PosAngleX = s->value("AngleX", 180 ).toInt(); //
        PosAngleY = s->value("AngleY", 180 ).toInt(); //
        PosAngleZ = s->value("AngleZ", 180 ).toInt(); //

        PosZoom = s->value("Zoom", 20 ).toInt(); //

        Settings::colorSettings[COLOR_X] = s->value("Color_X", QColor {
            0, 255, 0, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_Y] = s->value("Color_Y", QColor {
            255, 0, 0, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_Z] = s->value("Color_Z", QColor {
            0, 255, 255, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_BACKGROUND] = s->value("Color_BG", QColor {
            100, 100, 100, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_TOOL] = s->value("Color_Tool", QColor {
            255, 255, 0, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_WORKBENCH] = s->value("Color_WB", QColor {
            0, 0, 255, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_TRAVERSE] = s->value("Color_Traverse", QColor {
            255, 255, 255, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_RAPID] = s->value("Color_Rapid", QColor {
            255, 0, 0, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_WORK] = s->value("Color_Work", QColor {
            0, 255, 0, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_GRID] = s->value("Color_Grid", QColor {
            200, 200, 200, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_BORDER] = s->value("Color_Border", QColor {
            200, 200, 200, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_SURFACE] = s->value("Color_Surface", QColor {
            255, 255, 255, 255
        }).value<QColor>();
        Settings::colorSettings[COLOR_CONNECTION] = s->value("Color_Connect", QColor {
            150, 255, 100, 255
        }).value<QColor>();

        Settings::pointSize = s->value("PointSize", 1).toInt();
        Settings::lineWidth = s->value("LineWidth", 3).toInt();
        Settings::smoothMoving = s->value("SmoothMoving", false).toBool();
        Settings::showTraverse = s->value("ShowTraverse", false).toBool();
        Settings::showWorkbench = s->value("ShowWorkbench", false).toBool();

        s->endGroup();
    }

    bool res;

    s->beginGroup("mk1");

    for (int c = 0; c < axisNames.length(); c++) {
        int i = s->value("Pulse" + QString( axisNames.at(c)), 200).toInt( &res);
        Settings::coord[c].pulsePerMm = (res == true) ? i : 200;

        float f = s->value("Accel" + QString( axisNames.at(c)), 15).toFloat( &res);
        Settings::coord[c].acceleration = (res == true) ? f : 15;

        f = s->value("StartVelo" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].minVeloLimit = (res == true) ? f : 0;

        f = s->value("EndVelo" + QString( axisNames.at(c)), 400).toFloat( &res);
        Settings::coord[c].maxVeloLimit = (res == true) ? f : 400;

        Settings::coord[c].checkSoftLimits = s->value("SoftLimit" + QString( axisNames.at(c)), false).toBool( );

        f = s->value("SoftMin" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].softLimitMin = (res == true) ? f : 0;

        f = s->value("SoftMax" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].softLimitMax = (res == true) ? f : 0;

        f = s->value("Home" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].home = (res == true) ? f : 0;

        Settings::coord[c].useLimitMin = s->value("HardLimitMin" + QString( axisNames.at(c)), true).toBool();
        Settings::coord[c].useLimitMax = s->value("HardLimitMax" + QString( axisNames.at(c)), true).toBool();

        //
        Settings::coord[c].invertDirection = s->value("InvDirection" + QString( axisNames.at(c)), false).toBool();
        Settings::coord[c].invertPulses = s->value("InvPulses" + QString( axisNames.at(c)), false).toBool();
        Settings::coord[c].invLimitMax = s->value("InvLimitMax" + QString( axisNames.at(c)), false).toBool();
        Settings::coord[c].invLimitMin = s->value("InvLimitMin" + QString( axisNames.at(c)), false).toBool();
        Settings::coord[c].enabled = s->value("Enabled" + QString( axisNames.at(c)), true).toBool();

        f = s->value("Backlash" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].backlash = (res == true) ? f : 0;

        f = s->value("WorkAreaMin" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].workAreaMin = (res == true) ? f : 0;

        f = s->value("WorkAreaMax" + QString( axisNames.at(c)), 0).toFloat( &res);
        Settings::coord[c].workAreaMax = (res == true) ? f : 0;
        //
    }

    s->endGroup();

    updateSettingsOnGUI();

    delete s;
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
        delete filesGroup;
    }

    actFileSelect.clear();

    if (lastFiles.count() > 0) {
        filesMenu = new QMenu( translate(_RECENTFILES)); //insertAction
        QAction *actionRecent = menuFile->insertMenu(actionSave, filesMenu);
        filesGroup = new QActionGroup(this);

        foreach (QString iL, lastFiles) {
            QFileInfo fRecent(iL);

            iL = fRecent.absoluteFilePath();
        }

        lastFiles.removeDuplicates();

        foreach (QString iL, lastFiles) {
            QFileInfo fRecent(iL);

            if (fRecent.exists() == false) {
                continue;
            }

            QAction *tmpAction = new QAction(iL, actionRecent);

            filesGroup->addAction(tmpAction);
            filesMenu->addAction(tmpAction);

            actFileSelect.push_back(tmpAction);
        }

        connect(filesGroup, SIGNAL(triggered(QAction*)), this, SLOT(setFile(QAction*)));
    }
}


/**
 * @brief select file from recent files list
 *
 */
void MainWindow::setFile(QAction* a)
{
    QString fileStr;

    fileStr = a->text();

    fileStr = fileStr.remove("&");

    if (reader->OpenFile(fileStr) == false) {
        AddLog("File loading error: " + fileStr );
        return;
    }

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
        fileStr.replace(QDir::homePath(), QString("~"));
        AddLog("File loaded: " + fileStr );
    }
}


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

    currentLang = lngStr;

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
void MainWindow::getRotation()
{
    displayRotation();
}

/**
 * @brief
 *
 */
void MainWindow::getScale(int s)
{
    scale = s;
    displayRotation();
}


/**
 * @brief change the information about rotations on the push buttons
 *
 */
void MainWindow::displayRotation()
{
    posAngleX->setText( QString().sprintf("%d°", PosAngleX));
    posAngleY->setText( QString().sprintf("%d°", PosAngleY));
    posAngleZ->setText( QString().sprintf("%d°", PosAngleZ));
}


MainWindow::~MainWindow()
{
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
        MessageBox::exec(this, translate(_WARN), translate(_MSG_FOR_DISABLE), QMessageBox::Critical);
        ce->ignore();
        return;
    }

    int ans;
    ans = MessageBox::exec(this, translate(_EXIT), translate(_REALLYQUIT), QMessageBox::Question);

    if (ans == QMessageBox::No) {
        ce->ignore();
        return;
    }

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
        MessageBox::exec(this, translate(_WARN), translate(_MSG_FOR_DISABLE), QMessageBox::Critical);
        return;
    }

    if (MessageBox::exec(this, translate(_EXIT), translate(_REALLYQUIT), QMessageBox::Question) == QMessageBox::No) {
        //         ce->ignore();
        return;
    }

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
    groupB14->setTitle(translate(_BYTE_14));
    groupB15->setTitle(translate(_BYTE_15));
    groupB19->setTitle(translate(_BYTE_19));

    groupBoxMoving->setTitle(translate(_MOVING_TOPOINT));
    groupGenerate->setTitle(translate(_GEN_SIGNAL));
    groupPosition->setTitle(translate(_COORDINATES));
    groupBoxExec->setTitle(translate(_GCODE_RUNNING));
    groupManualControl->setTitle(translate(_MANUAL_CONTROL));
    groupIndicator->setTitle(translate(_DISPL_LIMITS));
    groupVelocity->setTitle(translate(_VELOCITY));

    toolRunMoving->setText(translate(_RUN));

    QStringList m = translate(_MATERIAL_LIST).split("\n");

    if (m.count() > 0) {
        if (cuttedMaterial < m.count()) {
            labelMaterial->setText(m.at(cuttedMaterial));
        } else {
            labelMaterial->setText(m.at(0));
        }
    }

    pushClean->setText(translate(_CLEAN));
    pushSendSignal->setText(translate(_GEN_SIGNAL));
    pushManualControl->setText(translate(_MANUAL_CONTROL));

    radioButtonOff->setText(translate(_OFF));
    radioButtonHz->setText(translate(_HZ));
    radioButtonRC->setText(translate(_RC));

    labelPWMVelo->setText(translate(_VELO_PWM));
    labelPWMCHan->setText(translate(_CHAN_PWM));

    if (enableOpenGL == true) {
        tabWidget->setTabText(0, translate(_DATA));
        tabWidget->setTabText(1, translate(_3D_VIEW));
        tabWidget->setTabText(2, translate(_WORKBENCH));
        tabWidget->setTabText(3, translate(_DIAGNOSTIC));
        tabWidget->setTabText(4, translate(_ADDITIONAL));
        tabWidget->setTabText(5, translate(_LOG));
    } else {
        tabWidget->setTabText(0, translate(_DATA));
        tabWidget->setTabText(1, translate(_WORKBENCH));
        tabWidget->setTabText(2, translate(_DIAGNOSTIC));
        tabWidget->setTabText(3, translate(_ADDITIONAL));
        tabWidget->setTabText(4, translate(_LOG));
    }

    labelSubmission->setText(translate(_SUBMISSION));
    labelMoving->setText(translate(_MOVING));

    checkEnSpindle->setText(translate(_ON_SPINDLE));
    //     checkHWLimits->setText(translate(_CHECK_HW_LIMITS));
    checkHomeAtStart->setText(translate(_GO_HOME_AT_START));
    checkHomeAtEnd->setText(translate(_GO_HOME_AT_END));

    labelVelo->setText(translate(_VELO));

    //     labelRotat->setText(translate(_ROTATION));

    labelMinX->setText(translate(_MIN));
    labelMaxX->setText(translate(_MAX));

    //
    labelRunFrom->setText(translate(_CURRENT_LINE));
    labelNumVelo->setText(translate(_VELO));

    menuFile->setTitle(translate(_FILE));
    menuSettings->setTitle(translate(_SETTINGS));
    menuController->setTitle(translate(_CONTROLLER));
    menuHelp->setTitle(translate(_HELP));

    actionOpen->setText(translate(_OPEN_FILE));

    if (filesMenu != 0) {
        filesMenu->setTitle( translate(_RECENTFILES));
    }

    actionSave->setText(translate(_SAVE_GCODE));
    actionExit->setText(translate(_EXIT));

    actionFluid->setText(translate(_COOLANT));
    actionMist->setText(translate(_MIST));

    actionProgram->setText(translate(_PROGRAM));
    //     actionOpenGL->setText(translate(_OPENGL));
    //     actionConnect->setText(translate(_CONNECT));
    //     actionDisconnect->setText(translate(_DISCONNECT));
    actionAbout->setText(translate(_ABOUT));

    listGCodeWidget->setHorizontalHeaderLabels((QStringList() << translate(_COMMAND) << translate(_INFO) << translate(_STATE)));
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
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_CONN), QMessageBox::Critical);
        return;
    }

    if (gCodeData.count() == 0) {
        // no data
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_DATA), QMessageBox::Critical);
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
        QString msg = translate(_QUEST_START_FROMLINE);
        beg = selected.first().row();

        int dlgres =  MessageBox::exec(this, translate(_START_PROG), msg.arg(QString::number(beg + 1)), QMessageBox::Question);

        if (dlgres == QMessageBox::Cancel) {
            return;
        }

        end = listGCodeWidget->rowCount() - 1;
    }

    if (selected.count() > 1) { //select lines range
        QString msg = translate(_QUEST_START_FROMTOLINE);

        beg = selected.first().row();
        end = selected.count() + beg - 1;
        int dlgr =  MessageBox::exec(this, translate(_START_PROG),  msg.arg(QString::number(beg + 1 ))
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
    QString s = translate(_FROM_TO).arg( Task::lineCodeStart + 1).arg( Task::lineCodeEnd + 1);
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
        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());

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

            //             AddLog(translate(_START_TASK_AT) + QDateTime().currentDateTime().toString());

            //             int MaxSpeedX = 100;
            //             int MaxSpeedY = 100;
            //             int MaxSpeedZ = 100;
            //             int MaxSpeedA = 100;

            mk1->pack9E(0x05);

            //             mk1->packBF(MaxSpeedX, MaxSpeedY, MaxSpeedZ, MaxSpeedA);
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

            //             mParams.posX = gcodeNow.X;
            //             mParams.posY = gcodeNow.Y;
            //             mParams.posZ = gcodeNow.Z + 10.0;
            //             mParams.posA = gcodeNow.A;//, userSpeedG0;
            //             mParams.speed = gcodeNow.vectSpeed;
            //             mParams.movingCode = gcodeNow.movingCode;
            //             mParams.restPulses = gcodeNow.stepsCounter;
            //             mParams.numberInstruction = Task::instrCounter;

            //             mk1->packCA(&mParams); // move to init position
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

        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());

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

        AddLog(translate(_START_TASK_AT) + QDateTime().currentDateTime().toString());

        //         int MaxSpeedX = 100;
        //         int MaxSpeedY = 100;
        //         int MaxSpeedZ = 100;
        //         int MaxSpeedA = 100;

        mk1->pack9E(0x05);

        //         mk1->packBF(MaxSpeedX, MaxSpeedY, MaxSpeedZ, MaxSpeedA);
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
    //         AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());
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
            MessageBox::exec(this, translate(_PAUSE), translate(_RECIEVED_M0), QMessageBox::Information);
        } else {
            QString msg = translate(_PAUSE_G4);
            statusLabel2->setText( msg.arg(QString::number(gcodeNow.pauseMSeconds)));

            QThread().wait(gcodeNow.pauseMSeconds); // pause in msec

            statusLabel2->setText( "" );
        }
    }

    //replace instrument
    if (gcodeNow.changeInstrument) {
        currentStatus = Task::Paused;

        //pause before user click
        QString msg = translate(_PAUSE_ACTIVATED);
        MessageBox::exec(this, translate(_PAUSE), msg.arg(QString::number(gcodeNow.numberInstrument)).arg(QString::number(gcodeNow.diametr)), QMessageBox::Information);
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

    labelRunFrom->setText( translate(_CURRENT_LINE) + " " + QString::number(Task::lineCodeNow + 1));
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



/**
 * @brief slot for signals from usb connector: detach or hotplug of controller
 *
 */
void MainWindow::onCncHotplug()
{
    bool e = mk1->isConnected();

    if (e == true) {
        AddLog(translate(_HOTPLUGED));
    } else {
        AddLog(translate(_DETACHED));
    }

    actionInfo->setEnabled(e);

    if (mainGUITimer.isActive() == false) {
        refreshElementsForms();
    }
}


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

    labelSpeed->setText( QString::number(mk1->getSpindleMoveSpeed()) + translate(_MM_MIN));
    //     statLabelNumInstr->setText( translate(_NUM_INSTR) + QString::number(mk1->numberCompleatedInstructions()));

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

        return;
    }

    switch (currentStatus) {
        case Task::Start: {
            statusLabel1->setText( translate(_START_TASK));
            break;
        }

        case Task::Paused: {
            statusLabel1->setText( translate(_PAUSE_TASK));
            break;
        }

        case Task::Stop: {
            statusLabel1->setText( translate(_STOP_TASK));
            break;
        }

        case Task::Working: {
            statusLabel1->setText( translate(_RUN_TASK));
            break;
        }

        case Task::Waiting: {
            statusLabel1->setText( translate(_WAIT));
            break;
        }
    }

    numPosX->setValue( Settings::coord[X].posMm());
    numPosY->setValue( Settings::coord[Y].posMm());
    numPosZ->setValue( Settings::coord[Z].posMm());

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
    groupBoxExec->setEnabled( cncConnected );

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
            numPosX->setReadOnly( true );

            toolResetCoorY->setEnabled( true );
            numPosY->setReadOnly( true );

            toolResetCoorZ->setEnabled( true );
            numPosZ->setReadOnly( true );

            toolResetCoorA->setEnabled( true );
            numAngleGrad->setReadOnly( true );
        } else {
            toolResetCoorX->setEnabled( false );
            numPosX->setReadOnly( false );

            toolResetCoorY->setEnabled( false );
            numPosY->setReadOnly( false );

            toolResetCoorZ->setEnabled( false );
            numPosZ->setReadOnly( false );

            toolResetCoorA->setEnabled( false );
            numAngleGrad->setReadOnly( false );
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
            numPosX->setReadOnly( true );

            toolResetCoorY->setEnabled( true );
            numPosY->setReadOnly( true );

            toolResetCoorZ->setEnabled( true );
            numPosZ->setReadOnly( true );

            toolResetCoorA->setEnabled( true );
            numAngleGrad->setReadOnly( true );
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
        numPosX->setReadOnly( false );

        toolResetCoorY->setEnabled( false );
        numPosY->setReadOnly( false );

        toolResetCoorZ->setEnabled( false );
        numPosZ->setReadOnly( false );

        toolResetCoorA->setEnabled( false );
        numAngleGrad->setReadOnly( false );
    }
}


/**
 * @brief slot for popup window for manual moving of device
 *
 */
void MainWindow::onManualControlDialog()
{
    ManualControlDialog *mc = new ManualControlDialog(this);
    mc->exec();
    delete mc;
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

    QStringList header = (QStringList() << translate(_COMMAND) << translate(_INFO) << translate(_STATE));

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

                float dX = fabs(gCodeData.at(i - 1).xyz.x() - gCodeData.at(i).xyz.x());
                float dY = fabs(gCodeData.at(i - 1).xyz.y() - gCodeData.at(i).xyz.y());
                float dH = sqrt(dX * dX + dY * dY);
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
                float dY = fabs(gCodeData.at(i - 1).xyz.y() - gCodeData.at(i).xyz.y());
                float dZ = fabs(gCodeData.at(i - 1).xyz.z() - gCodeData.at(i).xyz.z());
                float dH = sqrt(dZ * dZ + dY * dY);
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
                float dZ = fabs(gCodeData.at(i - 1).xyz.z() - gCodeData.at(i).xyz.z());
                float dX = fabs(gCodeData.at(i - 1).xyz.x() - gCodeData.at(i).xyz.x());
                float dH = sqrt(dX * dX + dZ * dZ);
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

        if (fabs(gCodeData[idx].deltaAngle) > fabs(PI - maxLookaheadAngleRad)) {
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
    reader->SaveFile();
}


/**
 * @brief open g-code file
 *
 */
void MainWindow::onOpenFile()
{
    QString nm;

    statusProgress->setValue(0);

    if (reader->OpenFile(nm) == false) {
        AddLog("File loading error: " + nm );
        return;
    }

    lastFiles.insert(0, nm);
    lastFiles.removeDuplicates();

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
        nm.replace(QDir::homePath(), QString("~"));
        AddLog("File loaded: " + nm );
    }
}

/**
 * @brief slot for calling of feed speed calculation pop up window
 *
 */
void MainWindow::onCalcVelocity()
{
    CuttingCalc *setfrm = new CuttingCalc(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        writeSettings();
        QStringList m = translate(_MATERIAL_LIST).split("\n");

        if (m.count() > 0) {
            if (cuttedMaterial < m.count()) {
                labelMaterial->setText(m.at(cuttedMaterial));
            } else {
                labelMaterial->setText(m.at(0));
            }
        }

        numVeloSubmission->setValue(veloCutting);
    }
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
    ShowSurface = true;

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
    numPosX->setValue(0.0);
}


/**
 * @brief slot for resetting of "y" coordinate
 *
 */
void MainWindow::onButtonYtoZero()
{
    mk1->deviceNewPosition(Settings::coord[X].actualPosPulses, 0, Settings::coord[Z].actualPosPulses, Settings::coord[A].actualPosPulses);
    numPosY->setValue(0.0);
}


/**
 * @brief slot for resetting of "z" coordinate
 *
 */
void MainWindow::onButtonZtoZero()
{
    mk1->deviceNewPosition(Settings::coord[X].actualPosPulses, Settings::coord[Y].actualPosPulses, 0, Settings::coord[A].actualPosPulses);
    numPosZ->setValue(0.0);
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
    numAngleGrad->setValue(0.0);
}


/**
 * @brief slot for popup window
 *
 */
void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
