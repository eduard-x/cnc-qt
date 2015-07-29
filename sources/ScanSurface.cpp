/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * Qt developing                                                            *
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


#include "includes/ScanSurface.h"


/******************************************************************************
** ScanSurfaceDialog
*/

class DeviceInfo;

ScanSurfaceDialog::ScanSurfaceDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    //     _cnc = (parent->cnc);

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

    translateDialog();

    connect(pushButtonScan, SIGNAL(clicked()), this, SLOT(onScan()));
    connect(pushButtonTest, SIGNAL(clicked()), this, SLOT(onTestScan()));

    connect(checkBoxViewOnly,  SIGNAL(stateChanged ( int)), this, SLOT(checkBoxChanged(int)));

    connect(numPosX, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numPosY, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numPosZ, SIGNAL(changed()), this, SLOT(valueChanged()));

    connect(numStepX, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numCountX, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numStepY, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numCountY, SIGNAL(changed()), this, SLOT(valueChanged()));

    connect(numSpeed, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(numReturn, SIGNAL(changed()), this, SLOT(valueChanged()));

    connect(dataGridView, SIGNAL(itemChanged ( QTableWidgetItem * )), this, SLOT(itemChanged(QTableWidgetItem*)));
    connect(dataGridView, SIGNAL(itemClicked ( QTableWidgetItem * )), this, SLOT(itemClicked(QTableWidgetItem*)));
    connect(dataGridView, SIGNAL(cellActivated ( int, int )), this, SLOT(cellActivated(int, int)));

    connect(toolButtonZUp, SIGNAL(pressed()), this, SLOT(buttonMinusZUp()));
    connect(toolButtonZUp, SIGNAL(released()), this, SLOT(buttonStop()));
    connect(toolButtonZSet, SIGNAL(clicked()), this, SLOT(buttonSetZ()));
    //     connect(toolButtonZSet, SIGNAL(released()), this, SLOT(valueChanged()));
    connect(toolButtonZDown, SIGNAL(pressed()), this, SLOT(buttonMinusZDown()));
    connect(toolButtonZDown, SIGNAL(released()), this, SLOT(buttonStop()));
    connect(toolButtonMove, SIGNAL(clicked(bool)), this, SLOT(valueChanged()));
    //     connect(toolButtonMove, SIGNAL(released()), this, SLOT(valueChanged()));

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


void ScanSurfaceDialog::feeler_Load()
{
    //TODO: загрузить данные из существующей матрицы

    numPosX->setValue(DeviceInfo::AxesX_PositionMM());
    numPosY->setValue(DeviceInfo::AxesY_PositionMM());
    numPosZ->setValue(DeviceInfo::AxesZ_PositionMM());

    refrechDataGrid();
}


void ScanSurfaceDialog::valueChanged()
{
    refrechDataGrid();
}


void ScanSurfaceDialog::refrechDataGrid()
{
    //наполним массив
    //dataCode.Matrix.Clear();

    int countX = numCountX->value();
    int countY = numCountY->value();
    double posX = numPosX->value();
    double posY = numPosY->value();
    double posZ = numPosZ->value();
    double stepX = numStepX->value();
    double stepY = numStepY->value();

    //     parent->surfaceMatrix  parent->surfaceMatrix = new dobPoint[countX, countY];

    for (int y = 0; y < countY; y++) {
        //matrixYline matrixline = new matrixYline();

        //matrixline.Y = numPosY->value() + (y* numStep->value());
        parent->surfaceMatrix.push_back(QVector< dPoint > ());

        for (int x = 0; x < countX; x++) {
            //             parent->surfaceMatrix[x][y] = new dobPoint(posX + (x * stepX), posY + (y * stepY), posZ);
            dPoint v = {posX + (x * stepX), posY + (y * stepY), posZ, 0.0 };
            parent->surfaceMatrix[y].push_back(v);
            //matrixline.X.Add(new matrixPoint(numPosX->value() + (x * numStep->value()), numPosZ->value(), true));
        }

        //dataCode.Matrix.Add(matrixline);
    }

    //и перезаполним таблицу
    dataGridView->clear();

    dataGridView->setColumnCount(countX + 1);
    dataGridView->setRowCount(countY + 1);

    for (int x = 0; x < countX; x++) {
        QTableWidgetItem *item = dataGridView->item(0, x + 1);
        item->setText("X " + QString::number(numPosX->value() + (x * numStepX->value())));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    }

    for (int y = 0; y < countY; y++) {

        QTableWidgetItem *item = dataGridView->item(y + 1, 0);
        item->setText("Y " + QString::number(numPosY->value() + (y * numStepY->value())));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    }

    bool edit = !(checkBoxViewOnly->isChecked());

    for (int y = 0; y < countY; y++) {
        for (int x = 0; x < countX; x++) {
            dataGridView->item(y + 1, x + 1)->setText(QString::number(parent->surfaceMatrix[y][x].Z));

            if (edit == true) {
                dataGridView->item(y + 1, x + 1)->setFlags(Qt::ItemIsEditable);
            } else {
                QTableWidgetItem *item = dataGridView->item(y + 1, x + 1);
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
                dataGridView->item(y + 1, x + 1)->setFlags(Qt::ItemIsEditable);
            } else {
                QTableWidgetItem *item = dataGridView->item(y + 1, x + 1);
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
    int x = item->column() - 1;
    int y = item->row() - 1;

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

    //     try {
    if (res == true) {
        parent->surfaceMatrix[y - 1][x - 1].Z = val;
    }

    //dataCode.Matrix[y - 1].X[x - 1].Z = val;
    //     } catch (Exception) {
    //throw;
    //     }
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
        pushButtonScan->setText(translate(_STOP_SCAN));

        if (scanThread == NULL) {
            return;
        }

        if (scanThread->isRunning()) {
            return;    //пока ещё работает поток
        }

        indexScanX = 0;
        indexScanY = 0;
        //indexMaxScanX = dataCode.Matrix[0].X.Count - 1;
        //indexMaxScanY = dataCode.Matrix.Count - 1;
        indexMaxScanX = (int)numCountX->value() - 1;
        indexMaxScanY = (int)numCountY->value() - 1;

        scanThread = new ScanThread(this);

        connect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
        scanThread->start();
    }
}


void ScanSurfaceDialog::onTimer()
{
    //     if (Scan) {
    //         button1.Text = @"Остановить";
    //     } else {
    //         button1.Text = @"Сканировать";
    //     }

    //     try {
    // dataGridView.Rows[indexScanY + 1].Cells[indexScanX + 1]->setText(dataCode.Matrix[indexScanY].X[indexScanX].Z);
    if (dataGridView->item(indexScanY + 1, indexScanX + 1) != NULL) {
        dataGridView->item(indexScanY + 1, indexScanX + 1)->setText( QString::number(parent->surfaceMatrix[indexScanY][indexScanX].Z));
    }

    //     } catch (Exception) {

    // throw;
    //     }

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
    mk1Controller *_cnc = sParent->parent->cnc;// = static_cast<MainWindow*>(parent);

    if (cnc->spindelMoveSpeed() != 0) {
        return;
    }

    //координаты куда передвинуться
    //double px = dataCode.Matrix[indexScanY].X[indexScanX].X;
    double px = parent->surfaceMatrix[sParent->indexScanY][sParent->indexScanX].X;
    //double pz = dataCode.Matrix[indexScanY].X[indexScanX].Z;
    double pz = sParent->numPosZ->value();
    //double py = dataCode.Matrix[indexScanY].Y;
    double py = parent->surfaceMatrix[sParent->indexScanY][sParent->indexScanX].Y;

    //спозиционируемся
    cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz), (int)sParent->numSpeed->value(), 0);

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
    parent->surfaceMatrix[sParent->indexScanY][sParent->indexScanX].Z = (double)DeviceInfo::AxesZ_PositionMM();

    cnc->packC0(0x01); //вкл

    cnc->packD2((int)sParent->numSpeed->value(), (double)sParent->numReturn->value()); // + настройка отхода, и скорости

    cnc->packC0(0x00);//выкл

    sleep(100);
    //спозиционируемся
    cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz), (int)sParent->numSpeed->value(), 0);

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

#if 0
// Поток выполняющий сканирование
// TODO: при сканировании иногда заполняет сразу 2 ячейки в таблице?!?
void ::threads()
{
    if (parent->cnc->ShpindelMoveSpeed != 0) {
        return;
    }

    //координаты куда передвинуться
    //double px = dataCode.Matrix[indexScanY].X[indexScanX].X;
    double px = parent->surfaceMatrix[indexScanY][indexScanX].X;
    //double pz = dataCode.Matrix[indexScanY].X[indexScanX].Z;
    double pz = numPosZ->value();
    //double py = dataCode.Matrix[indexScanY].Y;
    double py = parent->surfaceMatrix[indexScanY][indexScanX].Y;

    //спозиционируемся
    parent->cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz), (int)numSpeed->value(), 0);

    sleep(100);

    //опустим щуп
    parent->cnc->packC0(0x01); //вкл

    parent->cnc->packD2((int)numSpeed->value(), 0); // + настройка отхода, и скорости

    parent->cnc->packC0(0x00); //выкл

    Thread.Sleep(100);

    while (!DeviceInfo::AxesZ_LimitMax) {
        //dataCode.Matrix[indexScanY].X[indexScanX].Z = DeviceInfo::AxesZ_PositionMM() - numReturn->value();
        Thread.Sleep(100);
    }


    Thread.Sleep(300);
    //dataCode.Matrix[indexScanY].X[indexScanX].Z = DeviceInfo::AxesZ_PositionMM;
    parent->surfaceMatrix[indexScanY][indexScanX].Z = (double)DeviceInfo::AxesZ_PositionMM();

    parent->cnc->packC0(0x01); //вкл

    parent->cnc->packD2((int)numSpeed->value(), (double)numReturn->value()); // + настройка отхода, и скорости

    parent->cnc->packC0(0x00); //выкл


    Thread.Sleep(100);
    //спозиционируемся
    parent->cnc->packCA(DeviceInfo::CalcPosPulse("X", px), DeviceInfo::CalcPosPulse("Y", py), DeviceInfo::CalcPosPulse("Z", pz), (int)numSpeed->value(), 0);
    Thread.Sleep(100);

    if (indexScanX == indexMaxScanX && indexScanY == indexMaxScanY) {
        Scan = false;
        parent->cnc->packFF();
    }

    if (indexScanX < indexMaxScanX) {
        indexScanX++;
    } else {
        indexScanX = 0;

        if (indexScanY < indexMaxScanY) {
            indexScanY++;
        } else {
            indexScanY = 0;
        }
    }
}
#endif

void ScanSurfaceDialog::onTimer1()
{
    if ( selectedX == -1 && selectedY == -1) {
        label10->setText("X: 000.000  Y: 000.000");
        return;
    }

    label10->setText("X: " + QString::number(parent->surfaceMatrix[selectedY][selectedX].X) + "  Y: " + QString::number(parent->surfaceMatrix[selectedY][selectedX].X));
}

// move to the point
void ScanSurfaceDialog::buttonMove()
{
    if (!parent->cnc->testAllowActions()) {
        return;
    }

    if (selectedX == -1 && selectedY == -1) {
        return;
    }

    int speed = 200;

    parent->cnc->pack9E(0x05);

    parent->cnc->packBF(speed, speed, speed);

    parent->cnc->packC0();

    int pulseX = DeviceInfo::CalcPosPulse("X", parent->surfaceMatrix[selectedY][selectedX].X);
    int pulseY = DeviceInfo::CalcPosPulse("Y", parent->surfaceMatrix[selectedY][selectedX].Y);
    int pulseZ = DeviceInfo::CalcPosPulse("Z", parent->surfaceMatrix[selectedY][selectedX].Z);
    parent->cnc->packCA(pulseX, pulseY, pulseZ, speed, 0);

    parent->cnc->packFF();

    parent->cnc->pack9D();

    parent->cnc->pack9E(0x02);

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();

    parent->cnc->packFF();
}

// set z
void ScanSurfaceDialog::buttonSetZ()
{
    //узнаем координаты из таблицы, куда все поместить

    if (selectedX == -1 || selectedY == -1) {
        return;
    }

    parent->surfaceMatrix[selectedY][selectedX].Z = DeviceInfo::AxesZ_PositionMM();
    dataGridView->item(selectedY + 1, selectedX + 1)->setText( QString::number(parent->surfaceMatrix[selectedY][selectedX].Z));

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


// void ScanSurfaceDialog::butto()
// {
//     //     button5.BackColor = Color.FromName("Control");
//     parent->cnc->StopManualMove();
// }
