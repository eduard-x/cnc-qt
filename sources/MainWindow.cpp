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


void MessageTimerBox::showEvent ( QShowEvent * event )
{
    currentTime = 0;

    if (autoClose) {
        this->startTimer(1000);
    }
}


void MessageTimerBox::setDefaultText(const QString &t)
{
    defaultText = t;
}


void MessageTimerBox::setAutoClose(bool b)
{
    autoClose = b;
}


void MessageTimerBox::setTimeout(int t)
{
    timeout = t;
    QString tx;

    tx = defaultText;
    tx.replace("%d", QString::number(timeout));
    setText(tx);
}


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
int Task::posCodeStart = -1;
int Task::posCodeEnd = -1;
int Task::posCodeNow = -1;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), Reader ()
{
    setupUi(this);

    textLog->document()->setMaximumBlockCount(4000);

    setWindowTitle(translate(_PROG_NAME));

    currentAppDir = qApp->applicationDirPath();

    if (currentAppDir.lastIndexOf("/build") > 0) { // build dir detection
        currentAppDir.remove("/build" );
    }

#if USE_OPENGL == false
    enableOpenGL = false;
#else
    QString d = getenv( "DISPLAY" ); // linux machines only!

    // to disable the OpenGL features, if over ssh
    enableOpenGL = (d.indexOf(":0") == 0);
#endif
    currentLang = "English";

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

    readGUISettings();

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
    cnc->loadSettings();

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

    for (QVector<QAction*>::iterator itL = actLangSelect.begin(); itL != actLangSelect.end(); ++itL) {
        if ((*itL)->text() == currentLang) {
            (*itL)->setChecked(true);
            break;
        }
    }

    QStringList arguments = QCoreApplication::arguments();

    if (arguments.size() > 1) {
        if (arguments.at(1).length() > 0) { // as parameter is file name to load
            QString nm = arguments.at(1);
            OpenFile(nm);

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

    statusProgress->setRange(0, 100);
    statusProgress->setValue(0);

    translateGUI();

    refreshElementsForms();
};


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


void MainWindow::writeGUISettings()
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
    //     int i = 0;
    //
    //     for (QStringList::Iterator iDir = lastDirs.begin(); iDir != lastDirs.end(); iDir++, i++) {
    //         if (i > 8) { // max last dirs
    //             break;
    //         }
    //
    //         s->setValue("LASDIR" + QString::number(i), (*iDir));
    //     }

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

    s->sync();
}


void MainWindow::readGUISettings()
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

    //     for (int i = 0; i < 8; i++) {
    //         QString d = s->value("LASDIR" + QString::number(i)).toString();
    //         QDir dr;
    //
    //         if (d.length() == 0) {
    //             break;
    //         }
    //
    //         if (dr.exists(d) == true) {
    //             lastDirs << d;
    //         }
    //     }

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
}


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


void MainWindow::getZRotation(int z)
{
    zAngle = z;
    displayRotation();
}


void MainWindow::getFPS(int f)
{
    statusLabel2->setText( "OpenGL, FPS: " + QString::number(f));
}


void MainWindow::getXRotation(int x)
{
    xAngle = x;
    displayRotation();
}


void MainWindow::getYRotation(int y)
{
    yAngle = y;
    displayRotation();
}


void MainWindow::getScale(int s)
{
    scale = s;
    displayRotation();
}


void MainWindow::displayRotation()
{
    posAngleX->setText( QString().sprintf("%d°", xAngle));
    posAngleY->setText( QString().sprintf("%d°", yAngle));
    posAngleZ->setText( QString().sprintf("%d°", zAngle));
}


MainWindow::~MainWindow()
{
};


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

    writeGUISettings();

    delete cnc;

    ce->accept();

    QCoreApplication::quit();
}


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

    writeGUISettings();

    delete cnc;

    QCoreApplication::quit();
}


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

    actionProgram->setText(translate(_PROGRAM));
    //     actionOpenGL->setText(translate(_OPENGL));
    //     actionConnect->setText(translate(_CONNECT));
    //     actionDisconnect->setText(translate(_DISCONNECT));
    actionAbout->setText(translate(_ABOUT));

    listGCodeWidget->setHorizontalHeaderLabels((QStringList() << translate(_COMMAND) << translate(_INFO) << translate(_STATE)));
}


void MainWindow::onStartTask()
{
    if (mainTaskTimer.isActive()) {
        return;    //timer is active, task is running
    }

    if (!cnc->isConnected()) {
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_CONN), QMessageBox::Critical);
        return;
    }

    if (GCodeList.count() == 0) {
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
    Task::posCodeStart = beg;
    Task::posCodeEnd = end;
    Task::posCodeNow = Task::posCodeStart;

    QString s = translate(_FROM_TO).arg( Task::posCodeStart + 1).arg( Task::posCodeEnd + 1);
    labelTask->setText( s );

    statusProgress->setRange(Task::posCodeStart, Task::posCodeEnd);

    groupManualControl->setChecked( false ); // disable manual control

    Task::Status = Start;

    mainTaskTimer.start();

    refreshElementsForms();
}


void MainWindow::onPauseTask()
{
    if (Task::Status == Start) {
        return;    //if not started, do not set pause
    }

    if (Task::Status == Working || Task::Status == Paused) {
        Task::Status = (Task::Status == Paused) ? Working : Paused;
    }

    refreshElementsForms();
}


void MainWindow::onStopTask()
{
    if (Task::Status == Waiting) {
        return;
    }

    Task::Status = Stop;
    refreshElementsForms();
}


// return value: false if timer to stop
bool MainWindow::runCommand()
{
    // Velocity from main form
    int userSpeedG1 = (int)numVeloSubmission->value();
    int userSpeedG0 = (int)numVeloMoving->value();

    if (Task::posCodeNow > Task::posCodeEnd) {
        Task::Status = Stop;
        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());

        refreshElementsForms();
        //
        return false;
    }

    GCodeCommand gcodeNow = GCodeList.at(Task::posCodeNow);

    useHome = checkHome->isChecked();

    if (useHome == true) {
        cnc->coord[X].startPos = doubleSpinHomeX->value();
        cnc->coord[Y].startPos = doubleSpinHomeY->value();
        cnc->coord[Z].startPos = doubleSpinHomeZ->value();
        cnc->coord[A].startPos = 0.0;
    } else {
        cnc->coord[X].startPos = cnc->coord[X].actualPosmm;
        cnc->coord[Y].startPos = cnc->coord[Y].actualPosmm;
        cnc->coord[Z].startPos = cnc->coord[Z].actualPosmm;
        cnc->coord[A].startPos = 0.0;
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
        cnc->packCA(cnc->coord[X].startPos, cnc->coord[Y].startPos, cnc->coord[Z].startPos + 10.0, cnc->coord[A].startPos, userSpeedG0, 0, 0, 0.0);

        cnc->packCA(gcodeNow.X, gcodeNow.Y, cnc->coord[Z].startPos + 10.0, gcodeNow.A , userSpeedG0, gcodeNow.angleVectors, gcodeNow.Distance);

        Task::Status = Working;

        refreshElementsForms();

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

        refreshElementsForms();

        return false;
    }

    // Working

    if (Task::Status != Working) {
        refreshElementsForms();

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
    if (cnc->availableBufferSize() < 5) {
        return true;    // nothing before buffer clean
    }


    //TODO: to add in parameter the value
    if (Task::posCodeNow > (cnc->numberCompleatedInstructions() + 3)) {
        return true;    // don't send more than 3 commands
    }

    //command G4 or M0
    if (gcodeNow.needPause) {
        if (gcodeNow.mSeconds == 0) { // M0 - waiting command
            Task::Status = Paused;

            //pause before user click
            MessageBox::exec(this, translate(_PAUSE), translate(_RECIEVED_M0), QMessageBox::Information);
        } else {
            QString msg = translate(_PAUSE_G4);
            statusLabel2->setText( msg.arg(QString::number(gcodeNow.mSeconds)));

            QThread().wait(gcodeNow.mSeconds); // pause in msec

            statusLabel2->setText( "" );
        }

        refreshElementsForms();
    }

    //replace instrument
    if (gcodeNow.changeInstrument) {
        Task::Status = Paused;
        refreshElementsForms();

        //pause before user click
        QString msg = translate(_PAUSE_ACTIVATED);
        MessageBox::exec(this, translate(_PAUSE), msg.arg(QString::number(gcodeNow.numberInstrument)).arg(QString::number(gcodeNow.diametr)), QMessageBox::Information);
    }

    float pointX = gcodeNow.X;
    float pointY = gcodeNow.Y;
    float pointZ = gcodeNow.Z;
    float pointA = gcodeNow.A;

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
            pointZ += GetDeltaZ(pointX, pointY);
        }
    }

    //TODO: additional velocity control manual/automatical
    int speed = (gcodeNow.workspeed) ? userSpeedG1 : userSpeedG0;

    //     cnc->packCA(posX, posY, posZ, posA, speed, Task::posCodeNow);
    cnc->packCA(pointX, pointY, pointZ, pointA, speed, Task::posCodeNow, 0.0, 0);

    Task::posCodeNow++;
    labelRunFrom->setText( translate(_CURRENT_LINE) + " " + QString::number(Task::posCodeNow + 1));

    refreshElementsForms();

    return true;
}


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

// OTHER

// log output
void MainWindow::AddLog(QString _text)
{
    if (_text == NULL) {
        return;
    }

    textLog->append(_text);
}


//
void MainWindow::onStatus()
{
    // clean message
    statusLabel2->setText( "" );
}


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

    cnc->packCA(posX, posY, posZ, posA, speed, 0, 0.0, 0);

    cnc->packFF();

    cnc->pack9D();

    cnc->pack9E(0x02);

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();
}


// moving to the point
void MainWindow::onRunToPoint()
{
    if (!cnc->testAllowActions()) {
        return;
    }

    moveToPoint();
}


//DEBUGGING generator PWM
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


void MainWindow::onSendCommand()
{
    SendSignal();
}


// void MainWindow::onLikePoint()
// {
//     //TODO: open popup with list of points
// }


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


void MainWindow::onGeneratorCode()
{
    //     GeneratorCodeDialog *frm = new GeneratorCodeDialog(this);
    //     frm->exec()
}



float MainWindow::GetDeltaZ(float _x, float _y)
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


// slot for logging signal
void MainWindow::onCncMessage(int n_msg)
{
    textLog->append(QDateTime().currentDateTime().toString() + " - " + translate(n_msg));
}


// slot FROM COMTROLLER, new data
void MainWindow::onCncNewData()
{
    refreshElementsForms();

    //сдвинем границы
    if (ShowGrate) {
        if (cnc->coord[X].posMm() < grateXmin) {
            grateXmin = cnc->coord[X].posMm();
        }

        if (cnc->coord[X].posMm() > grateXmax) {
            grateXmax = cnc->coord[X].posMm();
        }

        if (cnc->coord[Y].posMm() < grateYmin) {
            grateYmin = cnc->coord[Y].posMm();
        }

        if (cnc->coord[Y].posMm() > grateYmax) {
            grateYmax = cnc->coord[Y].posMm();
        }
    }
}


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

    numPosX->setValue( cnc->coord[X].posMm());
    numPosY->setValue( cnc->coord[Y].posMm());
    numPosZ->setValue( cnc->coord[Z].posMm());

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

    maxXLED->setPixmap( bb15 & (1 << 0) ? redPix : greenPix /*cnc->coord[X].limitMax ? redPix : greenPix*/ );
    minXLED->setPixmap( bb15 & (1 << 1) ? redPix : greenPix /*cnc->coord[X].limitMin ? redPix : greenPix*/ );
    maxYLED->setPixmap( bb15 & (1 << 2) ? redPix : greenPix /*cnc->coord[Y].limitMax ? redPix : greenPix*/ );
    minYLED->setPixmap( bb15 & (1 << 3) ? redPix : greenPix /*cnc->coord[Y].limitMin ? redPix : greenPix*/ );
    maxZLED->setPixmap( bb15 & (1 << 4) ? redPix : greenPix /* cnc->coord[Z].limitMax ? redPix : greenPix*/ );
    minZLED->setPixmap( bb15 & (1 << 5) ? redPix : greenPix /*cnc->coord[Z].limitMin ? redPix : greenPix*/ );
    maxALED->setPixmap( bb15 & (1 << 6) ? redPix : greenPix /*cnc->coord[A].limitMax ? redPix : greenPix */);
    minALED->setPixmap( bb15 & (1 << 7) ? redPix : greenPix /*cnc->coord[A].limitMin ? redPix : greenPix */);

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


void MainWindow::onManualControlDialog()
{
    ManualControlDialog *mc = new ManualControlDialog(this);
    mc->exec();
    delete mc;
}


//
//  the table widget with data
//
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

    Task::posCodeStart = 0;
    Task::posCodeEnd = listGCodeWidget->rowCount();

    tabWidget->setCurrentIndex(0);

    statusProgress->setRange(0, listGCodeWidget->rowCount());
    statusProgress->setValue(0);

#if USE_OPENGL == true

    if (enableOpenGL == true) {
        scene3d->matrixReloaded();
    }

#endif
}


void MainWindow::onSaveFile()
{
    SaveFile();
}


void MainWindow::onOpenFile()
{
    QString nm;

    statusProgress->setValue(0);

    if (OpenFile(nm) == false) {
        AddLog("File loading error: " + nm );
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
        AddLog("File loaded: " + nm );
    }
}


void MainWindow::onCalcVelocity()
{
    CuttingCalc *setfrm = new CuttingCalc(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        writeGUISettings();
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


void MainWindow::onSettings()
{
    //   qDebug() << "onSetting";
    SettingsDialog *setfrm = new SettingsDialog(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Accepted) {
        cnc->saveSettings();
    }
}


void MainWindow::onCopyHome()
{
    doubleSpinHomeX->setValue( numPosX->value() );
    doubleSpinHomeY->setValue( numPosY->value() );
    doubleSpinHomeZ->setValue( numPosZ->value() );
}


void MainWindow::onSetHome()
{
}


void MainWindow::onCopyPos()
{
    doubleSpinMoveX->setValue( numPosX->value() );
    doubleSpinMoveY->setValue( numPosY->value() );
    doubleSpinMoveZ->setValue( numPosZ->value() );
}


void MainWindow::onLogClear()
{
    textLog->clear();
}


void MainWindow::onAbout()
{
    AboutDialog *dlg = new AboutDialog(this);
    dlg->exec();

    delete dlg;
}


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


void MainWindow::onEmergyStop()
{
    cnc->emergyStop();
}


void MainWindow::Feed()
{
    ShowSurface = true;

    ScanSurfaceDialog *dlg = new ScanSurfaceDialog(this);
    dlg->exec();

    delete dlg;
}


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


void MainWindow::onScanSurface()
{
    //scan surfcae
    Feed();
}


void MainWindow::onCellSelect(int row, int col)
{
}


void MainWindow::onEditGCode(int row, int col)
{
    EditGCodeDialog *dlg = new EditGCodeDialog(this);

    dlg->exec();

    delete dlg;
}


void MainWindow::onButtonXtoZero()
{
    cnc->deviceNewPosition(0, cnc->coord[Y].actualPosPulses, cnc->coord[Z].actualPosPulses, cnc->coord[A].actualPosPulses);
    numPosX->setValue(0.0);
}


void MainWindow::onButtonYtoZero()
{
    cnc->deviceNewPosition(cnc->coord[X].actualPosPulses, 0, cnc->coord[Z].actualPosPulses, cnc->coord[A].actualPosPulses);
    numPosY->setValue(0.0);
}


void MainWindow::onButtonZtoZero()
{
    cnc->deviceNewPosition(cnc->coord[X].actualPosPulses, cnc->coord[Y].actualPosPulses, 0, cnc->coord[A].actualPosPulses);
    numPosZ->setValue(0.0);
}


void MainWindow::onButtonAtoZero()
{
    cnc->deviceNewPosition(cnc->coord[X].actualPosPulses, cnc->coord[Y].actualPosPulses, cnc->coord[Z].actualPosPulses, 0);
    numAngleGrad->setValue(0.0);
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
