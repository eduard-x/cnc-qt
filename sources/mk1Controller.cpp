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

#include <QDebug>
#include <QIODevice>

#include "includes/mk1Controller.h"

#include <QString>

using namespace std;

/******************************************************************************
** mk1Controller
*/


short mk1Settings::FreebuffSize = 0;

int mk1Settings::NumberCompleatedInstruction = 0;


axis::axis()
{
    acceleration = 15.0;
    limitMax = false;
    limitMin = false;
    pulsePerMm = 400;
    actualPosPulses = 0;
    wrong = false;
}


float axis::posMm()
{
    return (float)(actualPosPulses / (float) pulsePerMm);
}


int axis::posPulse(float posMm)
{
    return (int)(posMm * (float)pulsePerMm);
}


int mk1Settings::spindle_MoveSpeed = 0;
bool mk1Settings::spindle_Enable = false;

bool mk1Settings::Estop = false;

bool mk1Settings::DEMO_DEVICE = false;


// static
libusb_device_handle *BinaryData::handle = 0;

libusb_hotplug_callback_handle mk1Controller::hotplug[2];

libusb_device_descriptor BinaryData::desc = {0};

QString mk1Controller::devDescriptor;


static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{

    int rc;
    //     QString descrStr;
    //
    //     //     struct libusb_device_descriptor desc;

    //     mk1Controller::getDeviceInfo(dev);

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

    mk1Controller::getDeviceInfo();

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

    coordList << "X" << "Y" << "Z" << "A";

    devConnected = false;

    int class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

    rc = libusb_init (NULL);

    if (rc < 0) {
        qDebug() << QString("failed to initialise libusb: %1").arg(libusb_error_name(rc));
    }

    hotplugThread = 0;

    readThread = 0;

    bool devAlreadyConnected = false;

    handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id); //these are vendorID and productID I found for my usb device

    if(handle == NULL) {
        qDebug() << "Device not connected" << endl;
    } else {
        qDebug() << "Device connected" << handle << endl;

#ifdef __linux__

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

        devAlreadyConnected = true;
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

        if (devAlreadyConnected == true) {
            if (handle != 0) {
                libusb_close(handle);
            }

            handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id); //these are vendorID and productID I found for my usb device
            int e = libusb_set_configuration (handle, 1);

            if(e < 0) {
                qDebug() << "Cannot Claim Interface";
                libusb_close(handle);
                handle = 0;
            } else    {
                qDebug() << "Claimed Interface";
            }

            getDeviceInfo();
        }
    }

    settingsFile = new QSettings("KarboSoft", "CNC-Qt");
}


int mk1Controller::getDeviceInfo()
{
    int rc;
    QString descrStr;

    libusb_device *dev = libusb_get_device(handle);

    rc = libusb_get_device_descriptor(dev, &desc);

    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "Error getting device descriptor";
        handle = 0;
        return -1;
    }

    // get device descriptor
    descrStr = QString().sprintf("VendorID: 0x%x ProductID: 0x%x\n\n", desc.idVendor,  desc.idProduct);
    descrStr += QString().sprintf("Number of possible configurations: %d\n", desc.bNumConfigurations);
    descrStr += QString().sprintf("Device Class: %d\n\n", desc.bDeviceClass);

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

    libusb_free_config_descriptor(config);

    devDescriptor = descrStr;

    return 0;
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
}


void mk1Controller::readNewData()
{
    //     qDebug() << "new data from usb";
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
// load settings
//
void mk1Controller::loadSettings()
{
    bool res;

    settingsFile->beginGroup("mk1");

    for (int c = 0; c < coordList.count(); c++) {
        int i = settingsFile->value("Pulse" + coordList.at(c), 400).toInt( &res);

        if (res == true) {
            coord[c].pulsePerMm = i;
        }
    }


    for (int c = 0; c < coordList.count(); c++) {
        float f = settingsFile->value("Accel" + coordList.at(c), 15).toFloat( &res);

        if (res == true) {
            coord[c].acceleration = f;
        }
    }

    for (int c = 0; c < coordList.count(); c++) {
        float f = settingsFile->value("StartVelo" + coordList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].minVelo = f;
        }
    }

    for (int c = 0; c < coordList.count(); c++) {
        float f = settingsFile->value("EndVelo" + coordList.at(c), 400).toFloat( &res);

        if (res == true) {
            coord[c].maxVelo = f;
        }
    }

    settingsFile->endGroup();
}

//
// save settings
//
void mk1Controller::saveSettings()
{
    settingsFile->beginGroup("mk1");

    for (int c = 0; c < coordList.count(); c++) {
        settingsFile->setValue("Pulse" + coordList.at(c), coord[c].pulsePerMm);
    }

    for (int c = 0; c < coordList.count(); c++) {
        settingsFile->setValue("Accel" + coordList.at(c), (double)coord[c].acceleration);
    }

    for (int c = 0; c < coordList.count(); c++) {
        settingsFile->setValue("StartVelo" + coordList.at(c), (double)coord[c].minVelo);
    }

    for (int c = 0; c < coordList.count(); c++) {
        settingsFile->setValue("EndVelo" + coordList.at(c), (double)coord[c].maxVelo);
    }

    settingsFile->endGroup();

    settingsFile->sync();

    sendSettings();
}


// send settings to mk
void mk1Controller::sendSettings()
{

    packD3();
    packAB();

    pack9F(); // set pulses per mm

    packA0(); // set acceleration
    packA1(); // set allowed limits

    packBF(coord[X].maxVelo, coord[Y].maxVelo, coord[Z].maxVelo, coord[A].maxVelo); // set max velocities

    packB5(false); // spindle off
    packB6(); // unknown

    packC2(); // unknown
    pack9D();

    pack9E(0x80);
}



// info about connection
//
bool mk1Controller::isConnected()
{
    return (handle != 0);
}

//
// velocity of spindle
//
int mk1Controller::spindleMoveSpeed()
{
    return spindle_MoveSpeed;
}

//
// current instruction number
//
long mk1Controller::numberCompleatedInstructions()
{
    return NumberCompleatedInstruction;
}

//
// splindle is on?
//
bool mk1Controller::isSpindelOn()
{
    return spindle_Enable;
}

//
// was stopped because of emergency?
//
bool mk1Controller::isEmergencyStopOn()
{
    return Estop;
}

//
// check connection
//
bool mk1Controller::testAllowActions()
{
    if (!isConnected()) {
        return false;
    }

    return true;
}

//
// free buffer size
//
int mk1Controller::availableBufferSize()
{
    return FreebuffSize;
}


//
// parse binary data
//
void mk1Controller::parseBinaryInfo()
{
    FreebuffSize = readBuf[1];
    spindle_MoveSpeed = (int)(((/*(readBuf[23] << 24) + (readBuf[22] << 16) +*/ (readBuf[21] << 8) + (readBuf[20]))) / 2.1);

    coord[X].actualPosPulses = ((readBuf[27] << 24) + (readBuf[26] << 16) + (readBuf[25] << 8) + (readBuf[24]));
    coord[Y].actualPosPulses = ((readBuf[31] << 24) + (readBuf[30] << 16) + (readBuf[29] << 8) + (readBuf[28]));
    coord[Z].actualPosPulses = ((readBuf[35] << 24) + (readBuf[34] << 16) + (readBuf[33] << 8) + (readBuf[32]));

    coord[X].limitMax = (readBuf[15] & (1 << 0)) != 0;
    coord[X].limitMax = (readBuf[15] & (1 << 1)) != 0;
    coord[Y].limitMax = (readBuf[15] & (1 << 2)) != 0;
    coord[Y].limitMax = (readBuf[15] & (1 << 3)) != 0;
    coord[Z].limitMax = (readBuf[15] & (1 << 4)) != 0;
    coord[Z].limitMax = (readBuf[15] & (1 << 5)) != 0;
    coord[A].limitMax = (readBuf[15] & (1 << 6)) != 0;
    coord[A].limitMax = (readBuf[15] & (1 << 7)) != 0;

    NumberCompleatedInstruction = ((readBuf[9] << 24) + (readBuf[8] << 16) + (readBuf[7] << 8) + (readBuf[6]));

    byte bb19 = readBuf[19];

    spindle_Enable = (bb19 & (1 << 0)) ? true : false;

    byte bb14 = readBuf[14];
    Estop = (bb14 & (1 << 7)) ? true : false;

    emit newDataFromMK1Controller();
}


// send number of message to main class
void mk1Controller::ADDMessage(int num)
{
    emit Message(num);
}


//
// enable spindle
//
void mk1Controller::spindleON()
{
    packB5(true);
}

//
// disable spindle
//
void mk1Controller::spindleOFF()
{
    packB5(false);
}

//
// send emergency stop
//
void mk1Controller::emergyStop()
{
    packAA();
}

//
// manual moving
//
// input qstring parameter is "+", "0" or "-"
// speed: velocity
void mk1Controller::startManualMove(QString x, QString y, QString z, QString a, int speed)
{
    if (!isConnected()) {
        return;
    }

    //if (!IsFreeToTask)
    //{
    //    return;
    //}

    byte axesDirection = 0x00; // = new SuperByte(0x00);

    // set the bits
    if (x == "-") {
        axesDirection |= (1 << 0);
    }

    if (x == "+") {
        axesDirection |= (1 << 1);
    }

    if (y == "-") {
        axesDirection |= (1 << 2);
    }

    if (y == "+") {
        axesDirection |= (1 << 3);
    }

    if (z == "-") {
        axesDirection |= (1 << 4);
    }

    if (z == "+") {
        axesDirection |= (1 << 5);
    }

    if (a == "-") {
        axesDirection |= (1 << 6);
    }

    if (a == "+") {
        axesDirection |= (1 << 7);
    }

    //DataClear();
    //DataAdd(BinaryData.packBE(axesDirection.valueByte, speed));
    packBE(axesDirection, speed);
    //Task_Start();
}


void mk1Controller::stopManualMove()
{
    if (!isConnected()) {
        return;
    }

    //     QByteArray buff = BinaryData.packBE(0x00, 0);

    packBE(0x00, 0, false);
    setByte(22, 0x01);
    //TODO: unknown
    //     buff[22] = 0x01;

    //DataClear();
    //DataAdd(buff);
    sendBinaryData();
    //Task_Start();
}


//
// new position
//
// input paramereters in pulses
void mk1Controller::deviceNewPosition(int x, int y, int z, int a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(x, y, z, a);
}


//
// new position in mm
//
//  input paramereters in mm
void mk1Controller::deviceNewPosition(float x, float y, float z, float a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(coord[X].posPulse(x), coord[Y].posPulse(y), coord[Z].posPulse(z), coord[A].posPulse(a));
}


// You MUST declare it, because of static field
byte BinaryData::readBuf[BUFFER_SIZE];
byte BinaryData::writeBuf[BUFFER_SIZE];



void BinaryData::setByte(byte offset, byte data)
{
    if (offset >= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset] = data;
}


byte BinaryData::getByte(byte offset)
{
    if (offset >= BUFFER_SIZE) {
        return 0;
    }

    return readBuf[offset];
}


void BinaryData::cleanBuf(byte *m)
{
    memset(m, 0x0, BUFFER_SIZE);
}


void BinaryData::packFourBytes(byte offset, int val)
{
    if ((offset + 3)>= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset + 0] = (byte)val;
    writeBuf[offset + 1] = (byte)(val >> 8);
    writeBuf[offset + 2] = (byte)(val >> 16);
    writeBuf[offset + 3] = (byte)(val >> 24);
}

//
// send the data to controller
//
// "checkBuffSize" check the buffer
void BinaryData::sendBinaryData(bool checkBuffSize)
{
    if (checkBuffSize) {
        if (FreebuffSize < 2) {
            //тут нужно зависнуть пока буфер не освободится
        }

        //TODO: check buffer....
    }

    if (!DEMO_DEVICE) {
        //         _error_code = _usb->write(rawData);//, 2000, bytesWritten);
        if (handle != 0) {
            int transferred = 0;
            int e = libusb_bulk_transfer(handle, BULK_ENDPOINT_IN, writeBuf, BUFFER_SIZE, &transferred, 30); // timeout 30 msecons

            if(e == 0 && transferred == BUFFER_SIZE) {
                //                 qDebug() << "Write successful!";
                //                 qDebug() << "Sent " << transferred << " bytes ";
            } else {
                qDebug() << "Error in write! e = " << e << " and transferred = " << transferred;
            }
        } else {
            qDebug() << "Device is not connected, send data error!";
        }

    } else {
        //TODO: send data to virtual
    }
}


//
// UNKNOWN COMMAND
//
void BinaryData::pack9D(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9d;

    writeBuf[4] = 0x80; //unknown
    writeBuf[5] = 0x01; //unknown

    if (send == true) {
        sendBinaryData();
    }
}


void BinaryData::pack9E(byte value, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9e;

    if (value == 0x80) { // if settings
        writeBuf[4] = value;
    } else {
        // bit mask for begin / end of moving
        // 01 or 05 ? begin of moving
        // 0x2: end of moving
        writeBuf[5] = value;
    }

    if (send == true) {
        sendBinaryData();
    }
}


// settings
// impulses per mm
void BinaryData::pack9F( bool send)
{
    cleanBuf(writeBuf);
    int _impX = coord[X].pulsePerMm;
    int _impY = coord[Y].pulsePerMm;
    int _impZ = coord[Z].pulsePerMm;
    int _impA = coord[A].pulsePerMm;

    writeBuf[0] = 0x9f;
    writeBuf[4] = 0x80; //TODO:unknown
    writeBuf[5] = 0xb1; //TODO:unknown

    packFourBytes(6, _impX);
    //     writeBuf[6] = (byte)(_impX);
    //     writeBuf[7] = (byte)(_impX >> 8);
    //     writeBuf[8] = (byte)(_impX >> 16);
    //     writeBuf[9] = (byte)(_impX >> 24);

    packFourBytes(10, _impY);
    //     writeBuf[10] = (byte)(_impY);
    //     writeBuf[11] = (byte)(_impY >> 8);
    //     writeBuf[12] = (byte)(_impY >> 16);
    //     writeBuf[13] = (byte)(_impY >> 24);

    packFourBytes(14, _impZ);
    //     writeBuf[14] = (byte)(_impZ);
    //     writeBuf[15] = (byte)(_impZ >> 8);
    //     writeBuf[16] = (byte)(_impZ >> 16);
    //     writeBuf[17] = (byte)(_impZ >> 24);

    packFourBytes(18, _impA);
    //     writeBuf[18] = (byte)(_impA);
    //     writeBuf[19] = (byte)(_impA >> 8);
    //     writeBuf[20] = (byte)(_impA >> 16);
    //     writeBuf[21] = (byte)(_impA >> 24);

    if (send == true) {
        sendBinaryData();
    }
}


// acceleration settings
void BinaryData::packA0(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa0;
    writeBuf[4] = 0x80;

    writeBuf[5] = 0x12;

    int AccelX = 4056;

    if (coord[X].acceleration > 0) {
        AccelX = 4056 * coord[X].pulsePerMm / sqrt(coord[X].acceleration);
    }

    packFourBytes(6, AccelX);
    //     writeBuf[6] = (byte)(AccelX);
    //     writeBuf[7] = (byte)(AccelX >> 8);
    //     writeBuf[8] = (byte)(AccelX >> 16);
    //     writeBuf[9] = (byte)(AccelX >> 24);

    int AccelY = 4056;

    if (coord[Y].acceleration > 0) {
        AccelY = 4056 * coord[Y].pulsePerMm / sqrt(coord[Y].acceleration);
    }

    packFourBytes(10, AccelY);
    //     writeBuf[10] = (byte)(AccelY);
    //     writeBuf[11] = (byte)(AccelY >> 8);
    //     writeBuf[12] = (byte)(AccelY >> 16);
    //     writeBuf[13] = (byte)(AccelY >> 24);

    int AccelZ = 4056;

    if (coord[Z].acceleration > 0) {
        AccelZ = 4056 * coord[Z].pulsePerMm / sqrt(coord[Z].acceleration);
    }

    packFourBytes(14, AccelZ);
    //     writeBuf[14] = (byte)(AccelZ);
    //     writeBuf[15] = (byte)(AccelZ >> 8);
    //     writeBuf[16] = (byte)(AccelZ >> 16);
    //     writeBuf[17] = (byte)(AccelZ >> 24);

    int AccelA = 4056;

    if (coord[A].acceleration > 0) {
        AccelA = 4056 * coord[A].pulsePerMm / sqrt(coord[A].acceleration);
    }

    packFourBytes(18, AccelA);
    //     writeBuf[18] = (byte)(AccelA);
    //     writeBuf[19] = (byte)(AccelA >> 8);
    //     writeBuf[20] = (byte)(AccelA >> 16);
    //     writeBuf[21] = (byte)(AccelA >> 24);

    writeBuf[42] = 0x60;// unknown byte
    writeBuf[43] = 0x09;// unknown byte

    writeBuf[46] = 0x08;// unknown byte

    // reverse of axis.: 0xff no reverse, 0xfe axis x, 0xfd axis y, 0xfb axis z
    writeBuf[57] = 0xff;// unknown byte
    writeBuf[58] = 0x01;// unknown byte

    // reverse motor steps, bitmask: 0 no inverting, 1 invert step X, 2 invert step Y, 4 invert step Z
    writeBuf[59] = 0x00; //
    writeBuf[60] = 0x00; //

    if (send == true) {
        sendBinaryData();
    }
}


// unknown settings
void BinaryData::packA1( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa1;

    writeBuf[4] = 0x80;

    // allow limits: bit 7 a+; bit 6 a-, bit 5 z+, bit 4 z-, bit 3 y+, bit 2 y-, bit 1 x+, bit 0 x-
    writeBuf[42] = 0xff;
    writeBuf[48] = 0xff; // unknown

    if (send == true) {
        sendBinaryData();
    }
}

//
// emergency STOP
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


// unknown settings
void BinaryData::packAB( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xab;

    writeBuf[4] = 0x80;

    if (send == true) {
        sendBinaryData();
    }
}


//
// spindle commands
//
// spindleON: on/off
// numShimChanel chan number 1,2, or 3
// ts signal type
// SpeedShim signal form
//
void BinaryData::packB5(bool spindleON, int numShimChanel, TypeSignal ts, int SpeedShim, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xb5;
    writeBuf[4] = 0x80;


    if (spindleON) {
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
            writeBuf[8] = 0x00; //only with 2 and 3 channel....
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

//     int itmp = SpeedShim;
    packFourBytes(10, SpeedShim);
    //     writeBuf[10] = (byte)(itmp);
    //     writeBuf[11] = (byte)(itmp >> 8);
    //     writeBuf[12] = (byte)(itmp >> 16);
    //      writeBuf[13] = (byte)(itmp >> 24);


    if (send == true) {
        sendBinaryData();
    }
}


// unknown settings
void BinaryData::packB6( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xb6;

    writeBuf[4] = 0x80;

    writeBuf[5] = 0x01; //TODO:unknown
    writeBuf[6] = 0x02; //TODO:unknown
    writeBuf[7] = 0x01; //TODO:unknown
    writeBuf[8] = 0x03; //TODO:unknown

    if (send == true) {
        sendBinaryData();
    }
}


//
// moving without stop (and stop)
//
// direction axes
// speed
//
void BinaryData::packBE(byte direction, int speed, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbe;
    writeBuf[4] = 0x80;
    writeBuf[6] = direction;

    int inewSpd = 0;

    if (speed != 0) {
        float dnewSpd = (1800 / (float)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //velocity
    packFourBytes(10, inewSpd);
    /*    writeBuf[10] = (byte)(inewSpd);
        writeBuf[11] = (byte)(inewSpd >> 8);
        writeBuf[12] = (byte)(inewSpd >> 16);
        writeBuf[13] = (byte)(inewSpd >> 24);  */

    if (send == true) {
        sendBinaryData();
    }
}



//
// set velocity limit
//
void BinaryData::packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbf;
    writeBuf[4] = 0x80; //TODO:unknown

    float dnewSpdX  = 3600; // 3584?

    if (speedLimitX != 0) {
        dnewSpdX = (3600 / (float)speedLimitX) * 1000;
    }

//     int inewSpdX = (int)dnewSpdX;

    packFourBytes(7, (int)dnewSpdX);
    //     writeBuf[7] = (byte)(inewSpdX);
    //     writeBuf[8] = (byte)(inewSpdX >> 8);
    //     writeBuf[9] = (byte)(inewSpdX >> 16);
    //     writeBuf[10] = (byte)(inewSpdX >> 24);

    float dnewSpdY = 3600;

    if (speedLimitY != 0) {
        dnewSpdY = (3600 / (float)speedLimitY) * 1000;
    }

//     int inewSpdY = (int)dnewSpdY;

    packFourBytes(11, (int)dnewSpdY);
    //     writeBuf[11] = (byte)(inewSpdY);
    //     writeBuf[12] = (byte)(inewSpdY >> 8);
    //     writeBuf[13] = (byte)(inewSpdY >> 16);
    //     writeBuf[14] = (byte)(inewSpdY >> 24);

    float dnewSpdZ = 3600;

    if (speedLimitZ != 0) {
        dnewSpdZ = (3600 / (float)speedLimitZ) * 1000;
    }

//     int inewSpdZ = (int)dnewSpdZ;

    packFourBytes(15, (int)dnewSpdZ);
    //     writeBuf[15] = (byte)(inewSpdZ);
    //     writeBuf[16] = (byte)(inewSpdZ >> 8);
    //     writeBuf[17] = (byte)(inewSpdZ >> 16);
    //     writeBuf[18] = (byte)(inewSpdZ >> 24);

    float dnewSpdA = 3600;

    if (speedLimitA != 0) {
        dnewSpdA = (3600 / (float)speedLimitA) * 1000;
    }

//     int inewSpdA = (int)dnewSpdA;

    packFourBytes(19, (int)dnewSpdA);
    //     writeBuf[19] = (byte)(inewSpdA);
    //     writeBuf[20] = (byte)(inewSpdA >> 8);
    //     writeBuf[21] = (byte)(inewSpdA >> 16);
    //     writeBuf[22] = (byte)(inewSpdA >> 24);

    if (send == true) {
        sendBinaryData();
    }
}


//
// command unknown....
//
// param "byte05"
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


// unknown settings
void BinaryData::packC2( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xc2;

    writeBuf[4] = 0x80;
    writeBuf[5] = 0x03;

    if (send == true) {
        sendBinaryData();
    }
}


//
// set the coordinates without moving
//
void BinaryData::packC8(int x, int y, int z, int a, bool send)
{
    //     int newPosX = x;
    //     int newPosY = y;
    //     int newPosZ = z;
    //     int newPosA = a;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xc8;

    //num of pulses
    packFourBytes(6, x);
    //     writeBuf[6] = (byte)(newPosX);
    //     writeBuf[7] = (byte)(newPosX >> 8);
    //     writeBuf[8] = (byte)(newPosX >> 16);
    //     writeBuf[9] = (byte)(newPosX >> 24);

    //num of pulses
    packFourBytes(10, y);
    //     writeBuf[10] = (byte)(newPosY);
    //     writeBuf[11] = (byte)(newPosY >> 8);
    //     writeBuf[12] = (byte)(newPosY >> 16);
    //     writeBuf[13] = (byte)(newPosY >> 24);

    //num of pulses
    packFourBytes(14, z);
    //     writeBuf[14] = (byte)(newPosZ);
    //     writeBuf[15] = (byte)(newPosZ >> 8);
    //     writeBuf[16] = (byte)(newPosZ >> 16);
    //     writeBuf[17] = (byte)(newPosZ >> 24);

    //num of pulses
    packFourBytes(18, a);
    //     writeBuf[18] = (byte)(newPosA);
    //     writeBuf[19] = (byte)(newPosA >> 8);
    //     writeBuf[20] = (byte)(newPosA >> 16);
    //     writeBuf[21] = (byte)(newPosA >> 24);

    if (send == true) {
        sendBinaryData();
    }
}


//
// moving to the point
//
void BinaryData::packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, bool send)
{
    int newPosX = _posX;
    int newPosY = _posY;
    int newPosZ = _posZ;
    int newPosA = _posA;
    int newInst = _NumberInstruction;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xca;

    //save the number instruction
    packFourBytes(1, newInst);
    //     writeBuf[1] = (byte)(newInst);
    //     writeBuf[2] = (byte)(newInst >> 8);
    //     writeBuf[3] = (byte)(newInst >> 16);
    //     writeBuf[4] = (byte)(newInst >> 24);

    writeBuf[5] = 0x39; //TODO: unnknown byte delay in µs?

    //how many pulses
    packFourBytes(6, newPosX);
    //     writeBuf[6] = (byte)(newPosX);
    //     writeBuf[7] = (byte)(newPosX >> 8);
    //     writeBuf[8] = (byte)(newPosX >> 16);
    //     writeBuf[9] = (byte)(newPosX >> 24);

    //how many pulses
    packFourBytes(10, newPosY);
    //     writeBuf[10] = (byte)(newPosY);
    //     writeBuf[11] = (byte)(newPosY >> 8);
    //     writeBuf[12] = (byte)(newPosY >> 16);
    //     writeBuf[13] = (byte)(newPosY >> 24);

    //how many pulses
    packFourBytes(14, newPosZ);
    //     writeBuf[14] = (byte)(newPosZ);
    //     writeBuf[15] = (byte)(newPosZ >> 8);
    //     writeBuf[16] = (byte)(newPosZ >> 16);
    //     writeBuf[17] = (byte)(newPosZ >> 24);

    //how many pulses
    packFourBytes(18, newPosA);
    //     writeBuf[18] = (byte)(newPosA);
    //     writeBuf[19] = (byte)(newPosA >> 8);
    //     writeBuf[20] = (byte)(newPosA >> 16);
    //     writeBuf[21] = (byte)(newPosA >> 24);

    int inewSpd = 2328; //TODO: default velocity

    if (_speed != 0) {
        float dnewSpd = (1800 / (float)_speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //axes xspeed
    packFourBytes(43, inewSpd);
    //     writeBuf[43] = (byte)(inewSpd);
    //     writeBuf[44] = (byte)(inewSpd >> 8);
    //     writeBuf[45] = (byte)(inewSpd >> 16);

    writeBuf[54] = 0x40;  //TODO: unknown byte

    if (send == true) {
        sendBinaryData();
    }
}


//
// check length of tool
//
void BinaryData::packD2(int speed, float returnDistance, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xd2;

    int inewSpd = 0;

    if (speed != 0) {
        float dnewSpd = (1800 / (float)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //velocity
    packFourBytes(43, inewSpd);
    //     writeBuf[43] = (byte)(inewSpd);
    //     writeBuf[44] = (byte)(inewSpd >> 8);
    //     writeBuf[45] = (byte)(inewSpd >> 16);


    //unknown
    writeBuf[46] = 0x10;

    //
    int inewReturn = (int)(returnDistance * (float)coord[Z].pulsePerMm);

    //return to position
    packFourBytes(50, inewReturn);
    //     writeBuf[50] = (byte)(inewReturn);
    //     writeBuf[51] = (byte)(inewReturn >> 8);
    //     writeBuf[52] = (byte)(inewReturn >> 16);

    //unknown
    writeBuf[55] = 0x12;
    writeBuf[56] = 0x7A;

    if (send == true) {
        sendBinaryData();
    }
}



// unknown settings
void BinaryData::packD3( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xd3;
    writeBuf[5] = 0x01;

    if (send == true) {
        sendBinaryData();
    }
}



//
// break of all operations
//
void BinaryData::packFF(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xff;

    if (send == true) {
        sendBinaryData();
    }
}


