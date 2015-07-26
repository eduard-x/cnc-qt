/****************************************************************************
 * C++ Implementation:                                                      *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
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
#include <QByteArray>
#include <QThread>
#include <QIODevice>

#include "MainWindow.h"
#include "Translator.h"

#include <libusb-1.0/libusb.h>

// #define USB_ENDPOINT_IN    ( LIBUSB_ENDPOINT_IN  | 1 )   /* endpoint address */
// #define USB_ENDPOINT_OUT   ( LIBUSB_ENDPOINT_OUT | 2 )   /* endpoint address */

#define BULK_ENDPOINT_OUT     0x81
#define BULK_ENDPOINT_IN      0x01

// extern struct dPoint;

class mk1Controller;

#if 0
static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);

    if (LIBUSB_SUCCESS != rc) {
        fprintf (stderr, "Error getting device descriptor\n");
    }

    printf ("Device attached: %04x:%04x\n", desc.idVendor, desc.idProduct);

    libusb_open (dev, &mk1Controller::handle);

    //     done++;

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    printf ("Device detached\n");

    libusb_close (mk1Controller::handle);

    //     done++;
    return 0;
}
#endif


class usbHotplugThread : public QThread
{
        Q_OBJECT
        void run()
        {
            /* expensive or blocking operation  */

            while(true) {
                libusb_handle_events(NULL);
                msleep(50);
                //                                 qDebug() << "hotdog" <<;
                emit hotplugEvent();
            }
        }
    signals:
        void hotplugEvent();
};



// Класс для получиния бинарных данных

class BinaryData
{
    public:
        enum TypeSignal {
            None,
            Hz,
            RC
        };

        static void packC0(byte byte05 = 0x0, bool send = true);
        static void packB5(bool shpindelON, int numShimChanel = 0, TypeSignal ts = None, int SpeedShim = 0, bool send = true);
        static void packAA(bool send = true);
        static void packC8(int x, int y, int z, bool send = true);
        static void packD2(int speed, double returnDistance, bool send = true);
        static void packBE(byte direction, int speed, bool send = true);
        static void pack9E(byte value, bool send = true);
        static void packBF(int speedLimitX, int speedLimitY, int speedLimitZ, bool send = true);
        static void packCA(int _posX, int _posY, int _posZ, int _speed, int _NumberInstruction, bool send = true);
        static void packFF(bool send = true);
        static void pack9D(bool send = true);

        static void setByte(short offset, byte data);
        static byte getByte(short offset);
        static void cleanBuf(byte *m);
        static void sendBinaryData(bool checkBuffSize = true);

    public:
        static libusb_device_handle *handle;
        static libusb_device_descriptor desc;

        //     protected:
        static byte writeBuf[BUFFER_SIZE];
        static byte readBuf[BUFFER_SIZE];
        //         static byte newBuf[BUFFER_SIZE]; // for compaire of readBuf from controller

};


// Статусы работы с устройством
enum EStatusDevice { Connect = 0, Disconnect };


class DeviceInfo
{
    public:
        // Сырые данные от контроллера
        static byte rawDataRead[BUFFER_SIZE];
        static byte rawDataWrite[BUFFER_SIZE];

        // Размер доступного буфера в контроллере
        static short FreebuffSize;

        // Номер выполненной инструкции
        static int NumberCompleatedInstruction;

        // Текущее положение в импульсах
        static int AxesX_PositionPulse;

        // Текущее положение в импульсах
        static int AxesY_PositionPulse;

        // Текущее положение в импульсах
        static int AxesZ_PositionPulse;

        // Текущее положение в импульсах
        static int AxesA_PositionPulse;

        static int AxesX_PulsePerMm;
        static int AxesY_PulsePerMm;
        static int AxesZ_PulsePerMm;
        static int AxesA_PulsePerMm;

        //срабатывание сенсора
        static bool AxesX_LimitMax;
        static bool AxesX_LimitMin;
        static bool AxesY_LimitMax;
        static bool AxesY_LimitMin;
        static bool AxesZ_LimitMax;
        static bool AxesZ_LimitMin;
        static bool AxesA_LimitMax;
        static bool AxesA_LimitMin;

        static int shpindel_MoveSpeed;
        static bool shpindel_Enable;

        static bool Estop;

        // Использование виртуального контроллера
        static bool DEMO_DEVICE;

    public: // methods
        static double AxesX_PositionMM();
        static double AxesY_PositionMM();
        static double AxesZ_PositionMM();
        static double AxesA_PositionMM();
        static byte getByte(short pos);

        //
        // Вычисление положения в импульсах, при указании оси, и положения в миллиметрах
        //
        // параметр "axes">имя оси X,Y,Z
        // параметр "posMm">положение в мм
        // возвращаемый: Количество импульсов
        static int CalcPosPulse(QString axes, double posMm);
};


class usbReadThread;


class mk1Controller : public QObject, public BinaryData
{
        Q_OBJECT

    public:
        explicit mk1Controller(QObject *parent = 0);
        virtual ~mk1Controller();

        //     private:
        //         MainWindow* parent;


    signals:
        void Message (int num);
        void wasConnected();
        void wasDisconnected();
        void hotplugSignal();
        //         void hotplugDisconnected();
        void newDataFromMK1Controller();
        //         void hotplugEvent();
#if 0
    public:
        delegate void DeviceEventConnect(object sender); //уведомление об установки связи
        delegate void DeviceEventDisconnect(object sender, DeviceEventArgsMessage e); //уведомление об обрыве/прекращении связи
        delegate void DeviceEventNewData(object sender); //уведомление что получены новые данные контроллером
        delegate void DeviceEventNewMessage(object sender, DeviceEventArgsMessage e); //для посылки управляющей программе сообщений о действиях

        //
        // Событие при успешном подключении к контроллеру
        //
        public event DeviceEventConnect WasConnected;
        //
        // Событие при отключении от контроллера, или разрыве связи с контроллером
        //
        public event DeviceEventDisconnect WasDisconnected;
        //
        // Получены новые данные от контроллера
        //
        public event DeviceEventNewData NewDataFrommk1Controller;
        //
        // Посылка строки описания (для ведения логов)
        //
        public event DeviceEventNewMessage Message;
#endif
        //     public:
        //         BinaryData rawData;

    public slots:
        //         void processBytes(const QByteArray &bytes);
        void handleHotplug();
        void readNewData();

    private:
        int count;

        static libusb_hotplug_callback_handle hotplug[2];
        //         DeviceInfo devInfo;
//     private:
        //
        // Поток для получения, посылки данных в контроллер
        //
        //         QThread *readThread;

        usbHotplugThread *hotplugThread;
        usbReadThread *readTHread;

        //         byte readBuffer[BUFFER_SIZE];
        //         byte _oldInfoFrommk1Controller[BUFFER_SIZE];

        QSettings *settingsFile; // Файл настроек программы

        //         libusb_device_handle *dev_handle; //a device handle
        //         libusb_context *ctx; //a libusb session
        //         libusb_device *_myUsbDevice;

        int _error_code;

        int product_id, vendor_id;//, class_id;
        //         UsbEndpointReader _usbReader;
        //         UsbEndpointWriter _usbWriter;

        bool availableNewData;
        //  DataCode   dataCode;

        //     private:
        //         bool _connected;
        QTimer hotplugTimer;

    public:
        void spindelON();
        void spindelOFF();
        void emergyStop();
        void stopManualMove();
        void deviceNewPosition(int x, int y, int z, int a = 0);
        void deviceNewPosition(double x, double y, double z, double a = 0.0);
        void startManualMove(QString x, QString y, QString z, QString a, int speed);

        void loadSettings();
        void saveSettings();

        int  getConfiguration();
        bool testAllowActions();

        bool isConnected(); // Возвращает информацию о наличии связи
        int  spindelMoveSpeed(); // Скорость движения шпинделя
        long numberComleatedInstructions(); // Номер выполняемой инструкцииConnected
        bool isSpindelOn(); // Свойство включен ли шпиндель
        bool isEstopOn(); // Свойство активированна ли аварийная остановка
        int  availableBufferSize();// Размер свободного буфера

    private:
        //         int read(libusb_device_handle *handle);
        //         int write(libusb_device_handle *handle);
        //         int readWrite(libusb_device_handle *handle);
        //         int readWriteLoop(libusb_device_handle *handle);

        void parseBinaryInfo();
        //         bool CompareArray(byte *arr1, byte *arr2);
        void ADDMessage(int code);

        //         static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
        //         static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
        //  void ADDMessage(const QString &s);
// private:
        Q_DISABLE_COPY(mk1Controller);

    private slots:
//         void threadsStart();
//         void threadFinished();
        //         void processReadBytes(const QByteArray &bytes);

};


class usbReadThread : public QThread
{
        Q_OBJECT
    public:
        usbReadThread(QObject *parent) : QThread(parent)
        {
            p = (mk1Controller*)parent;
        }
        void run()
        {
            /* expensive or blocking operation  */
            memset( buf, 0x0, BUFFER_SIZE);

            while(p->handle) {
                int bytesRead  = 0;
                int _error_code = libusb_bulk_transfer(p->handle, BULK_ENDPOINT_OUT, buf, BUFFER_SIZE, &bytesRead, 3000);
                //                 qDebug() << "read" << bytesRead << "err code" << _error_code;

                if (_error_code < 0 || bytesRead != BUFFER_SIZE) {
                    //                     qDebug() << "libusb_bulk_read failed:" << libusb_error_name(_error_code);

                    //                 if (_error_code != None) {
                    //                     //                     _connected = false;
                    //
                    //                     ADDMessage(_BREAK_CONN);
                    //
                    //                     //                     emit wasDisconnected();
                    //                 }
                    continue;
                }


                if (bytesRead == 0 || buf[0] != 0x01) {
                    continue; //пока получаем пакеты только с кодом 0х01
                }

                //                 qDebug() << "buf" << buf[0];

                if (memcmp(buf, p->readBuf, BUFFER_SIZE) != 0) {
                    memcpy(p->readBuf, buf, BUFFER_SIZE);


                    //                     availableNewData = true;

                    emit readEvent();
                }

                //                 libusb_handle_events(NULL);
                //                 msleep(50);
                //                 //                                 qDebug() << "hotdog" <<;
                //                 emit hotplugEvent();
            }
        }
    signals:
        void readEvent();

    private:
        byte buf[BUFFER_SIZE];
        mk1Controller* p;
};


#endif // MK1CONTROLLER_H
