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
        void run()
        {
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



// Класс для получиния бинарных данных
class BinaryData
{
    public:
        enum TypeSignal {
            None = 0,
            Hz,
            RC
        };

        static void packC0(byte byte05 = 0x0, bool send = true);
        static void packB5(bool spindelON, int numShimChanel = 0, TypeSignal ts = None, int SpeedShim = 0, bool send = true);
        static void packAA(bool send = true);
        static void packC8(int x, int y, int z, int a, bool send = true);
        static void packD2(int speed, double returnDistance, bool send = true);
        static void packBE(byte direction, int speed, bool send = true);
        static void pack9E(byte value, bool send = true);
        static void packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send = true);
        static void packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, bool send = true);
        static void packFF(bool send = true);
        static void pack9D(bool send = true);

        static void setByte(short offset, byte data);
        static byte getByte(short offset);
        static void cleanBuf(byte *m);
        static void sendBinaryData(bool checkBuffSize = true);

    public:
        static libusb_device_handle *handle;
        static libusb_device_descriptor desc;

        // Сырые данные от контроллера
        static byte writeBuf[BUFFER_SIZE];
        static byte readBuf[BUFFER_SIZE];
};


// Статусы работы с устройством
enum EStatusDevice { Connect = 0, Disconnect };


class DeviceInfo
{
    public:
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

        static int spindel_MoveSpeed;
        static bool spindel_Enable;

        static bool Estop;

        // Использование виртуального контроллера
        static bool DEMO_DEVICE;

    public: // methods
        static double AxesX_PositionMM();
        static double AxesY_PositionMM();
        static double AxesZ_PositionMM();
        static double AxesA_PositionMM();
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

    signals:
        // Посылка строки описания (для ведения логов)
        void Message (int num);
        // Событие при успешном подключении к контроллеру
        // Событие при отключении от контроллера, или разрыве связи с контроллером
        void hotplugSignal();

        // Получены новые данные от контроллера
        void newDataFromMK1Controller();

    public slots:
        void handleHotplug();
        void readNewData();
        void onBufFree();

    private:
        int count;
        bool devConnected;

        static libusb_hotplug_callback_handle hotplug[2];

        usbHotplugThread *hotplugThread;

        static QString devDescriptor;

        //
        // Поток для получения, посылки данных в контроллер
        usbReadThread *readThread;

        QSettings *settingsFile; // Файл настроек программы

        int _error_code;

        int product_id, vendor_id;//, class_id;
        //         UsbEndpointReader _usbReader;
        //         UsbEndpointWriter _usbWriter;

        bool availableNewData;

        QTimer hotplugTimer;

    public:
        void spindelON();
        void spindelOFF();
        void emergyStop();
        void stopManualMove();
        void deviceNewPosition(int x, int y, int z, int a = 0);
        void deviceNewPosition(double x, double y, double z, double a = 0.0);
        void startManualMove(QString x, QString y, QString z, QString a, int speed);

        QString getDescription();
        static void setDescription(const QString &s);
        static void resetDescription();

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
        void parseBinaryInfo();
        void ADDMessage(int code);

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
                    continue; //пока получаем пакеты только с кодом 0х01
                }

                //                 if (buf[1] < 2){
                //                     emit bufIsFree(); // здесь не выполнялось никогда.
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
