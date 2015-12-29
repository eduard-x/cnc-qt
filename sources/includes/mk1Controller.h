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


#ifndef MK1CONTROLLER_H
#define MK1CONTROLLER_H

#define BUFFER_SIZE 64

#define byte unsigned char

#include <QSettings>
#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QThread>
#include <QIODevice>

#include "MainWindow.h"
#include "Translator.h"

#include <libusb-1.0/libusb.h>

#define BULK_ENDPOINT_OUT     0x81
#define BULK_ENDPOINT_IN      0x01


class mk1Controller;


class usbHotplugThread : public QThread
{
        Q_OBJECT
        void run() {
            /* expensive or blocking operation  */
            while(true) {
                int r = libusb_handle_events(NULL);
                msleep(150);
                //                 qDebug() << "libusb_handle_events" << r;

                emit hotplugEvent();
            }
        }
    signals:
        void hotplugEvent();
};


enum AxisNames { X = 0, Y, Z, A, B, C, U, V, W };


class axis
{
    public:
        axis(); // constructor
        float posMm();
        int posPulse(float posMm);

    public:
        float minVelo;
        float maxVelo;
        float acceleration;
        int   pulsePerMm;
        float actualPosmm;
        int   actualPosPulses;
        bool  invert;
        bool  useLimitMin;
        bool  useLimitMax;
        bool  actualLimitMax;
        bool  actualLimitMin;
        float startPos;
        bool  checkSoftLimits;

        float softMax;
        float softMin;
        float areaMin;
        float areaMax;
        float home;
        bool  wrong;
};


class mk1Settings
{
    public:
        // current size of free buffer
        static short FreebuffSize;

        // number of current instuction
        static int NumberCompleatedInstruction;

        axis coord[4]; // array of 4 axis for mk1
        //         QVector<axis> mk2[9]; // array of 9 axis for mk2

        static bool setSettings;
        static int spindleMoveSpeed;
        static bool spindleEnabled;
        static bool mistEnabled;
        static bool fluidEnabled;

        static bool Estop;

        // for virtual controller
        static bool DEMO_DEVICE;
};


// class for communication with device
class mk1Data : public mk1Settings
{
    public:
        enum TypeSignal {
            None = 0,
            Hz,
            RC
        };

    public:
        void packC0(byte byte05 = 0x0, bool send = true);
        void packB5(bool spindleON, int numShimChanel = 0, TypeSignal ts = None, int SpeedShim = 61535, bool send = true);
        void packAA(bool send = true);
        void packA0( bool send = true);
        void packA1( bool send = true);
        void packC8(int x, int y, int z, int a, bool send = true);
        void packD3( bool send = true);
        void packC2( bool send = true);
        void packB6( bool mist, bool fluid, bool send = true);
        void packAB( bool send = true);
        void packD2(int speed, float returnDistance, bool send = true);
        void packBE(byte direction, int speed, bool send = true);
        void pack9E(byte value, bool send = true);
        void pack9F( bool send = true);
        void packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send = true);
        void packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, float distance, int _pause = 0x39, bool send = true);
        void packCA(float _posX, float _posY, float _posZ, float _posA, int _speed, int _NumberInstruction, float distance, int _pause = 0x39, bool send = true);
        void packFF(bool send = true);
        void pack9D(bool send = true);
        void setByte(byte offset, byte data);
        byte getByte(byte offset);
        void sendBinaryData(bool checkBuffSize = true);

    private:
        static void packFourBytes(byte offset, int val);
        static void packTwoBytes(byte offset, int val);
        static void cleanBuf(byte *m);

    public:
        static libusb_device_handle *handle;
        static libusb_device_descriptor desc;

        // raw data
        static byte writeBuf[BUFFER_SIZE];
        static byte readBuf[BUFFER_SIZE];
};


// devise state
enum EStatusDevice { Connect = 0, Disconnect };


class usbReadThread;


class mk1Controller : public QObject, public mk1Data
{
        Q_OBJECT

    public:
        explicit mk1Controller(QObject *parent = 0);
        virtual ~mk1Controller();

    signals:
        // for logging
        void Message (int num);

        // signal if connect/disconnect
        void hotplugSignal();

        // new data from controller
        void newDataFromMK1Controller();

    public slots:
        void handleHotplug();
        void readNewData();
        void onBufFree();

    private:
        int count;
        bool devConnected;
        bool spindleSetEnable;
        bool fluidSetEnable;
        bool mistSetEnable;

        QStringList axisList;

        static libusb_hotplug_callback_handle hotplug[2];

        usbHotplugThread *hotplugThread;

        static QString devDescriptor;

        //
        // thread for reading from controller
        usbReadThread *readThread;

        QSettings *settingsFile; // main settings file

        int _error_code;

        int product_id, vendor_id;//, class_id;

        bool availableNewData;

        QTimer hotplugTimer;

    public:
        void spindleON();
        void spindleOFF();
        void mistON();
        void mistOFF();
        void fluidON();
        void fluidOFF();
        void emergyStop();
        void stopManualMove();
        void deviceNewPosition(int x, int y, int z, int a = 0);
        void deviceNewPosition(float x, float y, float z, float a = 0.0);
        void startManualMove(QString x, QString y, QString z, QString a, int speed);

        void setStartPos(float x, float y, float z, float a = 0.0);
        void setUseHome(bool b);

        QString getDescription();
        static void setDescription(const QString &s);
        static void resetDescription();
        static int getDeviceInfo();

        void loadSettings();
        void saveSettings();
        void sendSettings();

        //         int  getConfiguration();
        bool testAllowActions();

        bool isConnected();
        int  getSpindleMoveSpeed();
        long numberCompleatedInstructions();
        bool isSpindelOn();
        bool isMistOn();
        bool isFluidOn();
        bool isEmergencyStopOn();
        int  availableBufferSize();

    private:
        void parseBinaryInfo();
        void ADDMessage(int code);

        Q_DISABLE_COPY(mk1Controller);

        //     private slots:
        //         void threadsStart();
        //         void threadFinished();
        //         void processReadBytes(const QByteArray &bytes);

};


class usbReadThread : public QThread
{
        Q_OBJECT
    public:
        usbReadThread(QObject *parent) : QThread(parent) {
            p = (mk1Controller*)parent;
        }
        void run() {
            // init of read array
            memset( buf, 0x0, BUFFER_SIZE);

            while(p->handle) { // running if connected
                int bytesRead  = 0;
                int _error_code = libusb_bulk_transfer(p->handle, BULK_ENDPOINT_OUT, buf, BUFFER_SIZE, &bytesRead, 3000);

                if (_error_code < 0 || bytesRead != BUFFER_SIZE) {
                    continue;
                }

                if (bytesRead == 0) {
                    continue;
                }

                if (buf[0] != 0x01) {
                    continue; // we get only data with code 0х01
                }

                //                 if (buf[1] < 2){
                //                     emit bufIsFree(); // this was never called
                //                 }

                if (memcmp(buf, p->readBuf, BUFFER_SIZE) != 0) { // quick compare
                    memcpy(p->readBuf, buf, BUFFER_SIZE);

                    emit readEvent();
                }
            }

            exit();
        }
    signals:
        void readEvent();
        //         void bufIsFree();

    private:
        byte buf[BUFFER_SIZE];
        mk1Controller* p;
};


#endif // MK1CONTROLLER_H
