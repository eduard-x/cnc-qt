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
#include <QDebug>
#include <QVector>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QToolButton>

#if USE_OPENGL == true
#include <QtOpenGL>
#endif

#include "includes/Settings.h"
#include "includes/About.h"
#include "includes/Reader.h"
#include "includes/CuttingCalc.h"
#include "includes/EditGCode.h"
#include "includes/ManualControl.h"
#include "includes/ScanSurface.h"

#if USE_OPENGL == true
#include "includes/Settings3d.h"
#endif

#include "includes/MainWindow.h"


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
EStatusTask  Task::Status = Stop;
// int Task::instructionStart = -1;
// int Task::instructionEnd = -1;
// int Task::instructionNow = -1;
int Task::lineCodeStart = -1;
int Task::lineCodeNow = -1;
int Task::lineCodeEnd = -1;
int Task::instrCounter = -1;


/**
 * @brief
 *
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), Reader ()
{
    setupUi(this);

    textLog->document()->setMaximumBlockCount(4000);

    setWindowTitle(translate(_PROG_NAME));

    axisList << "X" << "Y" << "Z" << "A";

    currentAppDir = qApp->applicationDirPath();

    if (currentAppDir.lastIndexOf("/build") > 0) { // build dir detection
        currentAppDir.remove("/build" );
    }

    QString n = QString::number(1.01);
    Settings::toDecimalPoint = (n.indexOf(",") > 0) ? ',' : '.';
    Settings::fromDecimalPoint = (Settings::toDecimalPoint == ',') ? '.' : ',';


#if USE_OPENGL == false
    enableOpenGL = false;
#else
    QString d = getenv( "DISPLAY" ); // linux machines only!

    // to disable the OpenGL features, if over ssh
    enableOpenGL = (d.indexOf(QRegExp(":[0-9]")) == 0);
#endif
    currentLang = "English";

    filesMenu = 0;
    filesGroup = 0;

    QFont sysFont = qApp->font();
    sysFont = sysFont;

    fontSize = sysFont.pointSize();

    userKeys = (QVector<uKeys>() << (uKeys) {
        "UserAplus", Qt::Key_multiply
    } << (uKeys) {
        "UserAminus", Qt::Key_division
    } << (uKeys) {
        "UserZplus", Qt::Key_Home
    } << (uKeys) {
        "UserZminus", Qt::Key_End
    } << (uKeys) {
        "UserYplus", Qt::Key_Up
    } << (uKeys) {
        "UserYminus", Qt::Key_Down
    } << (uKeys) {
        "UserXplus", Qt::Key_Right
    } << (uKeys) {
        "UserXminus", Qt::Key_Left
    });

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

    //
    cnc = new mk1Controller();
    //     cnc->loadSettings();

    // OpenGL area
    if (enableOpenGL == true) {
#if USE_OPENGL == true
        scene3d = new GLWidget(this);

        scrollArea->setWidget(scene3d);

        OpenGL_preview->addWidget(scrollArea, 0, 0);
#endif
        QPalette palette = statusLabel2->palette();
        //  palette.setColor(statusLabel2->backgroundRole(), Qt::yellow);
        palette.setColor(statusLabel2->foregroundRole(), Qt::darkBlue);
        statusLabel2->setPalette(palette);
        statusLabel2->setText( "OpenGL " + translate(_ENABLED));
        // OpenGL is placed in widget
    } else {
#if USE_OPENGL == true
        scene3d = 0;
#endif
        QPalette palette = statusLabel2->palette();
        //  palette.setColor(statusLabel2->backgroundRole(), Qt::yellow);
        palette.setColor(statusLabel2->foregroundRole(), Qt::red);
        statusLabel2->setPalette(palette);
        statusLabel2->setText( "OpenGL " + translate(_DISABLED) );
        tabWidget->removeTab(1);
        actionOpenGL->setEnabled(false);
    }

    Correction = false;
    deltaX = 0;
    deltaY = 0;
    deltaZ = 0;
    koeffSizeX = 1;
    koeffSizeY = 1;
    deltaFeed = false;

    zAngle = 0;
    yAngle = 0;
    xAngle = 0;
    scale = 1;

    addConnections();

    if (getLangTable() == false) {
        MessageBox::exec(this, translate(_ERR),
                         "Can't open language file!\nDefault GUI language is english", QMessageBox::Critical);

        currentLang = "English";
    }

    for (QList<QAction*>::iterator itL = actLangSelect.begin(); itL != actLangSelect.end(); ++itL) {
        if ((*itL)->text() == currentLang) {
            (*itL)->setChecked(true);
            break;
        }
    }

    QStringList arguments = QCoreApplication::arguments();

    if (arguments.size() > 1) {
        if (arguments.at(1).length() > 0) { // as parameter is file name to load
            QString nm = arguments.at(1);

            if (OpenFile(nm) == true) {

                lastFiles.insert(0, nm);
                lastFiles.removeDuplicates();
                //                 qDebug() << lastFiles;

                reloadRecentList();

                QStringList l = getGoodList();
                fillListWidget(l);

                l = getBadList();

                if (l.count() != 0) {
                    foreach (QString s, l) {
                        AddLog(s);
                    }
                } else {
                    AddLog("File loaded" );
                }
            }
        }
    }

    statusProgress->setRange(0, 100);
    statusProgress->setValue(0);

    translateGUI();

    refreshElementsForms();
};


/**
 * @brief
 *
 */
bool MainWindow::getLangTable()
{
    QString lang = currentLang;
    QString fileLang = "";

    for (QStringList::Iterator iLang = langFiles.begin(); iLang != langFiles.end(); iLang++) {
        if ((*iLang).contains(":" + lang) > 0) {
            fileLang = *iLang;
            fileLang.remove(":" + lang);
            break;
        }
    }

    if (fileLang == "") {
        return (false);
    }

    if (QFile::exists(langDir + fileLang) == false) {
        MessageBox::exec(this, translate(_ERR), "Language file not exists!\n\n"
                         + langDir + "\n\n" + fileLang, QMessageBox::Warning);

        return (false);
    }

    return loadTranslation(langDir + fileLang);
}


/**
 * @brief
 *
 */
void MainWindow::addConnections()
{
    // connect menu actions
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onOpenFile()));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(onSaveFile()));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(onExit()));

    connect(actionOpenGL, SIGNAL(triggered()), this, SLOT(on3dSettings()));
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

    connect(listGCodeWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onEditGCode(int, int)));
    connect(listGCodeWidget, SIGNAL(cellActivated(int, int)), this, SLOT(onCellSelect(int, int)));

    connect(pushSendSignal, SIGNAL(clicked()), this, SLOT(onSendCommand()));
    connect(toolRunMoving, SIGNAL(clicked()), this, SLOT(onRunToPoint()));

    // connected or disconnected
    connect(cnc, SIGNAL(hotplugSignal ()), this, SLOT(onCncHotplug())); // cnc->WasConnected += CncConnect;
    //     connect(cnc, SIGNAL(hotplugDisconnected ()), this, SLOT(onCncHotplug())); // cnc->WasDisconnected += CncDisconnect;

    connect(cnc, SIGNAL(newDataFromMK1Controller ()), this, SLOT(onCncNewData())); // cnc->NewDataFromController += CncNewData;
    connect(cnc, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message += CncMessage;

    connect(&mainTaskTimer, SIGNAL(timeout()), this, SLOT(onMainTaskTimer()));
    connect(&mainGUITimer, SIGNAL(timeout()), this, SLOT(onRefreshGUITimer()));
    
//     mainGUITimer.setInterval(200); 
    mainGUITimer.start(500);// every 0.5 sec update

    mainTaskTimer.setInterval(20); // every 20 msec update

    if (enableOpenGL == true) {
#if USE_OPENGL == true
        // 3d buttons
        connect(posAngleXm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXm()));
        connect(posAngleX, SIGNAL(clicked()), scene3d, SLOT(onPosAngleX())); // reset to 0
        connect(scene3d, SIGNAL(xRotationChanged(int)), this, SLOT(getXRotation(int)));
        connect(scene3d, SIGNAL(fpsChanged(int)), this, SLOT(getFPS(int)));
        connect(posAngleXp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXp()));

        connect(posAngleYm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYm()));
        connect(posAngleY, SIGNAL(clicked()), scene3d, SLOT(onPosAngleY())); // reset to 0
        connect(scene3d, SIGNAL(yRotationChanged(int)), this, SLOT(getYRotation(int)));
        connect(posAngleYp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleYp()));

        connect(posAngleZm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZm()));
        connect(posAngleZ, SIGNAL(clicked()), scene3d, SLOT(onPosAngleZ())); // reset to 0
        connect(scene3d, SIGNAL(zRotationChanged(int)), this, SLOT(getZRotation(int)));
        connect(posAngleZp, SIGNAL(pressed()), scene3d, SLOT(onPosAngleZp()));

        connect(scene3d, SIGNAL(scaleChanged(int)), this, SLOT(getScale(int)));

        connect(pushDefaultPreview, SIGNAL(clicked()), scene3d, SLOT(onDefaulPreview()));
        // end of 3d buttons
#endif
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
    QLabel *label = new QLabel(cnc->getDescription());
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

    //     dictFormat = 0;

    foreach(QString entry, dirsLang) {
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


    for (QStringList::Iterator iL = fList.begin(); iL != fList.end(); iL++) {
        QFile fLang(lngDirName + *iL);

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

                    langFiles += (*iL) + ":" + nm;
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
// to get locale and convert to internal string
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
 * @brief save settings of program
 *
 */
void MainWindow::writeSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );
    s->setValue("pos", pos());
    s->setValue("size", size());
    //     s->setValue("WorkDir", currentWorkDir);
    s->setValue("LANGUAGE", currentLang);

    s->setValue("VelocityCutting", numVeloSubmission->value());
    s->setValue("VelocityMoving", numVeloMoving->value());
    s->setValue("VelocityManual", numVeloManual->value());

    s->setValue("SplitArcPerMM", Settings::splitsPerMm);
    s->setValue("LookaheadAngle", Settings::maxLookaheadAngle);

    s->setValue("UnitMM", unitMm);
    s->setValue("ToolDiameter", toolDiameter);
    s->setValue("ToolFlutes", toolFlutes);
    s->setValue("ToolRPM", toolRPM);

    s->setValue("CuttedMaterial", cuttedMaterial);

    for(int i = 0; i < userKeys.count(); ++i) {
        s->setValue(userKeys.at(i).name, (quint32)userKeys.at(i).code);
    }

    if (groupManualControl->isChecked() == false) {
        currentKeyPad = -1;
    }

    s->setValue("KeyControl", (int)currentKeyPad);
    //     s->setValue("LASTPROJ", currentProject);
    //     s->setValue("FontSize", fontSize);
    //     s->setValue("GUIFont", sysFont);
    // qDebug() << "writeGUISettings";
    int i = 0;

    for (QStringList::Iterator iFile = lastFiles.begin(); iFile != lastFiles.end(); iFile++, i++) {
        if (i > 9) { // max last dirs
            break;
        }

        s->setValue("LASTFILE" + QString::number(i), (*iFile));
    }

    // opengl settings
    s->setValue("ShowLines", ShowLines);
    s->setValue("ShowPoints", ShowPoints);

    s->setValue("ShowInstrument", ShowInstrument);
    s->setValue("ShowGrid", ShowGrid);
    s->setValue("ShowSurface", ShowSurface);
    s->setValue("ShowAxes", ShowAxes);

    s->setValue("GrigStep", GrigStep);

    s->setValue("GridXstart", GridXstart);
    s->setValue("GridXend", GridXend);
    s->setValue("GridYstart", GridYstart);
    s->setValue("GridYend", GridYend);

    s->setValue("ShowGrate", ShowGrate); // grenzen


    s->beginGroup("mk1");

    for (int c = 0; c < axisList.count(); c++) {
        s->setValue("Pulse" + axisList.at(c), Settings::coord[c].pulsePerMm);
        s->setValue("Accel" + axisList.at(c), (double)Settings::coord[c].acceleration);
        s->setValue("StartVelo" + axisList.at(c), (double)Settings::coord[c].minVelo);
        s->setValue("EndVelo" + axisList.at(c), (double)Settings::coord[c].maxVelo);

        //
        s->setValue("Backlash" + axisList.at(c), (double)Settings::coord[c].backlash);
        s->setValue("InvDirection" + axisList.at(c), (bool)Settings::coord[c].invertDirection);
        s->setValue("InvPulses" + axisList.at(c), (bool)Settings::coord[c].invertPulses);
        s->setValue("InvLimitMax" + axisList.at(c), (bool)Settings::coord[c].invLimitMax);
        s->setValue("InvLimitMin" + axisList.at(c), (bool)Settings::coord[c].invLimitMin);
        s->setValue("WorkAreaMin" + axisList.at(c), (double)Settings::coord[c].workAreaMin);
        s->setValue("WorkAreaMax" + axisList.at(c), (double)Settings::coord[c].workAreaMax);
        s->setValue("Enabled" + axisList.at(c), (bool)Settings::coord[c].enabled);
        //

        s->setValue("HardLimitMin" + axisList.at(c), (bool)Settings::coord[c].useLimitMin);
        s->setValue("HardLimitMax" + axisList.at(c), (bool)Settings::coord[c].useLimitMax);

        s->setValue("SoftLimit" + axisList.at(c), (bool)Settings::coord[c].checkSoftLimits);
        s->setValue("SoftMin" + axisList.at(c), (double)Settings::coord[c].softLimitMin);
        s->setValue("SoftMax" + axisList.at(c), (double)Settings::coord[c].softLimitMax);

        s->setValue("Home" + axisList.at(c), (double)Settings::coord[c].home);
    }

    s->endGroup();

    s->sync();
}


/**
 * @brief load settings of program
 *
 */
void MainWindow::readSettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "KarboSoft", "CNC-Qt" );
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

    for(int i = 0; i < userKeys.count(); ++i) {
        userKeys[i].code = (Qt::Key)s->value(userKeys.at(i).name, (quint32)userKeys.at(i).code).toUInt();
    }

    groupManualControl->setChecked(currentKeyPad != -1);


    numVeloSubmission->setValue(veloCutting);
    numVeloMoving->setValue(veloMoving);
    numVeloManual->setValue(veloManual);

    QString l;
    l = getLocaleString();

    currentLang = s->value("LANGUAGE", l).toString();
    //     currentProject = s->value("LASTPROJ").toString();
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

    // opengl settings
    ShowLines = s->value("ShowLines", false).toBool();
    ShowPoints = s->value("ShowPoints", true).toBool();

    ShowInstrument = s->value("ShowInstrument", true).toBool();
    ShowGrid = s->value("ShowGrid", true).toBool();
    ShowSurface = s->value("ShowSurface", false).toBool();
    ShowAxes = s->value("ShowAxes", false).toBool();

    GrigStep = s->value("GrigStep", 10).toInt();

    GridXstart = s->value("GridXstart", -100).toInt();
    GridXend = s->value("GridXend", 100).toInt();
    GridYstart = s->value("GridYstart", -100).toInt();
    GridYend = s->value("GridYend", 100).toInt();

    ShowGrate = s->value("ShowGrate", true).toBool(); // grenzen

    bool res;

    s->beginGroup("mk1");

    for (int c = 0; c < axisList.count(); c++) {
        int i = s->value("Pulse" + axisList.at(c), 200).toInt( &res);
        Settings::coord[c].pulsePerMm = (res == true) ? i : 200;

        float f = s->value("Accel" + axisList.at(c), 15).toFloat( &res);
        Settings::coord[c].acceleration = (res == true) ? f : 15;

        f = s->value("StartVelo" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].minVelo = (res == true) ? f : 0;

        f = s->value("EndVelo" + axisList.at(c), 400).toFloat( &res);
        Settings::coord[c].maxVelo = (res == true) ? f : 400;

        Settings::coord[c].checkSoftLimits = s->value("SoftLimit" + axisList.at(c), false).toBool( );

        f = s->value("SoftMin" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].softLimitMin = (res == true) ? f : 0;

        f = s->value("SoftMax" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].softLimitMax = (res == true) ? f : 0;

        f = s->value("Home" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].home = (res == true) ? f : 0;

        Settings::coord[c].useLimitMin = s->value("HardLimitMin" + axisList.at(c), true).toBool();
        Settings::coord[c].useLimitMax = s->value("HardLimitMax" + axisList.at(c), true).toBool();

        //
        Settings::coord[c].invertDirection = s->value("InvDirection" + axisList.at(c), false).toBool();
        Settings::coord[c].invertPulses = s->value("InvPulses" + axisList.at(c), false).toBool();
        Settings::coord[c].invLimitMax = s->value("InvLimitMax" + axisList.at(c), false).toBool();
        Settings::coord[c].invLimitMin = s->value("InvLimitMin" + axisList.at(c), false).toBool();
        Settings::coord[c].enabled = s->value("Enabled" + axisList.at(c), true).toBool();

        f = s->value("Backlash" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].backlash = (res == true) ? f : 0;

        f = s->value("WorkAreaMin" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].workAreaMin = (res == true) ? f : 0;

        f = s->value("WorkAreaMax" + axisList.at(c), 0).toFloat( &res);
        Settings::coord[c].workAreaMax = (res == true) ? f : 0;
        //
    }

    s->endGroup();

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

        for (QStringList::Iterator iL = lastFiles.begin(); iL != lastFiles.end(); iL++) {
            QFileInfo fRecent(*iL);

            *iL = fRecent.absoluteFilePath();
        }

        lastFiles.removeDuplicates();

        for (QStringList::Iterator iL = lastFiles.begin(); iL != lastFiles.end(); iL++) {
            QFileInfo fRecent(*iL);

            if (fRecent.exists() == false) {
                continue;
            }

            QAction *tmpAction = new QAction(*iL, actionRecent);

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

    if (OpenFile(fileStr) == false) {
        AddLog("File loading error: " + fileStr );
        return;
    }

    QStringList l = getGoodList();
    fillListWidget(l);

    l = getBadList();

    if (l.count() != 0) {
        foreach (QString s, l) {
            AddLog(s);
        }
    } else {
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
    currentLang = lngStr;

    if (getLangTable() == false) {
        qDebug() << "setLang" << false;
    }

    QVector<QAction*>::iterator itL;

    disconnect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(setLang(QAction*)));

    mnu->setChecked(true);

    connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(setLang(QAction*)));

    translateGUI();
}


/**
 * @brief
 *
 */
void MainWindow::getZRotation(int z)
{
    zAngle = z;
    displayRotation();
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
void MainWindow::getXRotation(int x)
{
    xAngle = x;
    displayRotation();
}


/**
 * @brief
 *
 */
void MainWindow::getYRotation(int y)
{
    yAngle = y;
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
    posAngleX->setText( QString().sprintf("%d°", xAngle));
    posAngleY->setText( QString().sprintf("%d°", yAngle));
    posAngleZ->setText( QString().sprintf("%d°", zAngle));
}


MainWindow::~MainWindow()
{
};


/**
 * @brief close event of program
 *
 */
void MainWindow::closeEvent(QCloseEvent* ce)
{
    if (Task::Status != Stop) {
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

    disconnect(cnc, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message -= CncMessage;

    writeSettings();

    delete cnc;

    ce->accept();

    QCoreApplication::quit();
}


/**
 * @brief
 *
 */
void MainWindow::onExit()
{
    if (Task::Status != Stop) {
        MessageBox::exec(this, translate(_WARN), translate(_MSG_FOR_DISABLE), QMessageBox::Critical);
        return;
    }

    if (MessageBox::exec(this, translate(_EXIT), translate(_REALLYQUIT), QMessageBox::Question) == QMessageBox::No) {
        //         ce->ignore();
        return;
    }

    disconnect(cnc, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message -= CncMessage;

    writeSettings();

    delete cnc;

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
        tabWidget->setTabText(2, translate(_ADDITIONAL));
        tabWidget->setTabText(3, translate(_LOG));
    } else {
        tabWidget->setTabText(0, translate(_DATA));
        tabWidget->setTabText(1, translate(_ADDITIONAL));
        tabWidget->setTabText(2, translate(_LOG));
    }

    labelSubmission->setText(translate(_SUBMISSION));
    labelMoving->setText(translate(_MOVING));

    checkBoxEnSpindnle->setText(translate(_ON_SPINDLE));

    labelVelo->setText(translate(_VELO));

    //     labelRotat->setText(translate(_ROTATION));

    labelMinX->setText(translate(_MIN));
    labelMaxX->setText(translate(_MAX));

    labelMinY->setText(translate(_MIN));
    labelMaxY->setText(translate(_MAX));

    labelMinZ->setText(translate(_MIN));
    labelMaxZ->setText(translate(_MAX));

    labelMinA->setText(translate(_MIN));
    labelMaxA->setText(translate(_MAX));

    //
    labelRunFrom->setText(translate(_CURRENT_LINE));
    labelNumVelo->setText(translate(_VELO));

    menuFile->setTitle(translate(_FILE));
    menuSettings->setTitle(translate(_SETTINGS));
    menuController->setTitle(translate(_CONTROLLER));
    menuHelp->setTitle(translate(_HELP));

    actionOpen->setText(translate(_OPEN_FILE));
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
    if (mainTaskTimer.isActive()) {
        return;    //timer is active, task is running
    }

    if (!cnc->isConnected()) {
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_CONN), QMessageBox::Critical);
        return;
    }

    if (gCodeList.count() == 0) {
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

    foreach (GCodeData c, gCodeList) {
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

    qDebug() << "ranges, lines:" << Task::lineCodeStart << Task::lineCodeEnd /*<< "code" << Task::instructionStart << Task::instructionEnd*/ << "size of code" << gCodeList.count();
    QString s = translate(_FROM_TO).arg( Task::lineCodeStart + 1).arg( Task::lineCodeEnd + 1);
    labelTask->setText( s );

    statusProgress->setRange(Task::lineCodeStart, Task::lineCodeEnd);

    groupManualControl->setChecked( false ); // disable manual control

    Task::Status = Start;

    mainTaskTimer.start();

//     refreshElementsForms();
}


/**
 * @brief  slot for task pause after "pause" button
 *
 */
void MainWindow::onPauseTask()
{
    if (Task::Status == Start) {
        return;    //if not started, do not set pause
    }

    if (Task::Status == Working || Task::Status == Paused) {
        Task::Status = (Task::Status == Paused) ? Working : Paused;
    }

//     refreshElementsForms();
}


/**
 * @brief slot for task stop after "stop" button
 *
 */
void MainWindow::onStopTask()
{
    if (Task::Status == Waiting) {
        return;
    }

    Task::Status = Stop;
//     refreshElementsForms();
}


/**
 * @brief slot from main timer signal
 *
 */
void MainWindow::onMainTaskTimer()
{
    if (!cnc->isConnected()) {
        if (mainTaskTimer.isActive()) {
            mainTaskTimer.stop();
        }

        return;
    }

    mainTaskTimer.stop();

    if (runCommand() == true) {
        mainTaskTimer.start();
    }

} //void mainTaskTimer_Tick


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
    if (mainTaskTimer.isActive() == false){
        return;
    }
    
    mainTaskTimer.stop();
     
    if (runCommand() == true) {
        mainTaskTimer.start();
    }
}


/**
 * @brief
 *
 * @return false if timer to stop
 */
bool MainWindow::runCommand()
{
    // Velocity from main form
    int userSpeedG1 = (int)numVeloSubmission->value();
    int userSpeedG0 = (int)numVeloMoving->value();

    if (Task::lineCodeNow > Task::lineCodeEnd) {
        Task::Status = Stop;
        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());

//         refreshElementsForms();
        //
        return false;
    }

    GCodeData gcodeNow = gCodeList.at(Task::instrCounter);

    useHome = checkHome->isChecked();

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

    // Start
    if (Task::Status == Start) { // init of controller
        AddLog(translate(_START_TASK_AT) + QDateTime().currentDateTime().toString());

        int MaxSpeedX = 100;
        int MaxSpeedY = 100;
        int MaxSpeedZ = 100;
        int MaxSpeedA = 100;

        cnc->pack9E(0x05);

        cnc->packBF(MaxSpeedX, MaxSpeedY, MaxSpeedZ, MaxSpeedA);

        cnc->packC0();

        //         cnc->setUseHome( == true); // home or from current position

        //moving to the first point axes X and Y
        //TODO: spindle move higher, now 10 mm
        moveParameters mParams;
        mParams.posX = Settings::coord[X].startPos;
        mParams.posY = Settings::coord[Y].startPos;
        mParams.posZ = Settings::coord[Z].startPos + 10.0;
        mParams.posA = Settings::coord[A].startPos;//, userSpeedG0;
        mParams.speed = 0;//gcodeNow.vectSpeed;
        mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.movingCode;
        mParams.restPulses = 0;//gcodeNow.stepsCounter;
        mParams.numberInstruction = 0;

        cnc->packCA(&mParams); // move to init position

        mParams.posX = gcodeNow.X;
        mParams.posY = gcodeNow.Y;
        mParams.posZ = gcodeNow.Z + 10.0;
        mParams.posA = gcodeNow.A;//, userSpeedG0;
        mParams.speed = gcodeNow.vectSpeed;
        mParams.movingCode = gcodeNow.movingCode;
        mParams.restPulses = gcodeNow.stepsCounter;
        mParams.numberInstruction = Task::instrCounter;

        cnc->packCA(&mParams); // move to init position
        //         cnc->packCA(gcodeNow.X, gcodeNow.Y, Settings::coord[Z].startPos + 10.0, gcodeNow.A , userSpeedG0, gcodeNow.angleVectors, gcodeNow.Distance);

        Task::Status = Working;

//         refreshElementsForms();

        return true; //after start code
    }

    // Stop
    if (Task::Status == Stop) {
        //TODO: move spindle up, possible moving to "home" position

        cnc->packFF();

        cnc->pack9D();

        cnc->pack9E(0x02);

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());
        Task::Status = Stop;
        //         mainTaskTimer.stop();

//         refreshElementsForms();

        return false;
    }

    // Working

    if (Task::Status != Working) {
//         refreshElementsForms();

        return false;
    }

    // the task is ready
    //     if (Task::posCodeNow > Task::posCodeEnd) {
    //         Task::Status = Stop;
    //         AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());
    //
    //         //         mainTaskTimer.stop();
    //         return false;
    //     }


    //TODO: to add in parameter the value
    if (cnc->availableBufferSize() <= 3) {
        return true;    // nothing before buffer clean
    }

    //TODO: to add in parameter the value
    if (Task::instrCounter > (cnc->numberCompleatedInstructions() + 3)) {
        return true;    // don't send more than N commands
    }

    qDebug() << "buff size free: " << cnc->availableBufferSize() << "current instruction: " << Task::instrCounter << "compleate instructions: " << cnc->numberCompleatedInstructions(); 
    
    //command G4 or M0
    if (gcodeNow.pauseMSeconds != -1) {
        if (gcodeNow.pauseMSeconds == 0) { // M0 - waiting command
            Task::Status = Paused;

            //pause before user click
            MessageBox::exec(this, translate(_PAUSE), translate(_RECIEVED_M0), QMessageBox::Information);
        } else {
            QString msg = translate(_PAUSE_G4);
            statusLabel2->setText( msg.arg(QString::number(gcodeNow.pauseMSeconds)));

            QThread().wait(gcodeNow.pauseMSeconds); // pause in msec

            statusLabel2->setText( "" );
        }

//         refreshElementsForms();
        //         return true;
    }

    //replace instrument
    if (gcodeNow.changeInstrument) {
        Task::Status = Paused;
//         refreshElementsForms();

        //pause before user click
        QString msg = translate(_PAUSE_ACTIVATED);
        MessageBox::exec(this, translate(_PAUSE), msg.arg(QString::number(gcodeNow.numberInstrument)).arg(QString::number(gcodeNow.diametr)), QMessageBox::Information);
    }

    int commands = 1;
    
    if (Task::instrCounter > cnc->numberCompleatedInstructions()){;//cnc->availableBufferSize() - 3;
        commands = Task::instrCounter - cnc->numberCompleatedInstructions();
    }
    
    for (int i = 0; i < commands; i++) {
        float pointX = gcodeNow.X;
        float pointY = gcodeNow.Y;
        float pointZ = gcodeNow.Z;
        float pointA = gcodeNow.A;

        Task::lineCodeNow = gcodeNow.numberLine;

        //moving in G-code
        if (Correction) {
            // proportion
            pointX *= koeffSizeX;
            pointY *= koeffSizeY;

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
            mParams.posX = pointX;
            mParams.posY = pointY;
            mParams.posZ = pointZ;
            mParams.posA = pointA;//, userSpeedG0;
            mParams.speed = gcodeNow.vectSpeed;
            mParams.movingCode = gcodeNow.movingCode; //
            mParams.restPulses = gcodeNow.stepsCounter;//
            mParams.numberInstruction = Task::instrCounter++;

            cnc->packCA(&mParams); // move to init position
            //     cnc->packCA(posX, posY, posZ, posA, speed, Task::posCodeNow);
            //     cnc->packCA(pointX, pointY, pointZ, pointA, speed, Task::instructionNow++, 0.0, 0);

        }
    }

    //     Task::posCodeNow++;
    labelRunFrom->setText( translate(_CURRENT_LINE) + " " + QString::number(Task::lineCodeNow + 1));

//     refreshElementsForms();

    return true;
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
void MainWindow::onStatus()
{
    // clean message
    statusLabel2->setText( "" );
}


/**
 * @brief
 *
 */
void MainWindow::moveToPoint(bool surfaceScan)
{
    int speed = 0;
    float posX, posY, posZ, posA;

    if (cnc->isConnected() == false) {
        return;
    }

    if (surfaceScan == true) {
        speed = 200;

        if (scanPosY == -1 || scanPosX == -1) {
            return;
        }

        posX = surfaceMatrix[scanPosY][scanPosX].X;
        posY = surfaceMatrix[scanPosY][scanPosX].Y;
        posZ = surfaceMatrix[scanPosY][scanPosX].Z;
        posA = surfaceMatrix[scanPosY][scanPosX].A;
    } else {
        speed = spinMoveVelo->value();

        posX = doubleSpinMoveX->value();
        posY = doubleSpinMoveY->value();
        posZ = doubleSpinMoveZ->value();
        posA = numAngleGrad->value();
    }

    cnc->pack9E(0x05);

    cnc->packBF(speed, speed, speed, speed);

    cnc->packC0();

    {
        moveParameters mParams;
        mParams.posX = posX;
        mParams.posY = posY;
        mParams.posZ = posZ;
        mParams.posA = posA;//, userSpeedG0;
        mParams.speed = speed;
        mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.movingCode;
        mParams.restPulses = 0;//gcodeNow.stepsCounter;
        mParams.numberInstruction = 0;

        //         cnc->packCA(posX, posY, posZ, posA, speed, 0, 0.0, 0);
        cnc->packCA(&mParams);
    }

    cnc->packFF();

    cnc->pack9D();

    cnc->pack9E(0x02);

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();
}


/**
 * @brief slot for moving to the point
 *
 */
void MainWindow::onRunToPoint()
{
    if (!cnc->testAllowActions()) {
        return;
    }

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

    //     if (radioButtonOff->isChecked()) {
    //         tSign = mk1Data::None;
    //     }

    if (radioButtonHz->isChecked()) {
        tSign = mk1Data::Hz;
    }

    if (radioButtonRC->isChecked()) {
        tSign = mk1Data::RC;
    }

    if (checkBoxEnSpindnle->isChecked()) {
        cnc->spindleON();
    } else {
        cnc->spindleOFF();
    }

    cnc->packB5(checkBoxEnSpindnle->isChecked(), (int)spinBoxChann->value(), tSign, (int)spinBoxVelo->value());
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
    dPoint pResult = {_x, _y, 0.0, 0.0}; //new dobPoint(_x, _y, 0);

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
            if ((_x > surfaceMatrix[y][0].X) && (_x < surfaceMatrix[y + 1][0].X) && (surfaceMatrix[0][x].Y < _y) && (surfaceMatrix[0][x + 1].Y > _y)) {
                indexXmin = x;
                indexYmin = y;
            }
        }
    }

    dPoint p1 = {surfaceMatrix[indexYmin][indexXmin].X, surfaceMatrix[indexYmin][indexXmin].Y, surfaceMatrix[indexYmin][indexXmin].Z, 0.0};
    dPoint p3 = {surfaceMatrix[indexYmin + 1][indexXmin].X, surfaceMatrix[indexYmin + 1][indexXmin].Y, surfaceMatrix[indexYmin + 1][indexXmin].Z, 0.0};
    dPoint p2 = {surfaceMatrix[indexYmin][indexXmin + 1].X, surfaceMatrix[indexYmin][indexXmin + 1].Y, surfaceMatrix[indexYmin][indexXmin + 1].Z, 0.0};
    dPoint p4 = {surfaceMatrix[indexYmin + 1][indexXmin + 1].X, surfaceMatrix[indexYmin + 1][indexXmin + 1].Y, surfaceMatrix[indexYmin + 1][indexXmin + 1].Z, 0.0};

    dPoint p12 = Geometry::CalcPX(p1, p2, pResult);
    dPoint p34 = Geometry::CalcPX(p3, p4, pResult);
    dPoint p1234 = Geometry::CalcPY(p12, p34, pResult);

    return p1234.Z;
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
    //     RefreshElementsForms();
    bool e = cnc->isConnected();

    if (e == true) {
        AddLog(translate(_HOTPLUGED));
    } else {
        AddLog(translate(_DETACHED));
    }

    actionInfo->setEnabled(e);

    refreshElementsForms();
}


/**
 * @brief update the widgtes after new data from microcontroller
 *
 */
void  MainWindow::refreshElementsForms()
{
    bool cncConnected = cnc->isConnected();

    groupPosition->setEnabled( cncConnected);
    groupManualControl->setEnabled( cncConnected);

    // set groupVelocity too?

    actionStop->setEnabled( cncConnected);
    actionSpindle->setEnabled( cncConnected);
    actionMist->setEnabled( cncConnected);
    actionFluid->setEnabled( cncConnected);

    labelSpeed->setText( QString::number(cnc->getSpindleMoveSpeed()) + translate(_MM_MIN));
    //     statLabelNumInstr->setText( translate(_NUM_INSTR) + QString::number(cnc->numberCompleatedInstructions()));

    if (!cncConnected) {
        QPixmap grayPix = QPixmap(":/images/ball_gray.png");

        maxXLED->setPixmap( grayPix );
        minXLED->setPixmap( grayPix );
        maxYLED->setPixmap( grayPix );
        minYLED->setPixmap( grayPix );
        maxZLED->setPixmap( grayPix );
        minZLED->setPixmap( grayPix );
        maxALED->setPixmap( grayPix );
        minALED->setPixmap( grayPix );

        toolRun->setEnabled( cncConnected);
        toolPause->setEnabled( cncConnected);
        toolStop->setEnabled( cncConnected);

        return;
    }

    switch (Task::Status) {
        case Start: {
            statusLabel1->setText( translate(_START_TASK));
            break;
        }

        case Paused: {
            statusLabel1->setText( translate(_PAUSE_TASK));
            break;
        }

        case Stop: {
            statusLabel1->setText( translate(_STOP_TASK));
            break;
        }

        case Working: {
            statusLabel1->setText( translate(_RUN_TASK));
            break;
        }

        case Waiting: {
            statusLabel1->setText( translate(_WAIT));
            break;
        }
    }

    numPosX->setValue( Settings::coord[X].posMm());
    numPosY->setValue( Settings::coord[Y].posMm());
    numPosZ->setValue( Settings::coord[Z].posMm());

#if 0

    if (cnc->isEmergencyStopOn()) {
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

    if (cnc->isSpindelOn()) {
        actionSpindle->BackColor = Color.Green;
        actionSpindle->ForeColor = Color.White;
    } else {
        actionSpindle->BackColor = Color.FromName("Control" );
        actionSpindle->ForeColor = Color.Black;
    }

#endif

    QPixmap greenPix = QPixmap(":/images/ball_green.png");
    QPixmap redPix = QPixmap(":/images/ball_red.png");

    byte bb15 = cnc->getByte(15);

    maxXLED->setPixmap( bb15 & (1 << 0) ? redPix : greenPix );
    minXLED->setPixmap( bb15 & (1 << 1) ? redPix : greenPix );
    maxYLED->setPixmap( bb15 & (1 << 2) ? redPix : greenPix );
    minYLED->setPixmap( bb15 & (1 << 3) ? redPix : greenPix );
    maxZLED->setPixmap( bb15 & (1 << 4) ? redPix : greenPix );
    minZLED->setPixmap( bb15 & (1 << 5) ? redPix : greenPix );
    maxALED->setPixmap( bb15 & (1 << 6) ? redPix : greenPix );
    minALED->setPixmap( bb15 & (1 << 7) ? redPix : greenPix );

    //***************

    //DEBUG:
    byte bb14 = cnc->getByte(14);
    labelB14B0->setPixmap( bb14 & (1 << 0) ? redPix : greenPix );
    labelB14B1->setPixmap( bb14 & (1 << 1) ? redPix : greenPix );
    labelB14B2->setPixmap( bb14 & (1 << 2) ? redPix : greenPix );
    labelB14B3->setPixmap( bb14 & (1 << 3) ? redPix : greenPix );
    labelB14B4->setPixmap( bb14 & (1 << 4) ? redPix : greenPix );
    labelB14B5->setPixmap( bb14 & (1 << 5) ? redPix : greenPix );
    labelB14B6->setPixmap( bb14 & (1 << 6) ? redPix : greenPix );
    labelB14B7->setPixmap( bb14 & (1 << 7) ? redPix : greenPix );


    labelB15B0->setPixmap( bb15 & (1 << 0) ? redPix : greenPix );
    labelB15B1->setPixmap( bb15 & (1 << 1) ? redPix : greenPix );
    labelB15B2->setPixmap( bb15 & (1 << 2) ? redPix : greenPix );
    labelB15B3->setPixmap( bb15 & (1 << 3) ? redPix : greenPix );
    labelB15B4->setPixmap( bb15 & (1 << 4) ? redPix : greenPix );
    labelB15B5->setPixmap( bb15 & (1 << 5) ? redPix : greenPix );
    labelB15B6->setPixmap( bb15 & (1 << 6) ? redPix : greenPix );
    labelB15B7->setPixmap( bb15 & (1 << 7) ? redPix : greenPix );


    byte bb19 = cnc->getByte(19);
    labelB19B0->setPixmap( bb19 & (1 << 0) ? redPix : greenPix );
    labelB19B1->setPixmap( bb19 & (1 << 1) ? redPix : greenPix );
    labelB19B2->setPixmap( bb19 & (1 << 2) ? redPix : greenPix );
    labelB19B3->setPixmap( bb19 & (1 << 3) ? redPix : greenPix );
    labelB19B4->setPixmap( bb19 & (1 << 4) ? redPix : greenPix );
    labelB19B5->setPixmap( bb19 & (1 << 5) ? redPix : greenPix );
    labelB19B6->setPixmap( bb19 & (1 << 6) ? redPix : greenPix );
    labelB19B7->setPixmap( bb19 & (1 << 7) ? redPix : greenPix );

    // end debug

    // bttons start/stop/pause of task
    groupBoxExec->setEnabled( cncConnected);

    if (cncConnected) {
#if 0

        if (mainTaskTimer.isActive()) {
            toolRun->setEnabled( false );

            if (Task::Status == Paused) {
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

        if (Task::Status == Waiting) {
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

        if (Task::Status == Working) {
            statusProgress->setValue( cnc->numberCompleatedInstructions());
            //listGkodeForUser.Rows[cnc->NumberComleatedInstructions].Selected = true;
            //TODO: to overwork it, because of resetting of selected ragne
            //listGCodeWidget->currentIndex() = cnc->NumberComleatedInstructions;
            toolRun->setEnabled( false );
            toolStop->setEnabled( true);
            toolPause->setEnabled( true );
        }

        if (Task::Status == Stop) {
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

        if (Task::Status == Paused) {
            toolRun->setEnabled( false );
            toolStop->setEnabled(false);
            toolPause->setEnabled( true);
        }

#if USE_OPENGL == true

        if (enableOpenGL == true) {
            scene3d->Draw();
            scene3d->updateGL();
        }

#endif
    } else {
        toolRun->setEnabled( cncConnected);
        toolPause->setEnabled( cncConnected);
        toolStop->setEnabled( cncConnected);

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
void MainWindow::fillListWidget(QStringList listCode)
{
    listGCodeWidget->clear();
    listGCodeWidget->setRowCount( 0);
    listGCodeWidget->setColumnCount(3);
    QStringList header = (QStringList() << translate(_COMMAND) << translate(_INFO) << translate(_STATE));

    listGCodeWidget->setHorizontalHeaderLabels(header);
    listGCodeWidget->setRowCount( listCode.count() );

    for(int i = 0; i < listCode.count(); i++) {
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


#if USE_OPENGL == true

    if (enableOpenGL == true) {
        scene3d->matrixReloaded();
    }

#endif
}


/**
 * @brief detect the min and max ranges
 *
 * @param[in] pos actual index in GCode data list, if pos is 0: init of min/max
 *
 */
void MainWindow::detectMinMax(int pos)
{
    if (pos > 0 && pos < gCodeList.size()) {
        if (gCodeList.at(pos).X > Settings::coord[X].softLimitMax) {
            Settings::coord[X].softLimitMax = gCodeList.at(pos).X;
        }

        if (gCodeList.at(pos).X < Settings::coord[X].softLimitMin) {
            Settings::coord[X].softLimitMin = gCodeList.at(pos).X;
        }

        if (gCodeList.at(pos).Y > Settings::coord[Y].softLimitMax) {
            Settings::coord[Y].softLimitMax = gCodeList.at(pos).Y;
        }

        if (gCodeList.at(pos).Y < Settings::coord[Y].softLimitMin) {
            Settings::coord[Y].softLimitMin = gCodeList.at(pos).Y;
        }

        if (gCodeList.at(pos).Z > Settings::coord[Z].softLimitMax) {
            Settings::coord[Z].softLimitMax = gCodeList.at(pos).Z;
        }

        if (gCodeList.at(pos).Z < Settings::coord[Z].softLimitMin) {
            Settings::coord[Z].softLimitMin = gCodeList.at(pos).Z;
        }

        return;
    }

    if (pos == 0) {
        Settings::coord[X].softLimitMax = gCodeList.at(pos).X;
        Settings::coord[X].softLimitMin = gCodeList.at(pos).X;
        Settings::coord[Y].softLimitMax = gCodeList.at(pos).Y;
        Settings::coord[Y].softLimitMin = gCodeList.at(pos).Y;
        Settings::coord[Z].softLimitMax = gCodeList.at(pos).Z;
        Settings::coord[Z].softLimitMin = gCodeList.at(pos).Z;
    }
}


/**
 * @brief function patches the data list before sending to mk1
 *
 * the data list will be patched dependend from current user settings:
 * speed, steps per mm and other. we need to patch data in case of settings changing
 */
void MainWindow::fixGCodeList()
{
    if (gCodeList.count() < 2) {
        return;
    }

    detectMinMax(0);

    maxLookaheadAngleRad = Settings::maxLookaheadAngle * PI / 180.0;// grad to rad

    // calculate the number of steps in one direction, if exists
    for (int idx = 0; idx < gCodeList.size(); idx++) {
        detectMinMax(idx);

        if (gCodeList[idx].movingCode == RAPID_LINE_CODE) {
            continue;
        }

        int endPos = calculateMinAngleSteps(idx); // and update the pos

        if (endPos >= 1) {
            patchSpeedAndAccelCode(idx, endPos);
            idx = endPos;
            continue;
        }
    }

#if 1

    // now debug
    for (int i = 0; i < gCodeList.size(); i++) {
        qDebug() << i << "line:" << gCodeList[i].numberLine << "accel:" << (hex) << gCodeList[i].movingCode << (dec) << "max coeff:" << gCodeList[i].vectorCoeff << "splits:" <<  gCodeList[i].splits
                 << "steps:" << gCodeList[i].stepsCounter << "vector speed:" << gCodeList[i].vectSpeed << "coords:" << gCodeList[i].X << gCodeList[i].Y << "delta angle:" << gCodeList[i].deltaAngle;
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
 * gCodeList [begPos .. endPos]
 *
 * @param[in] begPos from this position in gcode list
 * @param[in] endPos inclusively end position
 *
 */
void MainWindow::patchSpeedAndAccelCode(int begPos, int endPos)
{
    if (begPos < 1 || begPos >= gCodeList.count() - 1) {
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
    if ((Settings::coord[X].maxVelo != 0.0) && (Settings::coord[X].pulsePerMm != 0.0)) {
        dnewSpdX = 7.2e8 / ((float)Settings::coord[X].maxVelo * Settings::coord[X].pulsePerMm);
    }

    if ((Settings::coord[Y].maxVelo != 0.0) && (Settings::coord[Y].pulsePerMm != 0.0)) {
        dnewSpdY = 7.2e8 / ((float)Settings::coord[Y].maxVelo * Settings::coord[Y].pulsePerMm);
    }

    if ((Settings::coord[Z].maxVelo != 0.0) && (Settings::coord[Z].pulsePerMm != 0.0)) {
        dnewSpdZ = 7.2e8 / ((float)Settings::coord[Z].maxVelo * Settings::coord[Z].pulsePerMm);
    }

    switch (gCodeList[begPos].plane) {
        case XY: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                detectMinMax(i);
                float dX = fabs(gCodeList.at(i - 1).X - gCodeList.at(i).X);
                float dY = fabs(gCodeList.at(i - 1).Y - gCodeList.at(i).Y);
                float dH = sqrt(dX * dX + dY * dY);
                float coeff = 1.0;

                if (dX > dY) {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    // calculation of vect speed
                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeList[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                } else {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeList[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                }

                sumSteps += gCodeList[i].stepsCounter;

                gCodeList[i].vectorCoeff = coeff;
            }

            break;
        }

        case YZ: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                detectMinMax(i);

                float dY = fabs(gCodeList.at(i - 1).Y - gCodeList.at(i).Y);
                float dZ = fabs(gCodeList.at(i - 1).Z - gCodeList.at(i).Z);
                float dH = sqrt(dZ * dZ + dY * dY);
                float coeff = 1.0;

                if (dY > dZ) {
                    if (dY != 0.0) {
                        coeff = dH / dY;
                    }

                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdY); //
                    gCodeList[i].stepsCounter = qRound(dY * (float)Settings::coord[Y].pulsePerMm);
                } else {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeList[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                }

                sumSteps += gCodeList[i].stepsCounter;

                gCodeList[i].vectorCoeff = coeff;
            }

            break;
        }

        case ZX: {
            //* this loop is in the switch statement because of optimisation
            for (int i = begPos; i <= endPos; i++) {
                detectMinMax(i);

                float dZ = fabs(gCodeList.at(i - 1).Z - gCodeList.at(i).Z);
                float dX = fabs(gCodeList.at(i - 1).X - gCodeList.at(i).X);
                float dH = sqrt(dX * dX + dZ * dZ);
                float coeff = 1.0;

                if (dZ > dX) {
                    if (dZ != 0.0) {
                        coeff = dH / dZ;
                    }

                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdZ); //
                    gCodeList[i].stepsCounter = qRound(dZ * (float)Settings::coord[Z].pulsePerMm);
                } else {
                    if (dX != 0.0) {
                        coeff = dH / dX;
                    }

                    gCodeList[i].vectSpeed = (int)(coeff * dnewSpdX); //
                    gCodeList[i].stepsCounter = qRound(dX * (float)Settings::coord[X].pulsePerMm);
                }

                sumSteps += gCodeList[i].stepsCounter;

                gCodeList[i].vectorCoeff = coeff;
            }

            break;
        }

        default: {
            qDebug() << "no plane information: pos " << begPos << "x" << gCodeList[begPos].X << "y" << gCodeList[begPos].Y << "z" << gCodeList[begPos].Z;
        }
    }

    if (sumSteps > 0) {
        // now for steps
        for (int i = begPos; i < endPos; i++) {
            int tmpStps;
            tmpStps = gCodeList[i].stepsCounter;
            gCodeList[i].stepsCounter = sumSteps;
            sumSteps -= tmpStps;
            gCodeList[i].movingCode = CONSTSPEED_CODE;
        }

        gCodeList[begPos].movingCode = ACCELERAT_CODE;
        gCodeList[endPos].movingCode = DECELERAT_CODE;
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

    if (startPos > gCodeList.count() - 1 || startPos < 1) {
        qDebug() << "steps counter bigger than list";
        return -1;
    }

#if 1

    if (gCodeList.at(startPos).splits > 0) { // it's arc, splits inforamtion already calculated
        idx += gCodeList.at(startPos).splits;
        return idx;
    }

#endif

    // or for lines
    for (idx = startPos; idx < gCodeList.count() - 1; idx++) {
#if 1

        if (gCodeList.at(idx).movingCode == ACCELERAT_CODE && gCodeList.at(idx).splits > 0) {
            idx += gCodeList.at(idx).splits;
            return idx;
        }

#endif

        if (gCodeList.at(idx + 1).movingCode == RAPID_LINE_CODE) {
            return idx;
        }

#if 0
        qDebug() << "found diff accel code" << startPos << idx << (hex) << gCodeList.at(idx).movingCode << gCodeList[idx + 1].movingCode
                 << "coordinates" << (dec) << gCodeList.at(idx).X << gCodeList.at(idx).Y << gCodeList[idx + 1].X << gCodeList[idx + 1].Y;
#endif

        float a1 = gCodeList.at(idx).angle;
        float a2 = gCodeList.at(idx + 1).angle;

        gCodeList[idx].deltaAngle = (a1 - a2);

        if (fabs(gCodeList[idx].deltaAngle) > fabs(PI - maxLookaheadAngleRad)) {
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
    SaveFile();
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

    lastFiles.insert(0, nm);
    lastFiles.removeDuplicates();

    //     qDebug() << lastFiles;

    reloadRecentList();

    QStringList l = getGoodList();
    fillListWidget(l);

    l = getBadList();

    if (l.count() != 0) {
        foreach (QString s, l) {
            AddLog(s);
        }
    } else {
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
    //   qDebug() << "onSetting";
    SettingsDialog *setfrm = new SettingsDialog(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        cnc->sendSettings();
        writeSettings();
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
 * @brief slot for copying of actual coordinates into the "move to coordinates"
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
    if (cnc->isMistOn()) {
        actionMist->setIcon(QIcon(QPixmap(":/images/mist_off.png")));
        cnc->mistOFF();
    } else {
        actionMist->setIcon(QIcon(QPixmap(":/images/mist_on.png")));
        cnc->mistON();
    }
}


/**
 * @brief slot for enabling/disabling of fluid coolant on mk1
 *
 */
void MainWindow::onFluid()
{
    if (cnc->isFluidOn()) {
        actionFluid->setIcon(QIcon(QPixmap(":/images/coolant_off.png")));
        cnc->fluidOFF();
    } else {
        actionFluid->setIcon(QIcon(QPixmap(":/images/coolant_on.png")));
        cnc->fluidON();
    }
}

/**
 * @brief slot for enabling/disabling of spindle on mk1
 *
 */
void MainWindow::onSpindel()
{
    if (cnc->isSpindelOn()) {
        actionSpindle->setIcon(QIcon(QPixmap(":/images/forward_off.png")));
        cnc->spindleOFF();
    } else {
        actionSpindle->setIcon(QIcon(QPixmap(":/images/forward_on.png")));
        cnc->spindleON();
    }
}


/**
 * @brief slot for emergy stop on mk1
 *
 */
void MainWindow::onEmergyStop()
{
    cnc->emergyStop();
}


/**
 * @brief
 *
 */
void MainWindow::scanSurface()
{
    ShowSurface = true;

    ScanSurfaceDialog *dlg = new ScanSurfaceDialog(this);
    dlg->exec();

    delete dlg;
}


/**
 * @brief  slot for 3d settings of program
 *
 */
void MainWindow::on3dSettings()
{
#if USE_OPENGL == true

    if (enableOpenGL == true) {
        // 3d settings
        Settings3dDialog *dlg = new Settings3dDialog(this);
        dlg->exec();

        scene3d->updateGL();

        delete dlg;
    }

#endif
}


/**
 * @brief
 *
 */
void MainWindow::onScanSurface()
{
    //scan surfcae
    scanSurface();
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
    cnc->deviceNewPosition(0, Settings::coord[Y].actualPosPulses, Settings::coord[Z].actualPosPulses, Settings::coord[A].actualPosPulses);
    numPosX->setValue(0.0);
}


/**
 * @brief slot for resetting of "y" coordinate
 *
 */
void MainWindow::onButtonYtoZero()
{
    cnc->deviceNewPosition(Settings::coord[X].actualPosPulses, 0, Settings::coord[Z].actualPosPulses, Settings::coord[A].actualPosPulses);
    numPosY->setValue(0.0);
}


/**
 * @brief slot for resetting of "z" coordinate
 *
 */
void MainWindow::onButtonZtoZero()
{
    cnc->deviceNewPosition(Settings::coord[X].actualPosPulses, Settings::coord[Y].actualPosPulses, 0, Settings::coord[A].actualPosPulses);
    numPosZ->setValue(0.0);
}


/**
 * @brief slot for resetting of "a" angle
 *
 */
void MainWindow::onButtonAtoZero()
{
    cnc->deviceNewPosition(Settings::coord[X].actualPosPulses, Settings::coord[Y].actualPosPulses, Settings::coord[Z].actualPosPulses, 0);
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
