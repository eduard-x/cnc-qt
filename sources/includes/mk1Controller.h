/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QThread>
#include <QIODevice>

#include "MainWindow.h"
#include "Translator.h"
// #include "Settings.h"

#include <libusb-1.0/libusb.h>

#define BULK_ENDPOINT_OUT     0x81
#define BULK_ENDPOINT_IN      0x01


class mk1Controller;


// use enum AxisNames for 'pos' array
struct moveParameters {
    float pos[4]; // X, Y, Z, A
    //     float posX;
    //     float posY;
    //     float posZ;
    //     float posA;
    int   restPulses; // offset 46
    int   speed; // vector speed, offset 43
    int   numberInstruction;
    byte  movingCode; // 0x39, 0x31, 0x21, 0x11, 0x01, offset 5
};

#if 0
class usbHotplugThread : public QThread
{
        Q_OBJECT
        void run()
        {
            /* expensive or blocking operation  */
            while(true) {
                //                 int r =
                libusb_handle_events(NULL);
                msleep(150);
                //                 qDebug() << "libusb_handle_events" << r;

                emit hotplugEvent();
            }
        }
    signals:
        void hotplugEvent();
};
#endif

// class for communication with device
class mk1Data //: public mk1Settings
{
    public:
        enum TypeSignal {
            None = 0,
            Hz,
            RC
        };

    public:
        void packC0(byte byte05 = 0x0, bool send = true);
        void packB5(bool spindleON, int numPWMChanel = 0, TypeSignal ts = None, int SpeedPWM = 61535, bool send = true);
        void packAA(bool send = true);
        void packA0( bool send = true);
        void packA1( bool send = true);
        void packC8(int x, int y, int z, int a, bool send = true);
        void packD3( bool send = true);
        void packC2( bool send = true);
        void packB6( bool mist, bool fluid, bool send = true);
        void packAB( bool send = true);
        void packD2(int speed, float returnDistance, bool send = true);
        void packBE(byte direction, int speed, int lenInPulses, bool send = true);
        void pack9E(byte value, bool send = true);
        void pack9F( bool send = true);
        void packBF(int speedLimit[4], bool send = true);
        //         void packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, int _code, bool send = true);
        //         void packCA(float _posX, float _posY, float _posZ, float _posA, int _speed, int _NumberInstruction, int _code, bool send = true);
        void packCA(moveParameters *params, bool send = true);
        void packFF(bool send = true);
        void pack9D(bool send = true);
        void setByte(byte offset, byte data);
        byte getByte(byte offset);
        void sendBinaryData(bool checkBuffSize = true);
        //
        int  getSpindleMoveSpeed();
        //         void getSpindleMoveSpeed(int i);
        void setSpindleMoveSpeed(int i);
        long numberCompleatedInstructions();
        void setCompleatedInstructions(long i);
        bool isSpindelOn();
        void setSpindelOn(bool b);
        bool isMistOn();
        void setMistOn(bool b);
        bool isFluidOn();
        void setFluidOn(bool b);
        bool isEmergencyStopOn();
        void setEmergencyStopOn(bool b);
        int  availableBufferSize();
        void setBufferSize(int n);



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

        //
        bool setSettings;

    private:
        int  spindleMoveSpeed;
        bool spindleEnabled;
        bool mistEnabled;
        bool fluidEnabled;
        // number of current instuction
        int numberCompleatedInstruction;

        bool eStop;

        // current size of free buffer
        short freebuffSize;
};


// devise state
enum StatusDevice { Connect = 0, Disconnect };


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
        //         void onHandleHotplug();
        void onDeviceConnected();
        void onDeviceDisconnected();

        void onReadNewData();
        void onBufFree();

    private:
//         int count;
        bool devConnected;
        bool spindleSetEnable;
        bool fluidSetEnable;
        bool mistSetEnable;

        static libusb_hotplug_callback_handle hotplug[2];

        //         usbHotplugThread *hotplugThread;

        static QString devDescriptor;

        //
        // thread for reading from controller
        usbReadThread *readThread;

//         int _error_code;

        //         int product_id, vendor_id;//, class_id;

//         bool availableNewData;

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
        void startManualMove(QString x, QString y, QString z, QString a, int speed, int pulses);

        void setStartPos(float x, float y, float z, float a = 0.0);
        void setUseHome(bool b);

        QString getDescription();
        static void setDescription(const QString &s);
        static void resetDescription();
        static int getDeviceInfo();

        void sendSettings();

        //         int  getConfiguration();
        bool testAllowActions();

        bool isConnected();


    private:

        void ADDMessage(int code);
        void parseBinaryInfo();

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
        /**
         * constructor
         */
        usbReadThread(QObject *parent) : QThread(parent)
        {
            p = static_cast<mk1Controller*>(parent);
        }
        /**
         * destructor
         */
        ~usbReadThread()
        {
            if (p->handle) {
                libusb_release_interface(p->handle, 0);
                libusb_close(p->handle);
            }
        }

        void run()
        {
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

                if (buf[0] == 0x01 || buf[0] == 0x04) {// we get only data with code 0х01 or 0x04
                    if (memcmp(buf, p->readBuf, BUFFER_SIZE) != 0) { // quick compare
                        memcpy(p->readBuf, buf, BUFFER_SIZE);

                        emit readEvent();
                    }
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
