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
// #include <QUrl>

#include "includes/mk1Controller.h"
#include "includes/ScanSurface.h"


/******************************************************************************
** ScanSurfaceDialog
*/

// class DeviceInfo;

ScanSurfaceDialog::ScanSurfaceDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    //         _cnc = (parent->cnc);

    //     _surfaceMatrix = &(this->parent->surfaceMatrix);
    //     _deviceInfo = parent->cnc->devInfo;

    setStyleSheet(parent->programStyleSheet);

    selectedX = -1;
    selectedY = -1;

    Scan = false;

    indexScanX = 0;
    indexScanY = 0;
    indexMaxScanX = 0;
    indexMaxScanY = 0;

    scanThread = NULL;


    surfaceArr = parent->surfaceMatrix;

    sizeY = surfaceArr.count();
    sizeX = 0;

    if (sizeY > 0) {
        sizeX = surfaceArr[0].count();
    }

    translateDialog();

    //TODO: загрузить данные из существующей матрицы
    startOffsetX->setValue(DeviceInfo::AxesX_PositionMM());
    startOffsetY->setValue(DeviceInfo::AxesY_PositionMM());
    startOffsetZ->setValue(DeviceInfo::AxesZ_PositionMM());


    connect(pushOk, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(pushCancel, SIGNAL(clicked()), this, SLOT(reject()));

    connect(pushButtonScan, SIGNAL(clicked()), this, SLOT(onScan()));
    connect(pushButtonTest, SIGNAL(clicked()), this, SLOT(onTestScan()));

    connect(checkBoxViewOnly,  SIGNAL(stateChanged ( int)), this, SLOT(checkBoxChanged(int)));

    // start offset
    connect(startOffsetX, SIGNAL(valueChanged(QString)), this, SLOT(writeDataGridHeader()));
    connect(startOffsetY, SIGNAL(valueChanged(QString)), this, SLOT(writeDataGridHeader()));

    connect(startOffsetZ, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged()));

    connect(deltaStepX, SIGNAL(valueChanged(QString)), this, SLOT(writeDataGridHeader()));
    connect(numCountX, SIGNAL(valueChanged(QString)), this, SLOT(resizeDataGrid()));
    connect(deltaStepY, SIGNAL(valueChanged(QString)), this, SLOT(writeDataGridHeader()));
    connect(numCountY, SIGNAL(valueChanged(QString)), this, SLOT(resizeDataGrid()));

    connect(numSpeed, SIGNAL(valueChanged(int)), this, SLOT(valueSpeedChanged(int)));
    connect(numReturn, SIGNAL(valueChanged(int)), this, SLOT(valueReturnChanged(int)));

    connect(dataGridView, SIGNAL(itemChanged ( QTableWidgetItem * )), this, SLOT(itemChanged(QTableWidgetItem*)));
    connect(dataGridView, SIGNAL(itemClicked ( QTableWidgetItem * )), this, SLOT(itemClicked(QTableWidgetItem*)));
    connect(dataGridView, SIGNAL(cellActivated ( int, int )), this, SLOT(cellActivated(int, int)));

    connect(toolButtonZUp, SIGNAL(pressed()), this, SLOT(buttonPlusZDown()));
    connect(toolButtonZUp, SIGNAL(released()), this, SLOT(buttonStop()));
    connect(toolButtonZSet, SIGNAL(clicked()), this, SLOT(buttonSetZ()));
    //     connect(toolButtonZSet, SIGNAL(released()), this, SLOT(valueChanged()));
    connect(toolButtonZDown, SIGNAL(pressed()), this, SLOT(buttonMinusZDown()));
    connect(toolButtonZDown, SIGNAL(released()), this, SLOT(buttonStop()));
    connect(toolButtonMove, SIGNAL(clicked(bool)), this, SLOT(valueChanged()));
    //     connect(toolButtonMove, SIGNAL(released()), this, SLOT(valueChanged()));

    label10->setText("");
    
    adjustSize();
}


void ScanSurfaceDialog::translateDialog()
{
    groupStartPoint->setTitle(translate(_BEG_SURFACE));
    groupSteps->setTitle(translate(_STEP_PROP));
    groupTest->setTitle(translate(_TEST_SCAN));
    groupPoint->setTitle(translate(_TABLE_POINT));
    labelStepX->setText(translate(_STEP_X));
    labelStepY->setText(translate(_STEP_Y));
    checkBoxViewOnly->setText(translate(_VIEW_ONLY));
    labelVelo->setText(translate(_VELOSCAN));
    labelRet->setText(translate(_RET_MM));

    pushButtonTest->setText(translate(_TEST_SCAN));

    pushButtonScan->setText(translate(_SCAN));
}


// void ScanSurfaceDialog::loadMatrix()
// {
//     //TODO: загрузить данные из существующей матрицы
//
// //     startOffsetX->setValue(DeviceInfo::AxesX_PositionMM());
// //     startOffsetY->setValue(DeviceInfo::AxesY_PositionMM());
// //     startOffsetZ->setValue(DeviceInfo::AxesZ_PositionMM());
//
//     refreshDataGrid();
// }


void ScanSurfaceDialog::valueSpeedChanged(int n)
{
}


void ScanSurfaceDialog::valueReturnChanged(int n)
{
}


void ScanSurfaceDialog::onSave()
{
    parent->surfaceMatrix = surfaceArr;

    accept();
}


// refresh the header text information
void ScanSurfaceDialog::writeDataGridHeader()
{
    // start offset
    if (sizeX == 0 || sizeY == 0) {
        return;
    }

    double offsetX = startOffsetX->value();
    double offsetY = startOffsetY->value();
    double offsetZ = startOffsetZ->value();

    // delta
    double stepX = deltaStepX->value();
    double stepY = deltaStepY->value();

    QStringList xLabels, yLabels;

    for (int x = 0; x < sizeX; x++) {
        xLabels << QString().sprintf("X %4.2f", offsetX + (x * stepX));
    }

    dataGridView->setHorizontalHeaderLabels(xLabels);

    for (int y = 0; y < sizeY; y++) {
        yLabels << QString().sprintf("Y %4.2f", offsetY + (y * stepY));
    }

    dataGridView->setVerticalHeaderLabels(yLabels);
}


void ScanSurfaceDialog::resizeDataGrid()
{
    int countX = numCountX->value();
    int countY = numCountY->value();

    sizeX = countX;
    sizeY = countY;

    surfaceArr.resize(countY);

    for (int y = 0; y < countY; y++) {
        surfaceArr[y].resize(countX);
    }

    dataGridView->clear();

    dataGridView->setColumnCount(countX);
    dataGridView->setRowCount(countY);

    writeDataGridHeader();

    for (int y = 0; y < sizeY; y++) {
        for (int x = 0; x < sizeX; x++) {
            dataGridView->setItem(y, x, new QTableWidgetItem(""));
        }
    }

    refreshDataGrid();
}


void ScanSurfaceDialog::valueChanged()
{
    refreshDataGrid();
}


void ScanSurfaceDialog::refreshDataGrid()
{
    //наполним массив
    double offsetX = startOffsetX->value();
    double offsetY = startOffsetY->value();
    double offsetZ = startOffsetZ->value();

    double stepX = deltaStepX->value();
    double stepY = deltaStepY->value();

    bool edit = !(checkBoxViewOnly->isChecked());

    for (int y = 0; y < sizeY; y++) {
        for (int x = 0; x < sizeX; x++) {
            surfaceArr[y][x].X = offsetX + (x * stepX);
            surfaceArr[y][x].Y = offsetX + (y * stepY);
            surfaceArr[y][x].Z = offsetZ;

            dataGridView->item(y, x)->setText(QString().sprintf("Z %4.2f", surfaceArr[y][x].Z));

            if (edit == true) {
                dataGridView->item(y, x)->setFlags(Qt::ItemIsEditable);
            } else {
                QTableWidgetItem *item = dataGridView->item(y, x);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            }
        }
    }
}


void ScanSurfaceDialog::checkBoxChanged(int st)
{
    int countX = numCountX->value();
    int countY = numCountY->value();

    bool edit = !(checkBoxViewOnly->isChecked());

    for (int y = 0; y < countY; y++) {
        for (int x = 0; x < countX; x++) {
            if (edit == true) {
                dataGridView->item(y, x)->setFlags(Qt::ItemIsEditable);
            } else {
                QTableWidgetItem *item = dataGridView->item(y, x);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            }
        }
    }
}


void ScanSurfaceDialog::cellActivated ( int y, int x )
{
    if (x < 0 || y < 0) {
        selectedX = -1;
        selectedY = -1;
    } else {
        selectedX = x;
        selectedY = y;
    }
}


void ScanSurfaceDialog::itemClicked( QTableWidgetItem * item)
{
    int x = item->column();
    int y = item->row();

    if (x < 0 || y < 0) {
        selectedX = -1;
        selectedY = -1;
    } else {
        selectedX = x;
        selectedY = y;
    }
}


void ScanSurfaceDialog::itemChanged( QTableWidgetItem * item)
{
    int x = item->column();
    int y = item->row();

    if (x == 0 || y == 0) {
        return;
    }

    QString ss = dataGridView->item(y, x)->text();
    bool res;
    double val = ss.replace(".", ",").toDouble(&res);

    if (res == true) {
        surfaceArr[y][x].Z = val;
    }
}


void ScanSurfaceDialog::onTestScan()
{
    if (scanThread != NULL) {
        return;
    }

    if (scanThread->isRunning()) {
        return;    //небудем вклиниваться если что....
    }

    if (Scan) {
        return;
    }

    parent->cnc->packC0(0x01);  //вкл

    parent->cnc->packD2((int)numSpeed->value(), (double)numReturn->value());      // + настройка отхода, и скорости

    parent->cnc->packC0(0x00); //выкл
}


void ScanSurfaceDialog::onScan()
{
    //TODO: Добавить возможность выборочного сканирования
    if (Scan) {
        Scan = false;
        pushButtonScan->setText(translate(_SCAN));

        disconnect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
        emit scanThread->terminate();

        delete scanThread;

        scanThread = NULL;
    } else {
        if (scanThread == NULL) {
            return;
        }

        if (scanThread->isRunning()) {
            return;    //пока ещё работает поток
        }

        pushButtonScan->setText(translate(_STOP_SCAN));

        Scan = true;
        indexMaxScanX = (int)numCountX->value();
        indexMaxScanY = (int)numCountY->value();

        scanThread = new ScanThread(this);

        connect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
        scanThread->start();
    }

    groupStartPoint->setEnabled(!Scan);
    groupSteps->setEnabled(!Scan);
    groupPoint->setEnabled(!Scan);
}


void ScanSurfaceDialog::onTimer()
{
    if (dataGridView->item(indexScanY, indexScanX) != NULL) {
        dataGridView->item(indexScanY, indexScanX)->setText( QString().sprintf("Z %4.2f", surfaceArr[indexScanY][indexScanX].Z));
    }
}


ScanThread::ScanThread(QObject* p)
{
    sParent = static_cast<ScanSurfaceDialog*>(p);
    parent = sParent->parent;
    cnc = parent->cnc;

    setTerminationEnabled(true);
}

// Поток выполняющий сканирование
// TODO: при сканировании иногда заполняет сразу 2 ячейки в таблице?!?
void ScanThread::run()
{
    //     mk1Controller *_cnc = sParent->parent->cnc;// = static_cast<MainWindow*>(parent);

    if (cnc->spindelMoveSpeed() != 0) {
        return;
    }

    //координаты куда передвинуться
    //double px = dataCode.Matrix[indexScanY].X[indexScanX].X;
    double px = sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].X;
    //double pz = dataCode.Matrix[indexScanY].X[indexScanX].Z;
    double pz = sParent->startOffsetZ->value();
    //double py = dataCode.Matrix[indexScanY].Y;
    double py = sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].Y;
    double pa = 0;//sParent->numPosA->value();

    //спозиционируемся
    cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz),  DeviceInfo::CalcPosPulse("A", pa), (int)sParent->numSpeed->value(), 0);

    sleep(100);

    //опустим щуп
    cnc->packC0(0x01);//вкл

    cnc->packD2((int)sParent->numSpeed->value(), 0);// + настройка отхода, и скорости

    cnc->packC0(0x00); //выкл

    sleep(100);

    while (!DeviceInfo::AxesZ_LimitMax) {
        //dataCode.Matrix[indexScanY].X[indexScanX].Z = DeviceInfo::AxesZ_PositionMM() - numReturn->value();
        sleep(100);
    }


    sleep(300);
    //dataCode.Matrix[indexScanY].X[indexScanX].Z = DeviceInfo::AxesZ_PositionMM;
    sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].Z = (double)DeviceInfo::AxesZ_PositionMM();

    cnc->packC0(0x01); //вкл

    cnc->packD2((int)sParent->numSpeed->value(), (double)sParent->numReturn->value()); // + настройка отхода, и скорости

    cnc->packC0(0x00);//выкл

    sleep(100);
    //спозиционируемся
    cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz),  DeviceInfo::CalcPosPulse("A", pa), (int)sParent->numSpeed->value(), 0);

    sleep(100);

    if (sParent->indexScanX == sParent->indexMaxScanX && sParent->indexScanY == sParent->indexMaxScanY) {
        sParent->Scan = false;
        cnc->packFF();
    }

    if (sParent->indexScanX < sParent->indexMaxScanX) {
        sParent->indexScanX++;
    } else {
        sParent->indexScanX = 0;

        if (sParent->indexScanY < sParent->indexMaxScanY) {
            sParent->indexScanY++;
        } else {
            sParent->indexScanY = 0;
        }
    }
}

#if 0
void ScanSurfaceDialog::scanThreadDoWork()
{
    Scan = true;

    // connect(scanThread, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));

    //      scanThread->start();

    ScanSurfaceDialog::theads();
    //     }
}
#endif


void ScanSurfaceDialog::onTimer1()
{
    if ( selectedX == -1 && selectedY == -1) {
        label10->setText("X: 000.000  Y: 000.000");
        return;
    }

    label10->setText("X: " + QString::number(surfaceArr[selectedY][selectedX].X) + "  Y: " + QString::number(surfaceArr[selectedY][selectedX].X));
}

// move to the point
void ScanSurfaceDialog::buttonMove()
{
    if (!parent->cnc->testAllowActions()) {
        return;
    }

    parent->scanPosX = selectedX;
    parent->scanPosY = selectedY;

    if (selectedX == -1 && selectedY == -1) {
        return;
    }

    parent->surfaceMatrix = surfaceArr;

    parent->moveToPoint(true);

#if 0
    int speed = 200;

    parent->cnc->pack9E(0x05);

    parent->cnc->packBF(speed, speed, speed, speed);

    parent->cnc->packC0();

    int pulseX = DeviceInfo::CalcPosPulse("X", surfaceArr[selectedY][selectedX].X);
    int pulseY = DeviceInfo::CalcPosPulse("Y", surfaceArr[selectedY][selectedX].Y);
    int pulseZ = DeviceInfo::CalcPosPulse("Z", surfaceArr[selectedY][selectedX].Z);
    int pulseA = DeviceInfo::CalcPosPulse("A", surfaceArr[selectedY][selectedX].A);

    parent->cnc->packCA(pulseX, pulseY, pulseZ, pulseA, speed, 0);

    parent->cnc->packFF();

    parent->cnc->pack9D();

    parent->cnc->pack9E(0x02);

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();
#endif
}

// set z
void ScanSurfaceDialog::buttonSetZ()
{
    //узнаем координаты из таблицы, куда все поместить

    if (selectedX == -1 || selectedY == -1) {
        return;
    }

    surfaceArr[selectedY][selectedX].Z = DeviceInfo::AxesZ_PositionMM();
    dataGridView->item(selectedY, selectedX)->setText( QString().sprintf("Z %4.2f", surfaceArr[selectedY][selectedX].Z));

}


// z+
void ScanSurfaceDialog::buttonPlusZDown()
{
    //     button6.BackColor = Color.DarkGreen;
    parent->cnc->startManualMove("0", "0", "+", "0", 100);
}


void ScanSurfaceDialog::buttonStop()
{
    //     button6.BackColor = Color.FromName("Control");
    parent->cnc->stopManualMove();
}

// z-
void ScanSurfaceDialog::buttonMinusZDown()
{
    //     button5.BackColor = Color.DarkGreen;
    parent->cnc->startManualMove("0", "0", "-", "0", 100);
}

