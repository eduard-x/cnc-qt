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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
// #include <QTranslator>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QThread>
#include <QProgressBar>
#include <QScrollArea>
#include <QImage>
#include <QTimeLine>

#include <QtMath>

#include <libusb.h>

#include "version.h"

#include <QtOpenGL/QtOpenGL>

#include "GLWidget.h"

#include "usbwatcher.h"
#include "DataManager.h"
#include "mk1Controller.h"
#include "Geometry.h"
#include "Translator.h"

#include "SettingsDialog.h"
#include "About.h"
#include "EditGCode.h"
#include "ScanSurface.h"

#include "ui_MainWindow.h"

class AboutDialog;
class ParserData;
class GLWidget;
class mk1Controller;
class cTranslator;
class cDataManager;


class MessageTimerBox: public QMessageBox
{
    private:
        int timeout;
        bool autoClose;
        int currentTime;
        QString defaultText;

    public:
        void setTimeout(int t);
        void setDefaultText(const QString &t);
        void setAutoClose(bool a);
        void showEvent ( QShowEvent * event );

    private:
        void timerEvent(QTimerEvent *event);
        //     static int exec(const QString &title, const QString &text, int ticon, int timer);
};


class MessageBox: public cTranslator
{
    public:
        static int exec(void* p, const QString &title, const QString &text, int ticon);
};


//
// class for current state of controller
//
class Task
{
        //
    public:
        enum StatusTask { Waiting = 0, Start, Working, Paused, Stop };

        //         static StatusTask currentStatus;
        //
        //         static int instructionStart;
        //         static int instructionEnd;
        static int instrCounter;

        // gcode line number
        static int lineCodeStart;
        static int lineCodeNow;
        static int lineCodeEnd;
};



struct speedSettings {
    float startSpeed;
    float endSpeed;
    float acceler;
    int   stepsPerMm;
};


class MainWindow : public QMainWindow, public cDataManager, public Ui::MainWindow, public cTranslator  // , public Reader
{
        Q_OBJECT

    signals:
        void mk1Connected();
        void mk1Disconnected();

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

        float getDeltaZ(float _x, float _y);

        void moveToPoint(bool surfaceScan = false);
        Task::StatusTask getStatus();

    public:
        mk1Controller *mk1;

        QVector<QVector<coord> > surfaceMatrix; // scanned points of surface

        int scanPosX;
        int scanPosY;

        QString programStyleSheet;

        //
        // for the correction possibility
        bool Correction;
        float deltaX;
        float deltaY;
        float deltaZ;
        float coeffSizeX;
        float coeffSizeY;
        int  fixedAxes;
        bool deltaFeed;

        bool enableOpenGL;

        GLWidget *scene3d; // OpenGL widget


    public slots:
        // connect to controller
        void onCncMessage(int n_msg);
        void onCncNewData();


    private slots:
        void mk1_hotplug();
        void mk1_detach();

        void onExit();

        void onVisualize(bool b);
        void onFilter();

        void onManualControlDialog();

        void onOpenFile();
        void onSaveFile();

        void onChangeFix(bool b);

        void onDeviceInfo();
        void onDeviceReset();

        void onSettings();
        void onCalcVelocity();
        void onLogClear();

        void onCopyPos();
        void onSetHome();
        void onCopyHome();

        void onSpindel();
        void onMist();
        void onFluid();
        void onEmergyStop();
        void onScanSurface();
        void onEditGCode(int row, int col);
        void onCellSelect(int row, int col);

        void onStartTask();
        void onPauseTask();
        void onStopTask();
        //         void onCheckBoxWorkbenchLimits();
        //         void onCheckBoxWorkbenchSwap();
        void onRefreshGUITimer();

        void onCleanStatus(); // not connected
        void onRunToPoint();
        void onSendCommand();

        void onGeneratorCode(); // not connected

        bool readLangDir();

        void onAbout();
        void onAboutQt();

        void setLang(QAction* mnu);
        void setFile(QAction* fl);

    public slots:
        void onButtonXtoZero();
        void onButtonYtoZero();
        void onButtonZtoZero();
        void onButtonAtoZero();

        void getFPS(int fps);

        void logMessage(const QString &s);


    private:
        int  calculateMinAngleSteps(int pos);
        void patchSpeedAndAccelCode(int begPos, int endPos);
        void fixSerialList();

        bool OpenFile(QString &name);

        void drawWorkbench();
        //
        void reloadRecentList();

        void runNextCommand();
        void fillListWidget(QVector<QString> listCode);

        bool getLangTable();
        void setLangGUI();
        //         void displayRotation();
        void addConnections();
        void translateGUI();
        void addStatusWidgets();

        void refreshElementsForms();

        void AddLog(QString _text = "");
        void SendSignal();
        void readSettings();
        void writeSettings();
        void updateSettingsOnGUI();

        QString selectedLang;

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        USBWatcher *hotplugUSB;
        bool mk1_connected;
        //         libusb_context *context;

        Task::StatusTask currentStatus;
        QGraphicsScene *sceneCoordinates;

        QLabel *statusLabel1;
        QProgressBar *statusProgress;
        QLabel *statusLabel2;
        //         bool useHome;
        //         QTimer  renderTimer;
        QList<QAction*> actLangSelect;
        QList<QAction*> actFileSelect;
        QMenu *langMenu;
        QMenu *filesMenu;
        QActionGroup* langGroup;
        QActionGroup* filesGroup;

        QStringList langFiles;

        QTimer *refreshGUITimer;

        float maxLookaheadAngleRad;
        int scale;

};

#endif // MAINWINDOW_H
