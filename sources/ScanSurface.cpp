/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2019 by Eduard Kalinowski                             *
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


#include "ScanSurface.h"


/******************************************************************************
** ScanSurfaceDialog
*/
ScanSurfaceDialog::ScanSurfaceDialog(QWidget *p)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    cnc = parent->mk1;

    setStyleSheet(parent->programStyleSheet);

    selectedX = -1;
    selectedY = -1;

    Scan = false;

    //     QString n = QString::number(1.01);
    //     toDecimalPoint = (n.indexOf(",") > 0) ? ',' : '.';
    //     fromDecimalPoint = (toDecimalPoint == ',') ? '.' : ',';

    indexScanX = 0;
    indexScanY = 0;
    indexMaxScanX = 0;
    indexMaxScanY = 0;

    scanThread = NULL;

    numSpeed->setValue(200);
    numReturn->setValue(400);

    //TODO: loading data from the matrix
    surfaceArr = parent->surfaceMatrix;

    sizeY = surfaceArr.count();
    sizeX = 0;

    if (sizeY > 0) {
        sizeX = surfaceArr[0].count();
    }

    // ruled is the old
    // hermite and cubic are new
    QStringList ls = (QStringList() << "Ruled" << "Hermite-Spline" << "Quadric");
    comboZ->addItems(ls);

    comboGrid->addItems((QStringList() << "10" << "5" << "3" << "2" << "1"));

    translateDialog();

    startOffsetX->setValue(Settings::coord[X].posMm());
    startOffsetY->setValue(Settings::coord[Y].posMm());
    startOffsetZ->setValue(Settings::coord[Z].posMm());


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
    groupStartPoint->setTitle(translate(ID_BEG_SURFACE));
    groupSteps->setTitle(translate(ID_STEP_PROP));
    groupTest->setTitle(translate(ID_TEST_SCAN));
    groupPoint->setTitle(translate(ID_TABLE_POINT));
    labelStepX->setText(translate(ID_STEP_X));
    labelStepY->setText(translate(ID_STEP_Y));
    checkBoxViewOnly->setText(translate(ID_VIEW_ONLY));
    labelVelo->setText(translate(ID_VELO));
    labelRet->setText(translate(ID_RET_MM));
    groupBoxZ->setTitle(translate(ID_ALGORITHM_Z));
    labelGrid->setText(translate(ID_GRID));
    pushButtonTest->setText(translate(ID_TEST_SCAN));

    pushButtonScan->setText(translate(ID_SCAN));
}


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

    float offsetX = startOffsetX->value();
    float offsetY = startOffsetY->value();
    //     float offsetZ = startOffsetZ->value();

    // delta
    float stepX = deltaStepX->value();
    float stepY = deltaStepY->value();

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
            // init of coordinates: x, y, z, a
            surfaceArr[y][x] = (coord) {
                0.0, 0.0, 0.0, 0.0
            };
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
    float offsetX = startOffsetX->value();
    float offsetY = startOffsetY->value();
    float offsetZ = startOffsetZ->value();

    float stepX = deltaStepX->value();
    float stepY = deltaStepY->value();

    bool edit = !(checkBoxViewOnly->isChecked());

    for (int y = 0; y < sizeY; y++) {
        for (int x = 0; x < sizeX; x++) {
            surfaceArr[y][x].X = offsetX + (x * stepX);
            surfaceArr[y][x].Y = offsetX + (y * stepY);
            //             surfaceArr[y][x].Z = offsetZ;

            dataGridView->item(y, x)->setText(QString().sprintf("Z %4.2f", (surfaceArr[y][x].Z + offsetZ)));

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
    float val = ss.replace(Settings::fromDecimalPoint, Settings::toDecimalPoint).toDouble(&res);

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
        return;    //if already runs...
    }

    if (Scan) {
        return;
    }

    if (cnc->isConnected() == false) {
        return;
    }

    cnc->packC0(0x01);  // on

    cnc->packD2((int)numSpeed->value(), (float)numReturn->value());      // settings of movement and velocity

    cnc->packC0(0x00); // off
}


void ScanSurfaceDialog::onScan()
{
    //TODO: to add the selective scanning
    if (Scan) {
        Scan = false;
        pushButtonScan->setText(translate(ID_SCAN));

        disconnect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
        emit scanThread->terminate();

        delete scanThread;

        scanThread = NULL;
    } else {
        if (scanThread == NULL) {
            return;
        }

        if (scanThread->isRunning()) {
            return;    // if already runs
        }

        if (cnc->isConnected() == false) {
            return;
        }

        pushButtonScan->setText(translate(ID_STOP_SCAN));

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
    cnc = parent->mk1;

    setTerminationEnabled(true);
}

// scan thread
// TODO: при сканировании иногда заполняет сразу 2 ячейки в таблице?!?
void ScanThread::run()
{
    if (cnc->getSpindleMoveSpeed() != 0) {
        return;
    }

    // coordinates for moving
    //     {
    //float px = dataCode.Matrix[indexScanY].X[indexScanX].X;
    float px = sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].X;
    //float pz = dataCode.Matrix[indexScanY].X[indexScanX].Z;
    float pz = sParent->startOffsetZ->value();
    //float py = dataCode.Matrix[indexScanY].Y;
    float py = sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].Y;
    //     float pa = 0;//sParent->numPosA->value();

    // move to point
    moveParameters mParams;
    mParams.pos.X = px;
    mParams.pos.Y = py;
    mParams.pos.Z = pz;
    mParams.pos.A = 0;//, userSpeedG0;
    mParams.speed = (int)sParent->numSpeed->value();
    mParams.movingCode = RAPID_LINE_CODE; //gcodeNow.accelCode;
    mParams.restPulses = 0;//gcodeNow.stepsCounter;
    mParams.numberInstruction = 0;

    //         cnc->packCA(Settings::coord[X].posPulse( px), Settings::coord[Y].posPulse(py), Settings::coord[Z].posPulse( pz),  Settings::coord[A].posPulse( pa), (int)sParent->numSpeed->value(), 0, 0, 0.0);
    cnc->packCA(&mParams);
    //     }

    usleep(100);

    // опустим щуп
    cnc->packC0(0x01); // on

    cnc->packD2((int)sParent->numSpeed->value(), 0);// settings of movement and velocity

    cnc->packC0(0x00); // off

    usleep(100);

    while (!Settings::coord[Z].actualLimitMax) {
        //dataCode.Matrix[indexScanY].X[indexScanX].Z = cnc->PositionZmm() - numReturn->value();
        usleep(100);
    }

    usleep(300);
    //dataCode.Matrix[indexScanY].X[indexScanX].Z = cnc->PositionZmm;
    sParent->surfaceArr[sParent->indexScanY][sParent->indexScanX].Z = (float)Settings::coord[Z].posMm();

    cnc->packC0(0x01); // on

    cnc->packD2((int)sParent->numSpeed->value(), (float)sParent->numReturn->value()); // settings of movement and velocity

    cnc->packC0(0x00); // off

    usleep(100);
    // move to the point
    cnc->packCA(&mParams);
    //     cnc->packCA(Settings::coord[X].posPulse( px), Settings::coord[Y].posPulse(py), Settings::coord[Z].posPulse( pz),  Settings::coord[A].posPulse(pa), (int)sParent->numSpeed->value(), 0, 0, 0.0);

    usleep(100);

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


void ScanSurfaceDialog::onTimer1()
{
    if ( selectedX == -1 && selectedY == -1) {
        label10->setText(QString().sprintf("X: %4.2f Y: %4.2f", 0.0, 0.0));
        return;
    }

    label10->setText(QString().sprintf("X: %4.2f Y: %4.2f", surfaceArr[selectedY][selectedX].X, surfaceArr[selectedY][selectedX].Y));
}


// move to the point
void ScanSurfaceDialog::buttonMove()
{
    if (!cnc->testAllowActions()) {
        return;
    }

    parent->scanPosX = selectedX;
    parent->scanPosY = selectedY;

    if (selectedX == -1 && selectedY == -1) {
        return;
    }

    parent->surfaceMatrix = surfaceArr;

    parent->moveToPoint(true);
}


// set z
void ScanSurfaceDialog::buttonSetZ()
{
    // узнаем координаты из таблицы, куда все поместить
    if (selectedX == -1 || selectedY == -1) {
        return;
    }

    surfaceArr[selectedY][selectedX].Z = Settings::coord[Z].posMm();
    dataGridView->item(selectedY, selectedX)->setText( QString().sprintf("Z %4.2f", surfaceArr[selectedY][selectedX].Z));

}


// z+
void ScanSurfaceDialog::buttonPlusZDown()
{
    //     button6.BackColor = Color.DarkGreen;
    // TODO get settings from gui
    int pulses = 100;

    cnc->startManualMove("0", "0", "+", "0", 100, pulses);
}


void ScanSurfaceDialog::buttonStop()
{
    //     button6.BackColor = Color.FromName("Control");
    cnc->stopManualMove();
}

// z-
void ScanSurfaceDialog::buttonMinusZDown()
{
    //     button5.BackColor = Color.DarkGreen;
    // TODO get settings from gui
    int pulses = 100;

    cnc->startManualMove("0", "0", "-", "0", 100, pulses);
}

