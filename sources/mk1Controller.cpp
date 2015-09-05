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

#include <QDebug>
#include <QIODevice>

#include "includes/mk1Controller.h"

#include <QString>

using namespace std;
/******************************************************************************
** mk1Controller
*/


short DeviceInfo::FreebuffSize = 0;

int DeviceInfo::NumberCompleatedInstruction = 0;
int DeviceInfo::AxesX_PositionPulse = 0;
int DeviceInfo::AxesY_PositionPulse = 0;
int DeviceInfo::AxesZ_PositionPulse = 0;
int DeviceInfo::AxesA_PositionPulse = 0;

int DeviceInfo::AxesX_PulsePerMm = 400;
int DeviceInfo::AxesY_PulsePerMm = 400;
int DeviceInfo::AxesZ_PulsePerMm = 400;
int DeviceInfo::AxesA_PulsePerMm = 400;

bool DeviceInfo::AxesX_LimitMax = false;
bool DeviceInfo::AxesX_LimitMin = false;
bool DeviceInfo::AxesY_LimitMax = false;
bool DeviceInfo::AxesY_LimitMin = false;
bool DeviceInfo::AxesZ_LimitMax = false;
bool DeviceInfo::AxesZ_LimitMin = false;
bool DeviceInfo::AxesA_LimitMax = false;
bool DeviceInfo::AxesA_LimitMin = false;

int DeviceInfo::spindel_MoveSpeed = 0;
bool DeviceInfo::spindel_Enable = false;

bool DeviceInfo::Estop = false;

bool DeviceInfo::DEMO_DEVICE = false;


// static
libusb_device_handle *BinaryData::handle = 0;

libusb_hotplug_callback_handle mk1Controller::hotplug[2];

libusb_device_descriptor BinaryData::desc = {0};

QString mk1Controller::devDescriptor;


static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{

    int rc;
    QString descrStr;

    //     struct libusb_device_descriptor desc;

    rc = libusb_get_device_descriptor(dev, &mk1Controller::desc);

    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "Error getting device descriptor";
        mk1Controller::handle = 0;
        return -1;
    }

    qDebug() << "Device to attache" << mk1Controller::handle;

    //     qDebug() << QString("Device attached: %1:%2").arg(desc.idVendor).arg(desc.idProduct);

    if (mk1Controller::handle) {
        libusb_close (mk1Controller::handle);
        mk1Controller::handle = 0;
        return -1;
    }

    rc = libusb_open (dev, &mk1Controller::handle);

    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "Error opening device";
        mk1Controller::handle = 0;
        return -1;
    }

    // get device descriptor
    descrStr = QString().sprintf("VendorID: 0x%x ProductID: 0x%x\n\n", mk1Controller::desc.idVendor,  mk1Controller::desc.idProduct);
    descrStr += QString().sprintf("Number of possible configurations: %d\n", mk1Controller::desc.bNumConfigurations);
    descrStr += QString().sprintf("Device Class: %d\n\n", mk1Controller::desc.bDeviceClass);

    libusb_config_descriptor *config;
    libusb_get_config_descriptor(dev, 0, &config);
    descrStr += QString().sprintf("Interfaces: %d\n\n", config->bNumInterfaces);
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;

    for(int i = 0; i < (int)config->bNumInterfaces; i++) {
        inter = &config->interface[i];
        descrStr += QString().sprintf("Number of alternate settings: %d", inter->num_altsetting);

        for(int j = 0; j < inter->num_altsetting; j++) {
            interdesc = &inter->altsetting[j];
            descrStr += QString().sprintf("\nInterface Number: %d\n", interdesc->bInterfaceNumber);
            descrStr += QString().sprintf("Number of endpoints: %d\n", interdesc->bNumEndpoints);
            descrStr += QString().sprintf("Alternate Setting: %d\n", interdesc->bAlternateSetting);
            descrStr += QString().sprintf("Interface Class: %d\n", interdesc->bInterfaceClass);
            descrStr += QString().sprintf("Interface SubClass: %d\n", interdesc->bInterfaceSubClass);
            descrStr += QString().sprintf("Interface: %d\n", interdesc->iInterface);

            for(int k = 0; k < (int)interdesc->bNumEndpoints; k++) {
                epdesc = &interdesc->endpoint[k];
                descrStr += QString().sprintf("\nDescriptor Type: %d\n", epdesc->bDescriptorType);
                descrStr += QString().sprintf("Endpoint Address: %d\n", epdesc->bEndpointAddress);
                descrStr += QString().sprintf("Attributes: %d\n", epdesc->bmAttributes);
                descrStr += QString().sprintf("MaxPacketSize: %d\n", epdesc->wMaxPacketSize);
                descrStr += QString().sprintf("Interval: %d\n", epdesc->bInterval);
                descrStr += QString().sprintf("Refresh: %d\n", epdesc->bRefresh);
                descrStr += QString().sprintf("SynchAddress: %d\n", epdesc->bSynchAddress);
            }
        }
    }

    //     qDebug() << descrStr;

    libusb_free_config_descriptor(config);

    mk1Controller::setDescription(descrStr);
    // end of get device descriptor

#ifdef __linux__

    if(libusb_kernel_driver_active(mk1Controller::handle, 0) == 1) {
        qDebug() << "Kernel Driver Active";

        if(libusb_detach_kernel_driver(mk1Controller::handle, 0) == 0)    {
            qDebug() <<  "Kernel Driver Detached!";
        } else         {
            qDebug() << "Couldn't detach kernel driver!";
            libusb_close(mk1Controller::handle);
            return -1;
        }
    }

#endif

    int e = libusb_set_configuration (mk1Controller::handle, 1);

    if(e != 0)  {
        qDebug() << "Error in libusb_set_configuration";
        libusb_close(mk1Controller::handle);
        return -1;
    } else {
        qDebug() << "Device is in configured state!";
    }

    e = libusb_claim_interface(mk1Controller::handle, 0);

    if(e < 0) {
        qDebug() << "Cannot Claim Interface";
        libusb_close(mk1Controller::handle);
        return -1;
    } else {
        qDebug() << "Claimed Interface";
    }


    //     libusb_claim_interface (mk1Controller::handle, 0);

    // open read endpoint 1.
    //     mk1Controller::usbReader = _myUsbDevice.OpenEndpointReader(ReadEndpointID.Ep01);

    // open write endpoint 1.
    //     mk1Controller::usbWriter = _myUsbDevice.OpenEndpointWriter(WriteEndpointID.Ep01);

    //     emit mk1Controller::wasConnected();

    qDebug() << "Device attached" << mk1Controller::handle;

    return 0;
}


static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    qDebug() << "Device to detache" << mk1Controller::handle;

    if (mk1Controller::handle) {
        libusb_close (mk1Controller::handle);
    }

    mk1Controller::handle = 0;

    mk1Controller::resetDescription();

    //     emit mk1Controller::wasDisconnected();

    qDebug() << "Device detached";

    return 0;
}



mk1Controller::mk1Controller(QObject *parent) : QObject(parent)
{
    int rc;

    vendor_id  =  0x2121;
    product_id = 0x2130;

    handle = NULL;

    devConnected = false;

    int class_id   = LIBUSB_HOTPLUG_MATCH_ANY;
    //     libusb_context* ct;

    rc = libusb_init (NULL);

    if (rc < 0) {
        qDebug() << QString("failed to initialise libusb: %1").arg(libusb_error_name(rc));
    }

    hotplugThread = 0;

    readThread = 0;

    handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id); //these are vendorID and productID I found for my usb device

    if(handle == NULL) {
        qDebug() << "Device not connected" << endl;
        //         return false;
    } else {
        //         dev = libusb_get_device(handle);
        qDebug() << "Device connected" << handle << endl;


        //         if (handle) {
#ifdef __linux__
        //     qDebug() << "linux detach kernel driver";

        if(libusb_kernel_driver_active(handle, 0) == 1) {
            qDebug() << "Kernel Driver Active";

            if(libusb_detach_kernel_driver(handle, 0) == 0)    {
                qDebug() <<  "Kernel Driver Detached!";
            } else         {
                qDebug() << "Couldn't detach kernel driver!";
                libusb_close(handle);
                handle = 0;
            }
        }

#endif

        int e = libusb_set_configuration (handle, 1);

        if(e != 0)  {
            qDebug() << "Error in libusb_set_configuration";
            libusb_close(handle);
            handle = 0;
            //         return -1;
        } else    {
            qDebug() << "Device is in configured state!";
        }

        e = libusb_claim_interface(mk1Controller::handle, 0);

        if(e < 0) {
            qDebug() << "Cannot Claim Interface";
            libusb_close(handle);
            handle = 0;
        } else    {
            qDebug() << "Claimed Interface";
        }
    }

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
        qDebug() << "Hotplug capabilites are not supported on this platform";
        libusb_exit (NULL);
    } else {
        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_ENUMERATE, vendor_id,
                                               product_id, class_id, hotplug_callback, NULL, &hotplug[0]);

        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "Error registering callback 0";
            libusb_exit (NULL);
        } else {
            qDebug() << "Device registering attach callback";
        }

        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, vendor_id,
                                               product_id, class_id, hotplug_callback_detach, NULL, &hotplug[1]);

        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "Error registering callback 1";
            libusb_exit (NULL);
        } else {
            qDebug() << "Device registering detach callback";
            hotplugThread = new usbHotplugThread();
            connect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(handleHotplug()));
            hotplugThread->start();
        }
    }

    settingsFile = new QSettings("CNCSoft", "CNC-Qt");
}


mk1Controller::~mk1Controller()
{
    if (handle) {
        int e = libusb_release_interface(handle, 0);
        libusb_close(handle);
    }

    libusb_exit(0);
}


void mk1Controller::resetDescription()
{
    devDescriptor.clear();
}


QString mk1Controller::getDescription()
{
    return devDescriptor;
}


void mk1Controller::setDescription(const QString &c)
{
    devDescriptor = c;
    //     devDescriptor = st.split("\n");
}


// for the popup window
int mk1Controller::getConfiguration()
{
    //     char *data;
    //     int index;

    //     data = (char *)malloc(512);
    //     memset(data,0,512);

    //     index = desc.iiConfiguration;

    struct libusb_config_descriptor *config;

    //     libusb_get_string_descriptor_ascii(hDevice,index,data,512);
    //     libusb_get_config_descriptor(dev, 0, &config);

    struct libusb_endpoint_descriptor *epdesc;

    //     qDebug() <<"Number of endpoints: "<<(int)desc.bNumConfigurations;
    //     for(int k=0; k<(int)desc.bNumConfigurations; k++) {
    //         epdesc = &desc.endpoint[k];
    //         qDebug() <<"Descriptor Type: "<<(int)epdesc->bDescriptorType;
    //         qDebug() <<"EP Address: "<<(int)epdesc->bEndpointAddress;
    //     }

    printf("numInterfaces: %d\n", config->bNumInterfaces);

    //     for(int ii=0; iibNumInterfaces; ii++) {
    // printf("ii=%d\n",ii);
    //         inter = &config->interface[ii];
    //         printf("Number of alternate settings: %d\n",
    //                 inter->num_altsetting);
    //         for(j=0; jnum_altsetting; j++) {
    //             interdesc = &inter->altsetting[j];
    //             printf("Interface Number: %d\n",
    //                 interdesc->bInterfaceNumber);
    //             printf("Number of endpoints: %d\n",
    //                 interdesc->bNumEndpoints);
    //             for(k=0; kbNumEndpoints; k++) {
    //                 epdesc = &interdesc->endpoint[k];
    //             printf("Descriptor Type: %d\n",
    //                 epdesc->bDescriptorType);
    //             printf("EP Address: 0x%X\n",
    //                 epdesc->bEndpointAddress);
    //             printf("Max packet size: %d\n",
    //                 epdesc->wMaxPacketSize);
    //             printf("interval: %d\n",
    //                 epdesc->bInterval);
    //                 }// end for k
    //             }// end for j
    // printf("go1\n");
    //         }//end for ii

    //     qDebug() << "Interface Descriptors: ";
    //     qDebug() << "Number of Interfaces : " << desc.bNumInterfaces;
    //     qDebug() << "Length :" << desc.bLength;
    //     qDebug() << "Desc_Type :" << desc.bDescriptorType;
    //     qDebug() << "Config_index :" << desc.bNumConfigurations;
    //     qDebug() << "Total length :" << desc.bLength;
    //     qDebug() << "Configuration Value  :" << desc.bConfigurationValue;
    //     qDebug() << "Configuration Attributes :" << desc.bmAttributes;
    //     qDebug() << "MaxPower(mA) :" << desc.MaxPower;

    //     free(data);
    //     data = NULL;
    return 0;
}



void mk1Controller::readNewData()
{
    qDebug() << "new data from usb";
    parseBinaryInfo();
}


void mk1Controller::onBufFree()
{
    qDebug() << "signal: read buffer is free";
}


void mk1Controller::handleHotplug()
{
    disconnect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(handleHotplug()));

    if (handle) {
        if (devConnected == false) {
            qDebug() << "hotplug attach" << handle;

            emit hotplugSignal();
        }

        if (readThread == 0) {
            readThread = new usbReadThread(this);

            connect(readThread, SIGNAL(readEvent()), this, SLOT(readNewData()));

            //             connect(readThread, SIGNAL(bufIsFree()), this, SLOT(onBufFree()));
            readThread->start();
        }

        devConnected = true;

    } else {
        if (devConnected == true) {
            qDebug() << "hotplug detach" << handle;

            emit hotplugSignal();
        }

        readThread = 0;

        devConnected = false;
    }

    connect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(handleHotplug()));
}


//
// Загрузка настроек программы из файла
//
void mk1Controller::loadSettings()
{
    bool res;
    int i;

    i = settingsFile->value("pulseX", 400).toInt( &res);

    if (res == true) {
        DeviceInfo::AxesX_PulsePerMm = i;
    }

    i = settingsFile->value("pulseY", 400).toInt( &res);

    if (res == true) {
        DeviceInfo::AxesY_PulsePerMm = i;
    }

    i = settingsFile->value("pulseZ", 400).toInt( &res);

    if (res == true) {
        DeviceInfo::AxesZ_PulsePerMm = i;
    }

    i = settingsFile->value("pulseA", 400).toInt( &res);

    if (res == true) {
        DeviceInfo::AxesA_PulsePerMm = i;
    }
}

//
// Сохранение настроек в файл
//
void mk1Controller::saveSettings()
{
    settingsFile->setValue("pulseX", DeviceInfo::AxesX_PulsePerMm);
    settingsFile->setValue("pulseY", DeviceInfo::AxesY_PulsePerMm);
    settingsFile->setValue("pulseZ", DeviceInfo::AxesZ_PulsePerMm);
    settingsFile->setValue("pulseA", DeviceInfo::AxesA_PulsePerMm);
}


// Свойства для доступа извне к переменным
//TODO: пересмотреть необходимость процедур
//
// Возвращает информацию о наличии связи
//
bool mk1Controller::isConnected()
{
    return (handle != 0);
}

//
// Скорость движения шпинделя
//
int mk1Controller::spindelMoveSpeed()
{
    return DeviceInfo::spindel_MoveSpeed;
}

//
// Номер выполняемой инструкции
//
long mk1Controller::numberComleatedInstructions()
{
    return DeviceInfo::NumberCompleatedInstruction;
}

//
// Свойство включен ли шпиндель
//
bool mk1Controller::isSpindelOn()
{
    return DeviceInfo::spindel_Enable;
}

//
// Свойство активированна ли аварийная остановка
//
bool mk1Controller::isEstopOn()
{
    return DeviceInfo::Estop;
}

//
// Проверка наличия связи, и незанятости контроллера
//
// возвращаемый булево, возможно ли посылать контроллеру задачи
bool mk1Controller::testAllowActions()
{
    if (!isConnected()) {
        //StringError = @"Отсутствует связь с контроллером!";
        return false;
    }

    return true;
}

//
// Для проверки новых данных от контроллера
//
// public bool mk1Controller::AvailableNewData { get; set; }


//
// Размер свободного буфера
//
// ReSharper disable once UnusedMember.Global
int mk1Controller::availableBufferSize()
{
    return DeviceInfo::FreebuffSize;
}


// Поток выполнения задания

//
// Парсит полученные данные с контроллера
//
void mk1Controller::parseBinaryInfo()
{
    DeviceInfo::FreebuffSize = readBuf[1];
    DeviceInfo::spindel_MoveSpeed = (int)(((/*(readBuf[23] << 24) + (readBuf[22] << 16) +*/ (readBuf[21] << 8) + (readBuf[20]))) / 2.1);

    DeviceInfo::AxesX_PositionPulse = ((readBuf[27] << 24) + (readBuf[26] << 16) + (readBuf[25] << 8) + (readBuf[24]));
    DeviceInfo::AxesY_PositionPulse = ((readBuf[31] << 24) + (readBuf[30] << 16) + (readBuf[29] << 8) + (readBuf[28]));
    DeviceInfo::AxesZ_PositionPulse = ((readBuf[35] << 24) + (readBuf[34] << 16) + (readBuf[33] << 8) + (readBuf[32]));

    DeviceInfo::AxesX_LimitMax = (readBuf[15] & (1 << 0)) != 0;
    DeviceInfo::AxesX_LimitMin = (readBuf[15] & (1 << 1)) != 0;
    DeviceInfo::AxesY_LimitMax = (readBuf[15] & (1 << 2)) != 0;
    DeviceInfo::AxesY_LimitMin = (readBuf[15] & (1 << 3)) != 0;
    DeviceInfo::AxesZ_LimitMax = (readBuf[15] & (1 << 4)) != 0;
    DeviceInfo::AxesZ_LimitMin = (readBuf[15] & (1 << 5)) != 0;
    DeviceInfo::AxesA_LimitMax = (readBuf[15] & (1 << 6)) != 0;
    DeviceInfo::AxesA_LimitMin = (readBuf[15] & (1 << 7)) != 0;

    DeviceInfo::NumberCompleatedInstruction = ((readBuf[9] << 24) + (readBuf[8] << 16) + (readBuf[7] << 8) + (readBuf[6]));

    byte bb19 = readBuf[19];

    DeviceInfo::spindel_Enable = (bb19 & (1 << 0)) ? true : false;

    byte bb14 = readBuf[14];
    DeviceInfo::Estop = (bb14 & (1 << 7)) ? true : false;

    emit newDataFromMK1Controller();
}


void mk1Controller::ADDMessage(int num)
{
    //     if (Message != NULL) {
    //         Message(this, new DeviceEventArgsMessage(s));
    //     }
    emit Message(num);
}


//
// Включение шпинделя
//
void mk1Controller::spindelON()
{
    packB5(true);
}

//
// Выключение шпинделя
//
void mk1Controller::spindelOFF()
{
    packB5(false);
}

//
// Посылка аварийной остановки
//
void mk1Controller::emergyStop()
{
    packAA();
}

//
// Запуск движения без остановки
//
// параметр "x" Ось Х (доступные значения "+" "0" "-")
// параметр "y" Ось Y (доступные значения "+" "0" "-")
// параметр "z" Ось Z (доступные значения "+" "0" "-")
// параметр "speed"
void mk1Controller::startManualMove(QString x, QString y, QString z, QString a, int speed)
{
    if (!isConnected()) {
        //QStringError = "Для выключения шпинделя, нужно вначале установить связь с контроллером";
        //return false;
        return;
    }

    //if (!IsFreeToTask)
    //{
    //    return;
    //}

    byte axesDirection = 0x00; // = new SuperByte(0x00);

    //поставим нужные биты
    if (x == "-") {
        axesDirection |= (1 << 0); //.SetBit(0, true);
    }

    if (x == "+") {
        axesDirection |= (1 << 1); //.SetBit(1, true);
    }

    if (y == "-") {
        axesDirection |= (1 << 2); //.SetBit(2, true);
    }

    if (y == "+") {
        axesDirection |= (1 << 3); //.SetBit(3, true);
    }

    if (z == "-") {
        axesDirection |= (1 << 4); //.SetBit(4, true);
    }

    if (z == "+") {
        axesDirection |= (1 << 5); //.SetBit(5, true);
    }

    if (a == "-") {
        axesDirection |= (1 << 6); //.SetBit(6, true);
    }

    if (a == "+") {
        axesDirection |= (1 << 7); //.SetBit(7, true);
    }

    //DataClear();
    //DataAdd(BinaryData.packBE(axesDirection.valueByte, speed));
    packBE(axesDirection, speed);
    //Task_Start();
}


void mk1Controller::stopManualMove()
{
    if (!isConnected()) {
        //QStringError = "Для выключения шпинделя, нужно вначале установить связь с контроллером";
        //return false;
    }

    //     QByteArray buff = BinaryData.packBE(0x00, 0);

    packBE(0x00, 0, false);
    setByte(22, 0x01);
    //TODO: разобраться для чего, этот байт
    //     buff[22] = 0x01;

    //DataClear();
    //DataAdd(buff);
    sendBinaryData();
    //Task_Start();
}

//
// Установка в контроллер, нового положения по осям
//
// параметр "x" Положение в импульсах
// параметр "y" Положение в импульсах
// параметр "z" Положение в импульсах
void mk1Controller::deviceNewPosition(int x, int y, int z, int a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(x, y, z, a);
}

//
// Установка в контроллер, нового положения по осям в ммилиметрах
//
// параметр "x" в миллиметрах
// параметр "y" в миллиметрах
// параметр "z" в миллиметрах
// ReSharper disable once UnusedMember.Global
void mk1Controller::deviceNewPosition(double x, double y, double z, double a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(DeviceInfo::CalcPosPulse("X", x), DeviceInfo::CalcPosPulse("Y", y), DeviceInfo::CalcPosPulse("Z", z), DeviceInfo::CalcPosPulse("A", a));
}



double DeviceInfo::AxesX_PositionMM ()
{
    return (double)(AxesX_PositionPulse / (double) AxesX_PulsePerMm);
}


double DeviceInfo::AxesY_PositionMM ()
{
    return (double)(AxesY_PositionPulse / (double) AxesY_PulsePerMm);
}


double DeviceInfo::AxesZ_PositionMM ()
{
    return (double)(AxesZ_PositionPulse / (double) AxesZ_PulsePerMm);
}


double DeviceInfo::AxesA_PositionMM ()
{
    return (double)(AxesA_PositionPulse / (double) AxesA_PulsePerMm);
}

//
// Вычисление положения в импульсах, при указании оси, и положения в миллиметрах
//
// параметр "axes" имя оси X,Y,Z
// параметр "posMm" положение в мм
// возвращаемый Количество импульсов
int DeviceInfo::CalcPosPulse(QString axes, double posMm)
{
    if (axes == "X") {
        return (int)(posMm * (double)AxesX_PulsePerMm);
    }

    if (axes == "Y") {
        return (int)(posMm * (double)AxesY_PulsePerMm);
    }

    if (axes == "Z") {
        return (int)(posMm * (double)AxesZ_PulsePerMm);
    }

    if (axes == "A") {
        return (int)(posMm * (double)AxesA_PulsePerMm);
    }

    return 0;
}


// You MUST declare it, because of static field
byte BinaryData::readBuf[BUFFER_SIZE];
byte BinaryData::writeBuf[BUFFER_SIZE];

//
// Данная команда пока непонятна....
//
// параметр "byte05"
//
void BinaryData::packC0(byte byte05, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xC0;
    writeBuf[5] = byte05;

    if (send == true) {
        sendBinaryData();
    }
}


void BinaryData::setByte(short offset, byte data)
{
    if (offset >= BUFFER_SIZE || offset < 0) {
        return;
    }

    writeBuf[offset] = data;
}


byte BinaryData::getByte(short offset)
{
    if (offset >= BUFFER_SIZE || offset < 0) {
        return 0;
    }

    return readBuf[offset];
}



void BinaryData::cleanBuf(byte *m)
{
    memset(m, 0x0, BUFFER_SIZE);
}


//region Послания данных в контроллер

//
// Посылка в контроллер бинарных данных
//
// параметр "data" Набор данных
// параметр "checkBuffSize" Проверять ли размер доступного буффера контроллера
void BinaryData::sendBinaryData(bool checkBuffSize)
{
    if (checkBuffSize) {
        if (DeviceInfo::FreebuffSize < 2) {
            //тут нужно зависнуть пока буфер не освободится
        }

        //TODO: перед выполнением проверять буфер на занятость....
    }

    // ReSharper disable once SuggestVarOrType_BuiltInTypes
    // ReSharper disable once RedundantAssignment
    //     int bytesWritten = BUFFER_SIZE;

    if (!DeviceInfo::DEMO_DEVICE) {
        //         _error_code = _usb->write(rawData);//, 2000, bytesWritten);
        if (handle != 0) {
            int transferred = 0;
            int e = libusb_bulk_transfer(handle, BULK_ENDPOINT_IN, writeBuf, BUFFER_SIZE, &transferred, 30); // timeout 30 msecons

            if(e == 0 && transferred == BUFFER_SIZE) {
                qDebug() << "Write successful!";
                qDebug() << "Sent " << transferred << " bytes ";
            } else {
                qDebug() << "Error in write! e = " << e << " and transferred = " << transferred;
            }
        } else {
            qDebug() << "Device is not connected, send data error!";
        }

    } else {
        //TODO: добавить посылку виртуальному
    }
}

//
// Управление работой шпинделя
//
// параметр "spindelON" Вкл/Выключен
// параметр "numShimChanel" номер канала 1,2, или 3
// параметр "ts" Тип сигнала
// параметр "SpeedShim" Значение определяющее форму сигнала
//
void BinaryData::packB5(bool spindelON, int numShimChanel, TypeSignal ts, int SpeedShim, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xb5;
    writeBuf[4] = 0x80;


    if (spindelON) {
        writeBuf[5] = 0x02;
    } else {
        writeBuf[5] = 0x01;
    }

    writeBuf[6] = 0x01; //х.з.

    switch (numShimChanel) {
        case 2: {
            writeBuf[8] = 0x02;
            break;
        }

        case 3: {
            writeBuf[8] = 0x03;
            break;
        }

        default: {
            writeBuf[8] = 0x00; //доступен только 2 и 3 канал, остальные не подходят....
            break;
        }
    }


    switch (ts) {
        case Hz: { // TypeSignal.Hz: {
            writeBuf[9] = 0x01;
            break;
        }

        case  RC: { //TypeSignal.RC: {
            writeBuf[9] = 0x02;
            break;
        }

        default: {
            writeBuf[9] = 0x00;
            break;
        }
    }

    int itmp = SpeedShim;
    writeBuf[10] = (byte)(itmp);
    writeBuf[11] = (byte)(itmp >> 8);
    writeBuf[12] = (byte)(itmp >> 16);


    if (send == true) {
        sendBinaryData();
    }

    //writeBuf[10] = 0xff;
    //writeBuf[11] = 0xff;
    //writeBuf[12] = 0x04;
}

//
// Аварийная остановка
//
//
void BinaryData::packAA(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xaa;
    writeBuf[4] = 0x80;

    if (send == true) {
        sendBinaryData();
    }
}

//
// Установка в контроллер новых координат, без движения
//
// параметр "x"
// параметр "y"
// параметр "z"
//
void BinaryData::packC8(int x, int y, int z, int a, bool send)
{
    int newPosX = x;
    int newPosY = y;
    int newPosZ = z;
    int newPosA = a;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xc8;

    //сколько импульсов сделать
    writeBuf[6] = (byte)(newPosX);
    writeBuf[7] = (byte)(newPosX >> 8);
    writeBuf[8] = (byte)(newPosX >> 16);
    writeBuf[9] = (byte)(newPosX >> 24);

    //сколько импульсов сделать
    writeBuf[10] = (byte)(newPosY);
    writeBuf[11] = (byte)(newPosY >> 8);
    writeBuf[12] = (byte)(newPosY >> 16);
    writeBuf[13] = (byte)(newPosY >> 24);

    //сколько импульсов сделать
    writeBuf[14] = (byte)(newPosZ);
    writeBuf[15] = (byte)(newPosZ >> 8);
    writeBuf[16] = (byte)(newPosZ >> 16);
    writeBuf[17] = (byte)(newPosZ >> 24);

    //сколько импульсов сделать
    writeBuf[18] = (byte)(newPosA);
    writeBuf[19] = (byte)(newPosA >> 8);
    writeBuf[20] = (byte)(newPosA >> 16);
    writeBuf[21] = (byte)(newPosA >> 24);

    if (send == true) {
        sendBinaryData();
    }
}

//
// Проверка длины инструмента, или прощупывание
//
//
void BinaryData::packD2(int speed, double returnDistance, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xd2;

    int inewSpd = 0;

    if (speed != 0) {
        double dnewSpd = (1800 / (double)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //скорость
    writeBuf[43] = (byte)(inewSpd);
    writeBuf[44] = (byte)(inewSpd >> 8);
    writeBuf[45] = (byte)(inewSpd >> 16);


    //х.з.
    writeBuf[46] = 0x10;

    //
    int inewReturn = (int)(returnDistance * (double)DeviceInfo::AxesZ_PulsePerMm);

    //растояние возврата
    writeBuf[50] = (byte)(inewReturn);
    writeBuf[51] = (byte)(inewReturn >> 8);
    writeBuf[52] = (byte)(inewReturn >> 16);

    //х.з.
    writeBuf[55] = 0x12;
    writeBuf[56] = 0x7A;

    if (send == true) {
        sendBinaryData();
    }
}


//
// Запуск движения без остановки (и остановка)
//
// параметр "direction" Направление по осям в байте
// параметр "speed" Скорость движения
//
void BinaryData::packBE(byte direction, int speed, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbe;
    writeBuf[4] = 0x80;
    writeBuf[6] = direction;

    int inewSpd = 0;

    if (speed != 0) {
        double dnewSpd = (1800 / (double)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //скорость
    writeBuf[10] = (byte)(inewSpd);
    writeBuf[11] = (byte)(inewSpd >> 8);
    writeBuf[12] = (byte)(inewSpd >> 16);

    if (send == true) {
        sendBinaryData();
    }
}



////
//// используется временно для прощупывания
////
////
//void  BinaryData::packCA()
//{
//  cleanBuf(writeBuf);

//    writeBuf[0] = 0xca;

//    writeBuf[5] = 0xB9;
//    writeBuf[14] = 0xD0;
//    writeBuf[15] = 0x07;
//    writeBuf[43] = 0x10;
//    writeBuf[44] = 0x0E;

//    return buf;
//}



void BinaryData::pack9E(byte value, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9e;
    writeBuf[5] = value;

    if (send == true) {
        sendBinaryData();
    }
}

//
// Установка ограничения скорости
//
// параметр "speedLimitX" Максимальная скорость по оси X
// параметр "speedLimitY" Максимальная скорость по оси Y
// параметр "speedLimitZ" Максимальная скорость по оси Z
// параметр "speedLimitA" Максимальная скорость по оси A
//
void BinaryData::packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbf;
    writeBuf[4] = 0x80; //TODO: непонятный байт


    double dnewSpdX = (3600 / (double)speedLimitX) * 1000;
    int inewSpdX = (int)dnewSpdX;

    writeBuf[7] = (byte)(inewSpdX);
    writeBuf[8] = (byte)(inewSpdX >> 8);
    writeBuf[9] = (byte)(inewSpdX >> 16);
    writeBuf[10] = (byte)(inewSpdX >> 24);

    double dnewSpdY = (3600 / (double)speedLimitY) * 1000;
    int inewSpdY = (int)dnewSpdY;

    writeBuf[11] = (byte)(inewSpdY);
    writeBuf[12] = (byte)(inewSpdY >> 8);
    writeBuf[13] = (byte)(inewSpdY >> 16);
    writeBuf[14] = (byte)(inewSpdY >> 24);

    double dnewSpdZ = (3600 / (double)speedLimitZ) * 1000;
    int inewSpdZ = (int)dnewSpdZ;

    writeBuf[15] = (byte)(inewSpdZ);
    writeBuf[16] = (byte)(inewSpdZ >> 8);
    writeBuf[17] = (byte)(inewSpdZ >> 16);
    writeBuf[18] = (byte)(inewSpdZ >> 24);

    double dnewSpdA = (3600 / (double)speedLimitA) * 1000;
    int inewSpdA = (int)dnewSpdA;

    writeBuf[19] = (byte)(inewSpdA);
    writeBuf[20] = (byte)(inewSpdA >> 8);
    writeBuf[21] = (byte)(inewSpdA >> 16);
    writeBuf[22] = (byte)(inewSpdA >> 24);

    if (send == true) {
        sendBinaryData();
    }
}

//
// НЕИЗВЕСТНАЯ КОМАНДА
//
//
// void BinaryData::packC0(bool send)
// {
//     cleanBuf();
//
//     buf[0] = 0xc0;
// }

//
// Движение в указанную точку
//
// параметр "_posX" положение X в импульсах
// параметр "_posY" положение Y в импульсах
// параметр "_posZ" положение Z в импульсах
// параметр "_speed" скорость мм/минуту
// параметр "_NumberInstruction" Номер данной инструкции
// возвращаемый набор данных для посылки
void BinaryData::packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, bool send)
{
    int newPosX = _posX;
    int newPosY = _posY;
    int newPosZ = _posZ;
    int newPosA = _posA;
    int newInst = _NumberInstruction;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xca;

    //запись номера инструкции
    writeBuf[1] = (byte)(newInst);
    writeBuf[2] = (byte)(newInst >> 8);
    writeBuf[3] = (byte)(newInst >> 16);
    writeBuf[4] = (byte)(newInst >> 24);

    writeBuf[5] = 0x39; //TODO: непонятный байт

    //сколько импульсов сделать
    writeBuf[6] = (byte)(newPosX);
    writeBuf[7] = (byte)(newPosX >> 8);
    writeBuf[8] = (byte)(newPosX >> 16);
    writeBuf[9] = (byte)(newPosX >> 24);

    //сколько импульсов сделать
    writeBuf[10] = (byte)(newPosY);
    writeBuf[11] = (byte)(newPosY >> 8);
    writeBuf[12] = (byte)(newPosY >> 16);
    writeBuf[13] = (byte)(newPosY >> 24);

    //сколько импульсов сделать
    writeBuf[14] = (byte)(newPosZ);
    writeBuf[15] = (byte)(newPosZ >> 8);
    writeBuf[16] = (byte)(newPosZ >> 16);
    writeBuf[17] = (byte)(newPosZ >> 24);

    //сколько импульсов сделать
    writeBuf[18] = (byte)(newPosA);
    writeBuf[19] = (byte)(newPosA >> 8);
    writeBuf[20] = (byte)(newPosA >> 16);
    writeBuf[21] = (byte)(newPosA >> 24);

    int inewSpd = 2328; //TODO: скорость по умолчанию

    if (_speed != 0) {
        double dnewSpd = (1800 / (double)_speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //скорость ось х
    writeBuf[43] = (byte)(inewSpd);
    writeBuf[44] = (byte)(inewSpd >> 8);
    writeBuf[45] = (byte)(inewSpd >> 16);

    writeBuf[54] = 0x40;  //TODO: непонятный байт

    if (send == true) {
        sendBinaryData();
    }
}

//
// Завершение выполнения всех операций
//
//
void BinaryData::packFF(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xff;

    if (send == true) {
        sendBinaryData();
    }
}

//
// НЕИЗВЕСТНАЯ КОМАНДА
//
//
void BinaryData::pack9D(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9d;

    if (send == true) {
        sendBinaryData();
    }
}

//void BinaryData::GetPack07()
//{
//
// cleanBuf();
//    writeBuf[0] = 0x9e;
//    writeBuf[5] = 0x02;

//    return buf;
//}


