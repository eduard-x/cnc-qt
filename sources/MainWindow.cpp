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


#include <QApplication>
#include <QMainWindow>
#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QToolButton>

#include <QtOpenGL>

#include "includes/Settings.h"
#include "includes/About.h"
#include "includes/Reader.h"
#include "includes/EditGCode.h"
#include "includes/ManualControl.h"
#include "includes/ScanSurface.h"
// #include "includes/Translator.h"
#include "includes/Settings3d.h"
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
EStatusTask  Task::StatusTask = Waiting;
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

    currentLang = "English";

    QFont sysFont = qApp->font();
    sysFont = sysFont;

    fontSize = sysFont.pointSize();

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

    // 1 Подключение событий от контроллера
    cnc = new mk1Controller();
    cnc->loadSettings();

    // OpenGL area

    scene3d = new GLWidget(this);

    scrollArea->setWidget(scene3d);

    OpenGL_preview->addWidget(scrollArea, 0, 0);

    // OpenGL is placed in widget

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

            //             scene3d->matrixReloaded();
        }
    }

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

    //     connect(actionConnect, SIGNAL(triggered()), this, SLOT(onConnect()));
    //     connect(actionDisconnect, SIGNAL(triggered()), this, SLOT(onDisconnect()));
    //     connect(actionConnectDisconnect, SIGNAL(triggered()), this, SLOT(onConnDisconnect()));
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(onSettings()));

    connect(pushManualControl, SIGNAL(clicked()), this, SLOT(onManualControlDialog()));

    connect(toolResetCoorX, SIGNAL(clicked()), this, SLOT(onButtonXtoZero()));
    connect(toolResetCoorY, SIGNAL(clicked()), this, SLOT(onButtonYtoZero()));
    connect(toolResetCoorZ, SIGNAL(clicked()), this, SLOT(onButtonZtoZero()));

    connect(pushClean, SIGNAL(clicked()), this, SLOT(onLogClear()));
    connect(actionSpindle, SIGNAL(triggered()), this, SLOT(onSpindel()));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(onEmergyStop()));
    connect(actionScan, SIGNAL(triggered()), this, SLOT(onScanSurface()));

    connect(toolRun, SIGNAL(clicked()), this, SLOT(onStartTask()));
    connect(toolPause, SIGNAL(clicked()), this, SLOT(onPauseTask()));
    connect(toolStop, SIGNAL(clicked()), this, SLOT(onStopTask()));

    connect(actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    connect(actionAboutQt, SIGNAL(triggered()), this, SLOT(onAboutQt()));

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


    // 3d buttons
    connect(posAngleXm, SIGNAL(pressed()), scene3d, SLOT(onPosAngleXm()));
    connect(posAngleX, SIGNAL(clicked()), scene3d, SLOT(onPosAngleX())); // reset to 0
    connect(scene3d, SIGNAL(xRotationChanged(int)), this, SLOT(getXRotation(int)));
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

    onCncHotplug();
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

        if (fLang.open(QIODevice::ReadOnly)) {      //wird eingelesen
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
    s = new QSettings(QSettings::UserScope, "CNCSoft", "CNC-Qt" );
    s->setValue("pos", pos());
    s->setValue("size", size());
    //     s->setValue("WorkDir", currentWorkDir);
    s->setValue("LANGUAGE", currentLang);

    s->setValue("VelocitySubmission", numVeloSubmission->value());
    s->setValue("VelocityMoving", numVeloMoving->value());
    s->setValue("VelocityManual", numVeloManual->value());

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

    s->sync();
}


void MainWindow::readGUISettings()
{
    QSettings* s;
    s = new QSettings(QSettings::UserScope, "CNCSoft", "CNC-Qt" );
    QPoint pos = s->value("pos", QPoint(200, 200)).toPoint();
    QSize size = s->value("size", QSize(840, 640)).toSize();
    resize(size);
    move(pos);


    veloSubmission = s->value("VelocitySubmission", 200).toInt();
    veloMoving = s->value("VelocityMoving", 500).toInt();
    veloManual = s->value("VelocityManual", 400).toInt();

    numVeloSubmission->setValue(veloSubmission);
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
    posAngleX->setText( QString::number(xAngle) + "°" );
    posAngleY->setText( QString::number(yAngle) + "°" );
    posAngleZ->setText( QString::number(zAngle) + "°" );
}


MainWindow::~MainWindow()
{
};


void MainWindow::closeEvent(QCloseEvent* ce)
{
    if (cnc->isConnected()) {
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

    ce->accept();

    QCoreApplication::quit();
}


void MainWindow::onExit()
{
    if (cnc->isConnected()) {
        MessageBox::exec(this, translate(_WARN), translate(_MSG_FOR_DISABLE), QMessageBox::Critical);
        return;
    }

    if (MessageBox::exec(this, translate(_EXIT), translate(_REALLYQUIT), QMessageBox::Question) == QMessageBox::No) {
        //         ce->ignore();
        return;
    }

    disconnect(cnc, SIGNAL(Message (int)), this, SLOT(onCncMessage(int))); // cnc->Message -= CncMessage;

    writeGUISettings();

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
    groupControl->setTitle(translate(_MANUAL_CONTROL));
    groupIndicator->setTitle(translate(_DISPL_LIMITS));
    groupVelocity->setTitle(translate(_VELOCITY));

    toolRunMoving->setText(translate(_RUN));

    pushClean->setText(translate(_CLEAN));
    pushSendSignal->setText(translate(_GEN_SIGNAL));
    pushManualControl->setText(translate(_MANUAL_CONTROL));

    radioButtonOff->setText(translate(_OFF));
    radioButtonHz->setText(translate(_HZ));
    radioButtonRC->setText(translate(_RC));

    labelPWMVelo->setText(translate(_VELO_PWM));
    labelPWMCHan->setText(translate(_CHAN_PWM));

    tabWidget->setTabText(0, translate(_DATA));
    tabWidget->setTabText(1, translate(_3D_VIEW));
    tabWidget->setTabText(2, translate(_ADDITIONAL));
    tabWidget->setTabText(3, translate(_LOG));

    labelSubmission->setText(translate(_SUBMISSION));
    labelMoving->setText(translate(_MOVING));

    checkBoxEnSpindnle->setText(translate(_ON_SPINDLE));

    labelVelo->setText(translate(_VELO));

    labelRotat->setText(translate(_ROTATION));

    labelMinX->setText(translate(_MIN));
    labelMaxX->setText(translate(_MAX));

    labelMinY->setText(translate(_MIN));
    labelMaxY->setText(translate(_MAX));

    labelMinZ->setText(translate(_MIN));
    labelMaxZ->setText(translate(_MAX));

    labelMinA->setText(translate(_MIN));
    labelMaxA->setText(translate(_MAX));

    //
    labelRunFrom->setText(translate(_FROM_NUM));
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
        return;    //нельзя дальше, если таймер включен
    }

    if (!cnc->isConnected()) {
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_CONN), QMessageBox::Critical);
        return;
    }

    if (GCodeList.count() == 0) {
        // нет данных для выполнения
        MessageBox::exec(this, translate(_ERR), translate(_MSG_NO_DATA), QMessageBox::Critical);
        return;
    }

    int beg = 0;
    int end = 0;

    QItemSelectionModel* selectionModel = listGCodeWidget->selectionModel();

    QModelIndexList selected = selectionModel->selectedRows();

    //если в списке команд не выбрана строчка, то спозиционируемся на первой
    if (selected.count() == 0) {
        end = listGCodeWidget->rowCount();
    }

    if (selected.count() == 1) { //выбрана всего одна строка, а значит для выполнения будет указан диапазон, от этой строки и до конца
        QString msg = translate(_QUEST_START_FROMLINE);
        beg = selected.first().row();

        int dlgres =  MessageBox::exec(this, translate(_START_PROG), msg.arg(QString::number(beg + 1)), QMessageBox::Question);

        if (dlgres == QMessageBox::Cancel) {
            return;
        }

        end = listGCodeWidget->rowCount();
    }

    if (selected.count() > 1) { //выбран диапазон строк
        QString msg = translate(_QUEST_START_FROMTOLINE);

        beg = selected.first().row();
        end = selected.count() + beg;
        int dlgr =  MessageBox::exec(this, translate(_START_PROG),  msg.arg(QString::number(beg + 1 ))
                                     .arg(QString::number(end)), QMessageBox::Question);

        if (dlgr == QMessageBox::Cancel) {
            return;
        }
    }

    //установим границы выполнения
    Task::posCodeStart = beg;
    Task::posCodeEnd = end - 1;
    Task::posCodeNow = Task::posCodeStart;

    //     qDebug() << "start " << Task::posCodeStart << Task::posCodeEnd;
    QString s = "from :" + QString::number( Task::posCodeStart + 1 ) + " to: " + QString::number( Task::posCodeEnd + 1);
    statusSt->setText( s );

    groupControl->setChecked( false ); // отключим реакцию на нажатие NUM-pad

    Task::StatusTask = TaskStart;

    mainTaskTimer.start();

    refreshElementsForms();
}


void MainWindow::onPauseTask()
{
    if (Task::StatusTask == TaskStart) {
        return;    //пока задание не запустилось, нет смысла ставить паузу
    }

    if (Task::StatusTask == TaskWorking || Task::StatusTask == TaskPaused) {
        Task::StatusTask = (Task::StatusTask == TaskPaused) ? TaskWorking : TaskPaused;
    }

    refreshElementsForms();
}


void MainWindow::onStopTask()
{
    if (Task::StatusTask == Waiting) {
        return;
    }

    Task::StatusTask = TaskStop;
    refreshElementsForms();
}


// rreturn value: false if timer to stop
bool MainWindow::runCommand()
{
    // скорость с главной формы
    int userSpeedG1 = (int)numVeloSubmission->value();
    int userSpeedG0 = (int)numVeloMoving->value();

    //     qDebug() << "main timer" << GCodeList.count() << Task::posCodeNow;
    if (Task::posCodeNow >= GCodeList.count()) {
        //         mainTaskTimer.stop();
        return false;
    }

    GCodeCommand gcodeNow = GCodeList.at(Task::posCodeNow);

    // TaskStart
    if (Task::StatusTask == TaskStart) { // init of controller
        AddLog(translate(_START_TASK_AT) + QDateTime().currentDateTime().toString());

        int MaxSpeedX = 100;
        int MaxSpeedY = 100;
        int MaxSpeedZ = 100;
        int MaxSpeedA = 100;

        cnc->pack9E(0x05);

        cnc->packBF(MaxSpeedX, MaxSpeedY, MaxSpeedZ, MaxSpeedA);

        cnc->packC0();

        //так-же спозиционируемся, над первой точкой по оси X и Y
        //TODO: нужно ещё и поднять повыше шпиндель, а пока на 10 мм (продумать реализацию)
        cnc->packCA(DeviceInfo::AxesX_PositionPulse, DeviceInfo::AxesY_PositionPulse, DeviceInfo::AxesZ_PositionPulse + DeviceInfo::CalcPosPulse("Z", 10), DeviceInfo::AxesA_PositionPulse, userSpeedG0, 0);

        //TODO: И продумать реализацию к подходу к точке
        cnc->packCA(DeviceInfo::CalcPosPulse("X", gcodeNow.X), DeviceInfo::CalcPosPulse("Y", gcodeNow.Y), DeviceInfo::AxesZ_PositionPulse + DeviceInfo::CalcPosPulse("Z", 10), DeviceInfo::CalcPosPulse("A", gcodeNow.A), userSpeedG0, 0);

        Task::StatusTask = TaskWorking;
        refreshElementsForms();

        return true; //после запуска дальше код пропустим...
    }

    // TaskStop

    if (Task::StatusTask == TaskStop) {
        //TODO: добавить поднятие фрезы, возможное позиционирование в home

        cnc->packFF();

        cnc->pack9D();

        cnc->pack9E(0x02);

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        cnc->packFF();

        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());
        Task::StatusTask = Waiting;
        //         mainTaskTimer.stop();
        return false;
    }

    // TaskWorking

    if (Task::StatusTask != TaskWorking) {
        return false;
    }

    //Все необходимые команды завершены, пора всё завершить
    if (Task::posCodeNow > Task::posCodeEnd) {
        Task::StatusTask = TaskStop;
        AddLog(translate(_END_TASK_AT) + QDateTime().currentDateTime().toString());

        //         mainTaskTimer.stop();
        return false;
    }

    //TODO: добавить в параметр значение
    if (cnc->availableBufferSize() < 5) {
        return true;    // откажемся от посылки контроллеру, пока буфер не освободится
    }


    //TODO: добавить в параметр и это значение
    if (Task::posCodeNow > (cnc->numberComleatedInstructions() + 3)) {
        return true;    // Так-же не будем много посылать команд, т.е. далеко убегать
    }

    //команда остановки G4 или M0
    if (gcodeNow.needPause) {
        if (gcodeNow.mSeconds == 0) { // M0 - команда ожидания от пользователя
            Task::StatusTask = TaskPaused;
            refreshElementsForms();
            //пауза до клика пользователя
            //             QMessageBox.Show("Получена команда M0 для остановки! для дальнейшего выполнения нужно нажать на кнопку 'пауза'", "Пауза",
            //                              QMessageBoxButtons.OK, QMessageBoxIcon.Asterisk);
            MessageBox::exec(this, translate(_PAUSE), translate(_RECIEVED_M0), QMessageBox::Information);
        } else {
            QString msg = translate(_PAUSE_G4);
            statusSt->setText( msg.arg(QString::number(gcodeNow.mSeconds)));

            QThread().wait(gcodeNow.mSeconds); // пауза в мсек.

            statusSt->setText( "" );
        }
    }

    //команда смены инструмента
    if (gcodeNow.changeInstrument) {
        Task::StatusTask = TaskPaused;
        refreshElementsForms();

        //пауза до клика пользователя
        //         QMessageBox.Show("Активирована ПАУЗА! Установите инструмент №:" + gcodeNow.numberInstrument + " имеющий диаметр: " + gcodeNow.diametr + " мм. и нажмите для продолжения кнопку 'пауза'", "Пауза",
        //                          QMessageBoxButtons.OK, QMessageBoxIcon.Asterisk);
        QString msg = translate(_PAUSE_ACTIVATED);
        MessageBox::exec(this, translate(_PAUSE), msg.arg(QString::number(gcodeNow.numberInstrument)).arg(QString::number(gcodeNow.diametr)), QMessageBox::Information);
    }

    double pointX = gcodeNow.X;
    double pointY = gcodeNow.Y;
    double pointZ = gcodeNow.Z;
    double pointA = gcodeNow.A;

    //добавление смещения G-кода
    if (Correction) {
        // применение пропорций
        pointX *= koeffSizeX;
        pointY *= koeffSizeY;

        //применение смещения
        pointX += deltaX;
        pointY += deltaY;
        pointZ += deltaZ;

        //применение матрицы поверхности детали
        if (deltaFeed) {
            pointZ += GetDeltaZ(pointX, pointY);
        }
    }

    int posX = DeviceInfo::CalcPosPulse("X", pointX);
    int posY = DeviceInfo::CalcPosPulse("Y", pointY);
    int posZ = DeviceInfo::CalcPosPulse("Z", pointZ);
    int posA = DeviceInfo::CalcPosPulse("A", pointA);

    //TODO: доделать управление скоростью ручная/по программе
    int speed = (gcodeNow.workspeed) ? userSpeedG1 : userSpeedG0;

    cnc->packCA(posX, posY, posZ, posA, speed, Task::posCodeNow);

    Task::posCodeNow++;
    labelRunFrom->setText( translate(_FROM_NUM) + QString::number(Task::posCodeNow));

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

// вывод лога
void MainWindow::AddLog(QString _text)
{
    if (_text == NULL) {
        return;
    }

    textLog->append(_text);
}


//разобраться о необходимости
void MainWindow::onStatus()
{
    // Вызовем очистку сообщения
    statusSt->setText( "" );
}


void MainWindow::moveToPoint(bool surfaceScan)
{
    int speed = 0;
    double posX, posY, posZ, posA;

    if (surfaceScan == true) {
        speed = 200;

        if (scanPosY == -1 || scanPosX == -1) {
            return;
        }

        posX = DeviceInfo::CalcPosPulse("X", surfaceMatrix[scanPosY][scanPosX].X);
        posY = DeviceInfo::CalcPosPulse("Y", surfaceMatrix[scanPosY][scanPosX].Y);
        posZ = DeviceInfo::CalcPosPulse("Z", surfaceMatrix[scanPosY][scanPosX].Z);
        posA = DeviceInfo::CalcPosPulse("A", surfaceMatrix[scanPosY][scanPosX].A);
    } else {
        speed = spinMoveVelo->value();

        posX = DeviceInfo::CalcPosPulse("X", doubleSpinMoveX->value());
        posY = DeviceInfo::CalcPosPulse("Y", doubleSpinMoveY->value());
        posZ = DeviceInfo::CalcPosPulse("Z", doubleSpinMoveZ->value());
        posA = DeviceInfo::CalcPosPulse("A", numAngleGrad->value());
    }

    cnc->pack9E(0x05);

    cnc->packBF(speed, speed, speed, speed);

    cnc->packC0();


    cnc->packCA(posX, posY, posZ, posA, speed, 0);

    cnc->packFF();

    cnc->pack9D();

    cnc->pack9E(0x02);

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();

    cnc->packFF();
}


// движение в заданную точку
void MainWindow::onRunToPoint()
{
    if (!cnc->testAllowActions()) {
        return;
    }

    moveToPoint();
}


//ОТЛАДКА генератора ШИМ
void MainWindow::SendSignal()
{
    BinaryData::TypeSignal tSign;

    tSign = BinaryData::None;

    //     if (radioButtonOff->isChecked()) {
    //         tSign = BinaryData::None;
    //     }

    if (radioButtonHz->isChecked()) {
        tSign = BinaryData::Hz;
    }

    if (radioButtonRC->isChecked()) {
        tSign = BinaryData::RC;
    }

    cnc->packB5(checkBoxEnSpindnle->isChecked(), (int)spinBoxChann->value(), tSign, (int)spinBoxVelo->value());
}


void MainWindow::onSendCommand()
{
    SendSignal();
}


// to connect slider with spinbox
// void MainWindow::trackBar1_Scroll()
// {
//
//     //     spinBoxVelo->setValue( trackBar1->value());
//
//     //     SendSignal();
//
//     // cnc->packB5(checkBoxEnSpindnle->isChecked(), (int)spinBoxChann->value(), checkBox19.isChecked(), (int)spinBoxVelo->value());
// }


// void MainWindow::onLikePoint()
// {
//     //TODO: откроем окно со списком точек
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

    statusSt = new QLabel();
    statusSt->setFixedWidth(250);
    statusSt->setFixedHeight(17);
    statusbar->addPermanentWidget(statusSt);
}


void MainWindow::onGeneratorCode()
{
    //     GeneratorCodeDialog *frm = new GeneratorCodeDialog(this);
    //     frm->exec()
}


double MainWindow::GetDeltaZ(double _x, double _y)
{
    //точка которую нужно отобразить
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


//событие для ведения логов
void MainWindow::onCncMessage(int n_msg)
{
    textLog->append(QDateTime().currentDateTime().toString() + " - " + translate(n_msg));
}


// СОБЫТИЯ ОТ КОНТРОЛЛЕРА что получили новые данные
void MainWindow::onCncNewData()
{
    refreshElementsForms();

    //сдвинем границы
    if (ShowGrate) {
        if (DeviceInfo::AxesX_PositionMM() < grateXmin) {
            grateXmin = DeviceInfo::AxesX_PositionMM();
        }

        if (DeviceInfo::AxesX_PositionMM() > grateXmax) {
            grateXmax = DeviceInfo::AxesX_PositionMM();
        }

        if (DeviceInfo::AxesY_PositionMM() < grateYmin) {
            grateYmin = DeviceInfo::AxesY_PositionMM();
        }

        if (DeviceInfo::AxesY_PositionMM() > grateYmax) {
            grateYmax = DeviceInfo::AxesY_PositionMM();
        }
    }

    //     qDebug() << "onCncNewData";
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

    if (cncConnected) {
        //         actionConnectDisconnect->setIcon( QIcon(":/images/connect.png"));
        //         actionConnectDisconnect->setText( translate(_DISCONNECT_FROM_DEV) );
        QPalette palette = statusSt->palette();
        //  palette.setColor(statusSt->backgroundRole(), Qt::yellow);
        palette.setColor(statusSt->foregroundRole(), Qt::green);
        statusSt->setPalette(palette);
        //         statusSt->setPalette(Qt::green);
    } else {
        //         actionConnectDisconnect->setIcon( QIcon(":/images/disconnect.png"));
        //         actionConnectDisconnect->setText( translate(_CONNECT_TO_DEV));
        QPalette palette = statusSt->palette();
        //  palette.setColor(statusSt->backgroundRole(), Qt::yellow);
        palette.setColor(statusSt->foregroundRole(), Qt::red);
        statusSt->setPalette(palette);
    }

    groupPosition->setEnabled( cncConnected);
    groupControl->setEnabled( cncConnected);
    // set groupVelocity too?

    actionStop->setEnabled( cncConnected);
    actionSpindle->setEnabled( cncConnected);

    labelSpeed->setText( QString::number(cnc->spindelMoveSpeed()) + translate(_MM_MIN));
    //     statLabelNumInstr->setText( translate(_NUM_INSTR) + QString::number(cnc->numberComleatedInstructions()));

    if (!cncConnected) {
        QPixmap grayPix = QPixmap(":/images/ball_gray.png");

        maxXLED->setPixmap( grayPix);
        minXLED->setPixmap( grayPix);
        maxYLED->setPixmap( grayPix);
        minYLED->setPixmap( grayPix);
        maxZLED->setPixmap( grayPix);
        minZLED->setPixmap( grayPix);
        maxALED->setPixmap( grayPix);
        minALED->setPixmap( grayPix);

        toolRun->setEnabled( cncConnected);
        toolPause->setEnabled( cncConnected);
        toolStop->setEnabled( cncConnected);

        return;
    }

    switch (Task::StatusTask) {
        case TaskStart: {
            statusLabel1->setText( translate(_START_TASK));
            break;
        }

        case TaskPaused: {
            statusLabel1->setText( translate(_PAUSE_TASK));
            break;
        }

        case TaskStop: {
            statusLabel1->setText( translate(_STOP_TASK));
            break;
        }

        case TaskWorking: {
            statusLabel1->setText( translate(_RUN_TASK));
            break;
        }

        case Waiting: {
            statusLabel1->setText( translate(_WAIT));
            break;
        }
    }

    numPosX->setValue( DeviceInfo::AxesX_PositionMM());
    numPosY->setValue( DeviceInfo::AxesY_PositionMM());
    numPosZ->setValue( DeviceInfo::AxesZ_PositionMM());

#if 0

    if (cnc->isEstopOn()) {
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

    maxXLED->setPixmap( DeviceInfo::AxesX_LimitMax ? redPix : greenPix);
    minXLED->setPixmap( DeviceInfo::AxesX_LimitMin ? redPix : greenPix);
    maxYLED->setPixmap( DeviceInfo::AxesY_LimitMax ? redPix : greenPix);
    minYLED->setPixmap( DeviceInfo::AxesY_LimitMin ? redPix : greenPix);
    maxZLED->setPixmap( DeviceInfo::AxesZ_LimitMax ? redPix : greenPix);
    minZLED->setPixmap( DeviceInfo::AxesZ_LimitMin ? redPix : greenPix);
    maxALED->setPixmap( DeviceInfo::AxesA_LimitMax ? redPix : greenPix);
    minALED->setPixmap( DeviceInfo::AxesA_LimitMin ? redPix : greenPix);

    //***************

    //DEBUG:
    byte bb14 = cnc->getByte(14);
    checkB14B0->setChecked( bb14 & (1 << 0));
    checkB14B1->setChecked( bb14 & (1 << 1));
    checkB14B2->setChecked( bb14 & (1 << 2));
    checkB14B3->setChecked( bb14 & (1 << 3));
    checkB14B4->setChecked( bb14 & (1 << 4));
    checkB14B5->setChecked( bb14 & (1 << 5));
    checkB14B6->setChecked( bb14 & (1 << 6));
    checkB14B7->setChecked( bb14 & (1 << 7));


    byte bb15 = cnc->getByte(15);
    checkB15B0->setChecked( bb15 & (1 << 0));
    checkB15B1->setChecked( bb15 & (1 << 1));
    checkB15B2->setChecked( bb15 & (1 << 2));
    checkB15B3->setChecked( bb15 & (1 << 3));
    checkB15B4->setChecked( bb15 & (1 << 4));
    checkB15B5->setChecked( bb15 & (1 << 5));
    checkB15B6->setChecked( bb15 & (1 << 6));
    checkB15B7->setChecked( bb15 & (1 << 7));


    byte bb19 = cnc->getByte(19);
    checkB19B0->setChecked( bb19 & (1 << 0));
    checkB19B1->setChecked( bb19 & (1 << 1));
    checkB19B2->setChecked( bb19 & (1 << 2));
    checkB19B3->setChecked( bb19 & (1 << 3));
    checkB19B4->setChecked( bb19 & (1 << 4));
    checkB19B5->setChecked( bb19 & (1 << 5));
    checkB19B6->setChecked( bb19 & (1 << 6));
    checkB19B7->setChecked( bb19 & (1 << 7));

    // end debug

    // Кнопки запуска остановки заданий
    groupBoxExec->setEnabled( cncConnected);

    if (cncConnected) {
        if (mainTaskTimer.isActive()) {
            toolRun->setEnabled( false );

            if (Task::StatusTask == TaskPaused) {
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

        if (Task::StatusTask == Waiting) {
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

        if (Task::StatusTask == TaskWorking) {
            statusProgress->setValue( cnc->numberComleatedInstructions());
            //listGkodeForUser.Rows[cnc->NumberComleatedInstructions].Selected = true;
            //TODO: переделать алгоритм, иначе это изменение сбивает выделенный диапазон
            //listGCodeWidget->currentIndex() = cnc->NumberComleatedInstructions;
        }
    } else {
        toolRun->setEnabled( cncConnected);
        toolPause->setEnabled( cncConnected);
        toolStop->setEnabled( cncConnected);
    }
}


void MainWindow::onManualControlDialog()
{
    ManualControlDialog *mc = new ManualControlDialog(this);
    mc->exec();
    delete mc;
}


//
// Перезаполнение данных
//
// params: listCode"
void MainWindow::fillListWidget(QStringList listCode)
{
    listGCodeWidget->clear();
    listGCodeWidget->setRowCount( 0);
    listGCodeWidget->setColumnCount(3);
    QStringList header = (QStringList() << translate(_COMMAND) << translate(_INFO) << translate(_STATE));

    listGCodeWidget->setHorizontalHeaderLabels(header);

    int maxIndexLen = QString::number(listCode.count()).length(); //вычисление количества символов используемых для нумерации записей

    foreach (QString valueStr, listCode) {
        listGCodeWidget->insertRow( listGCodeWidget->rowCount() );

        QTableWidgetItem *newItem = new QTableWidgetItem(valueStr);
        newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable); // set read only

        listGCodeWidget->setItem(listGCodeWidget->rowCount() - 1, 0, newItem);

        for (int j = 1; j < 3; j++) { // set other elements read only
            QTableWidgetItem *it = new QTableWidgetItem();
            it->setFlags(it->flags() ^ Qt::ItemIsEditable);
            listGCodeWidget->setItem(listGCodeWidget->rowCount() - 1, j, it);
        }
    }

    listGCodeWidget->resizeColumnsToContents();

    tabWidget->setCurrentIndex(0);

    statusProgress->setRange(1, listGCodeWidget->rowCount() - 1);

    scene3d->matrixReloaded();
}


void MainWindow::onSaveFile()
{
    SaveFile();
}


void MainWindow::onOpenFile()
{
    OpenFile();

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


void MainWindow::onSettings()
{
    //   qDebug() << "onSetting";
    SettingsDialog *setfrm = new SettingsDialog(this);
    int dlgResult = setfrm->exec();

    if (dlgResult == QMessageBox::Ok) {
        cnc->saveSettings();
    }
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


void MainWindow::onSpindel()
{
    if (cnc->isSpindelOn()) {
        cnc->spindelOFF();
    } else {
        cnc->spindelON();
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
    //вызов 3Д формы
    //покажем графические настройки
    Settings3dDialog *dlg = new Settings3dDialog(this);
    dlg->exec();

    scene3d->updateGL();

    delete dlg;
}


void MainWindow::onScanSurface()
{
    //вызов формы сканирования
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
    cnc->deviceNewPosition(0, DeviceInfo::AxesY_PositionPulse, DeviceInfo::AxesZ_PositionPulse);
    numPosX->setValue(0.0);
}


void MainWindow::onButtonYtoZero()
{
    cnc->deviceNewPosition(DeviceInfo::AxesX_PositionPulse, 0, DeviceInfo::AxesZ_PositionPulse);
    numPosY->setValue(0.0);
}


void MainWindow::onButtonZtoZero()
{
    cnc->deviceNewPosition(DeviceInfo::AxesX_PositionPulse, DeviceInfo::AxesY_PositionPulse, 0);
    numPosZ->setValue(0.0);
}


void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
