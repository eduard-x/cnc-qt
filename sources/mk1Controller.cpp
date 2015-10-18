/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2015 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
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

int DeviceInfo::spindle_MoveSpeed = 0;
bool DeviceInfo::spindle_Enable = false;

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

    bool devAlreadyConnected = false;
    
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
	devAlreadyConnected = true;

// 	libusb_close(handle);
	
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
        
        if (devAlreadyConnected == true){
	    libusb_close(handle);
	
	    handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id); //these are vendorID and productID I found for my usb device
	    int e = libusb_set_configuration (handle, 1);

	    if(e < 0) {
		qDebug() << "Cannot Claim Interface";
		libusb_close(handle);
		handle = 0;
	    } else    {
		qDebug() << "Claimed Interface";
	    }
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
// load settings
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
// save settings
//
void mk1Controller::saveSettings()
{
    settingsFile->setValue("pulseX", DeviceInfo::AxesX_PulsePerMm);
    settingsFile->setValue("pulseY", DeviceInfo::AxesY_PulsePerMm);
    settingsFile->setValue("pulseZ", DeviceInfo::AxesZ_PulsePerMm);
    settingsFile->setValue("pulseA", DeviceInfo::AxesA_PulsePerMm);
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
    return DeviceInfo::spindle_MoveSpeed;
}

//
// current instruction number
//
long mk1Controller::numberComleatedInstructions()
{
    return DeviceInfo::NumberCompleatedInstruction;
}

//
// splindle is on?
//
bool mk1Controller::isSpindelOn()
{
    return DeviceInfo::spindle_Enable;
}

//
// was stopped because of emergency?
//
bool mk1Controller::isEmergencyStopOn()
{
    return DeviceInfo::Estop;
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
    return DeviceInfo::FreebuffSize;
}


//
// parse binary data
//
void mk1Controller::parseBinaryInfo()
{
    DeviceInfo::FreebuffSize = readBuf[1];
    DeviceInfo::spindle_MoveSpeed = (int)(((/*(readBuf[23] << 24) + (readBuf[22] << 16) +*/ (readBuf[21] << 8) + (readBuf[20]))) / 2.1);

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

    DeviceInfo::spindle_Enable = (bb19 & (1 << 0)) ? true : false;

    byte bb14 = readBuf[14];
    DeviceInfo::Estop = (bb14 & (1 << 7)) ? true : false;

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

    packC8(DeviceInfo::CalcPosPulse("X", x), DeviceInfo::CalcPosPulse("Y", y), DeviceInfo::CalcPosPulse("Z", z), DeviceInfo::CalcPosPulse("A", a));
}



float DeviceInfo::AxesX_PositionMM ()
{
    return (float)(AxesX_PositionPulse / (float) AxesX_PulsePerMm);
}


float DeviceInfo::AxesY_PositionMM ()
{
    return (float)(AxesY_PositionPulse / (float) AxesY_PulsePerMm);
}


float DeviceInfo::AxesZ_PositionMM ()
{
    return (float)(AxesZ_PositionPulse / (float) AxesZ_PulsePerMm);
}


float DeviceInfo::AxesA_PositionMM ()
{
    return (float)(AxesA_PositionPulse / (float) AxesA_PulsePerMm);
}

//
//calculation of position in pulses
//
// axes: name
// posMm po in mm
// return num of pulses
int DeviceInfo::CalcPosPulse(QString axes, float posMm)
{
    if (axes == "X") {
        return (int)(posMm * (float)AxesX_PulsePerMm);
    }

    if (axes == "Y") {
        return (int)(posMm * (float)AxesY_PulsePerMm);
    }

    if (axes == "Z") {
        return (int)(posMm * (float)AxesZ_PulsePerMm);
    }

    if (axes == "A") {
        return (int)(posMm * (float)AxesA_PulsePerMm);
    }

    return 0;
}


// You MUST declare it, because of static field
byte BinaryData::readBuf[BUFFER_SIZE];
byte BinaryData::writeBuf[BUFFER_SIZE];

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


//
// send the data to controller
//
// "checkBuffSize" check the buffer
void BinaryData::sendBinaryData(bool checkBuffSize)
{
    if (checkBuffSize) {
        if (DeviceInfo::FreebuffSize < 2) {
            //тут нужно зависнуть пока буфер не освободится
        }

        //TODO: check buffer....
    }

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
        //TODO: send data to virtual
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
// emergency STOP
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
// set the coordinates without moving
//
void BinaryData::packC8(int x, int y, int z, int a, bool send)
{
    int newPosX = x;
    int newPosY = y;
    int newPosZ = z;
    int newPosA = a;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xc8;

    //num of pulses
    writeBuf[6] = (byte)(newPosX);
    writeBuf[7] = (byte)(newPosX >> 8);
    writeBuf[8] = (byte)(newPosX >> 16);
    writeBuf[9] = (byte)(newPosX >> 24);

    //num of pulses
    writeBuf[10] = (byte)(newPosY);
    writeBuf[11] = (byte)(newPosY >> 8);
    writeBuf[12] = (byte)(newPosY >> 16);
    writeBuf[13] = (byte)(newPosY >> 24);

    //num of pulses
    writeBuf[14] = (byte)(newPosZ);
    writeBuf[15] = (byte)(newPosZ >> 8);
    writeBuf[16] = (byte)(newPosZ >> 16);
    writeBuf[17] = (byte)(newPosZ >> 24);

    //num of pulses
    writeBuf[18] = (byte)(newPosA);
    writeBuf[19] = (byte)(newPosA >> 8);
    writeBuf[20] = (byte)(newPosA >> 16);
    writeBuf[21] = (byte)(newPosA >> 24);

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
    writeBuf[43] = (byte)(inewSpd);
    writeBuf[44] = (byte)(inewSpd >> 8);
    writeBuf[45] = (byte)(inewSpd >> 16);


    //unknown
    writeBuf[46] = 0x10;

    //
    int inewReturn = (int)(returnDistance * (float)DeviceInfo::AxesZ_PulsePerMm);

    //return to position
    writeBuf[50] = (byte)(inewReturn);
    writeBuf[51] = (byte)(inewReturn >> 8);
    writeBuf[52] = (byte)(inewReturn >> 16);

    //unknown
    writeBuf[55] = 0x12;
    writeBuf[56] = 0x7A;

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
    writeBuf[10] = (byte)(inewSpd);
    writeBuf[11] = (byte)(inewSpd >> 8);
    writeBuf[12] = (byte)(inewSpd >> 16);

    if (send == true) {
        sendBinaryData();
    }
}



////
//// using temporary for scanning
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
// set velocity limit
//
void BinaryData::packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbf;
    writeBuf[4] = 0x80; //TODO:unknown


    float dnewSpdX = (3600 / (float)speedLimitX) * 1000;
    int inewSpdX = (int)dnewSpdX;

    writeBuf[7] = (byte)(inewSpdX);
    writeBuf[8] = (byte)(inewSpdX >> 8);
    writeBuf[9] = (byte)(inewSpdX >> 16);
    writeBuf[10] = (byte)(inewSpdX >> 24);

    float dnewSpdY = (3600 / (float)speedLimitY) * 1000;
    int inewSpdY = (int)dnewSpdY;

    writeBuf[11] = (byte)(inewSpdY);
    writeBuf[12] = (byte)(inewSpdY >> 8);
    writeBuf[13] = (byte)(inewSpdY >> 16);
    writeBuf[14] = (byte)(inewSpdY >> 24);

    float dnewSpdZ = (3600 / (float)speedLimitZ) * 1000;
    int inewSpdZ = (int)dnewSpdZ;

    writeBuf[15] = (byte)(inewSpdZ);
    writeBuf[16] = (byte)(inewSpdZ >> 8);
    writeBuf[17] = (byte)(inewSpdZ >> 16);
    writeBuf[18] = (byte)(inewSpdZ >> 24);

    float dnewSpdA = (3600 / (float)speedLimitA) * 1000;
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
// UNKNOWN COMMAND
//
//
// void BinaryData::packC0(bool send)
// {
//     cleanBuf();
//
//     buf[0] = 0xc0;
// }

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
    writeBuf[1] = (byte)(newInst);
    writeBuf[2] = (byte)(newInst >> 8);
    writeBuf[3] = (byte)(newInst >> 16);
    writeBuf[4] = (byte)(newInst >> 24);

    writeBuf[5] = 0x39; //TODO: unnknown byte

    //how many pulses
    writeBuf[6] = (byte)(newPosX);
    writeBuf[7] = (byte)(newPosX >> 8);
    writeBuf[8] = (byte)(newPosX >> 16);
    writeBuf[9] = (byte)(newPosX >> 24);

    //how many pulses
    writeBuf[10] = (byte)(newPosY);
    writeBuf[11] = (byte)(newPosY >> 8);
    writeBuf[12] = (byte)(newPosY >> 16);
    writeBuf[13] = (byte)(newPosY >> 24);

    //how many pulses
    writeBuf[14] = (byte)(newPosZ);
    writeBuf[15] = (byte)(newPosZ >> 8);
    writeBuf[16] = (byte)(newPosZ >> 16);
    writeBuf[17] = (byte)(newPosZ >> 24);

    //how many pulses
    writeBuf[18] = (byte)(newPosA);
    writeBuf[19] = (byte)(newPosA >> 8);
    writeBuf[20] = (byte)(newPosA >> 16);
    writeBuf[21] = (byte)(newPosA >> 24);

    int inewSpd = 2328; //TODO: default velocity

    if (_speed != 0) {
        float dnewSpd = (1800 / (float)_speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //axes xspeed
    writeBuf[43] = (byte)(inewSpd);
    writeBuf[44] = (byte)(inewSpd >> 8);
    writeBuf[45] = (byte)(inewSpd >> 16);

    writeBuf[54] = 0x40;  //TODO: unknown byte

    if (send == true) {
        sendBinaryData();
    }
}

//
// break of all operations
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
// UNKNOWN COMMAND
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


