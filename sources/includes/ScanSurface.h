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


#ifndef SCANSURFACE_H
#define SCANSURFACE_H

#include "MainWindow.h"

#include "ui_ScanSurface.h"

class ScanThread;

class ScanSurfaceDialog : public QDialog, public Ui::ScanSurfaceDialog,  public cTranslator
{
        Q_OBJECT
    public:
        ScanSurfaceDialog(QWidget *p = 0);
        //         void threads();

    private slots:
        void onScan();
        void onTimer();
        void onTimer1();
        void onTestScan();
        void checkBoxChanged(int st ); // view only
        void valueChanged(); // from SpinBoxes only
        void cellActivated ( int row, int column );
        void itemChanged ( QTableWidgetItem * item );
        void itemClicked ( QTableWidgetItem * item );
        void buttonMove();
        void buttonSetZ();
        void buttonPlusZDown();
        void buttonStop();
        void buttonMinusZDown();
        //         void buttonMinusZUp();

    private:
        void translateDialog();
        void refrechDataGrid();
        void feeler_Load();
        //  void scanThreadDoWork();

    private:

        //         DeviceInfo _deviceInfo;

        QTimer taskTimer;
        ScanThread *scanThread;


    public:
        MainWindow* parent;

        int selectedX;
        int selectedY;

        int indexScanX;
        int indexScanY;
        int indexMaxScanX;
        int indexMaxScanY;
        bool Scan;
};


class ScanThread : public QThread
{
        Q_OBJECT
    public:
        ScanThread(QObject* p);
        void run();
    private:
        ScanSurfaceDialog* sParent;
        MainWindow *parent;
        mk1Controller *cnc;
        //  signals:
        //      void resultReady(const QString &s);
};





#endif // SCANSURFACE_H
