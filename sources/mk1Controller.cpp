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
    acceleration = 50.0;
    actualLimitMax = false;
    actualLimitMin = false;
    enabled = true;
    backlash = 0.0;
    invertDirection = false;
    invertPulses = false;
    invLimitMax = false;
    invLimitMin = false;
    workAreaMax = 100.0;
    workAreaMin = -100.0;
    pulsePerMm = 200;
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


int mk1Settings::spindleMoveSpeed = 0;
bool mk1Settings::spindleEnabled = false;
bool mk1Settings::mistEnabled = false;
bool mk1Settings::fluidEnabled = false;
bool mk1Settings::setSettings = false;

bool mk1Settings::Estop = false;

bool mk1Settings::DEMO_DEVICE = false;


// static
libusb_device_handle *mk1Data::handle = 0;

libusb_hotplug_callback_handle mk1Controller::hotplug[2];

libusb_device_descriptor mk1Data::desc = {0};

QString mk1Controller::devDescriptor;


static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{

    int rc;

    qDebug() << "Device to attache" << mk1Controller::handle;

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

    axisList << "X" << "Y" << "Z" << "A";

    devConnected = false;

    int class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

    rc = libusb_init (NULL);

    if (rc < 0) {
        qDebug() << QString("failed to initialise libusb: %1").arg(libusb_error_name(rc));
    }

    hotplugThread = 0;

    spindleSetEnable = false;
    fluidSetEnable = false;
    mistSetEnable = false;

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

            pack9D(); // to get actual info from device
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


void mk1Controller::setUseHome(bool b)
{
    if (b == true) {
    }
}


// startpos in mm
void mk1Controller::setStartPos(float x, float y, float z, float a)
{
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

            pack9D(); // to get tha actual info from device
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

    for (int c = 0; c < axisList.count(); c++) {
        int i = settingsFile->value("Pulse" + axisList.at(c), 400).toInt( &res);

        if (res == true) {
            coord[c].pulsePerMm = i;
        }

        float f = settingsFile->value("Accel" + axisList.at(c), 15).toFloat( &res);

        if (res == true) {
            coord[c].acceleration = f;
        }

        f = settingsFile->value("StartVelo" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].minVelo = f;
        }

        f = settingsFile->value("EndVelo" + axisList.at(c), 400).toFloat( &res);

        if (res == true) {
            coord[c].maxVelo = f;
        }

        coord[c].checkSoftLimits = settingsFile->value("SoftLimit" + axisList.at(c), false).toBool( );

        f = settingsFile->value("SoftMin" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].softLimitMin = f;
        }

        f = settingsFile->value("SoftMax" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].softLimitMax = f;
        }

        f = settingsFile->value("Home" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].home = f;
        }

        coord[c].useLimitMin = settingsFile->value("HardLimitMin" + axisList.at(c), true).toBool();
        coord[c].useLimitMax = settingsFile->value("HardLimitMax" + axisList.at(c), true).toBool();

        //
        coord[c].invertDirection = settingsFile->value("InvDirection" + axisList.at(c), false).toBool();
        coord[c].invertPulses = settingsFile->value("InvPulses" + axisList.at(c), false).toBool();
        coord[c].invLimitMax = settingsFile->value("InvLimitMax" + axisList.at(c), false).toBool();
        coord[c].invLimitMin = settingsFile->value("InvLimitMin" + axisList.at(c), false).toBool();
        coord[c].enabled = settingsFile->value("Enabled" + axisList.at(c), true).toBool();

        f = settingsFile->value("Backlash" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].backlash = f;
        }

        f = settingsFile->value("WorkAreaMin" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].workAreaMin = f;
        }

        f = settingsFile->value("WorkAreaMax" + axisList.at(c), 0).toFloat( &res);

        if (res == true) {
            coord[c].workAreaMax = f;
        }

        //
    }

    settingsFile->endGroup();
}

//
// save settings
//
void mk1Controller::saveSettings()
{
    settingsFile->beginGroup("mk1");

    for (int c = 0; c < axisList.count(); c++) {
        settingsFile->setValue("Pulse" + axisList.at(c), coord[c].pulsePerMm);
        settingsFile->setValue("Accel" + axisList.at(c), (double)coord[c].acceleration);
        settingsFile->setValue("StartVelo" + axisList.at(c), (double)coord[c].minVelo);
        settingsFile->setValue("EndVelo" + axisList.at(c), (double)coord[c].maxVelo);

        //
        settingsFile->setValue("Backlash" + axisList.at(c), (double)coord[c].backlash);
        settingsFile->setValue("InvDirection" + axisList.at(c), (bool)coord[c].invertDirection);
        settingsFile->setValue("InvPulses" + axisList.at(c), (bool)coord[c].invertPulses);
        settingsFile->setValue("InvLimitMax" + axisList.at(c), (bool)coord[c].invLimitMax);
        settingsFile->setValue("InvLimitMin" + axisList.at(c), (bool)coord[c].invLimitMin);
        settingsFile->setValue("WorkAreaMin" + axisList.at(c), (double)coord[c].workAreaMin);
        settingsFile->setValue("WorkAreaMax" + axisList.at(c), (double)coord[c].workAreaMax);
        settingsFile->setValue("Enabled" + axisList.at(c), (bool)coord[c].enabled);
        //

        settingsFile->setValue("HardLimitMin" + axisList.at(c), (bool)coord[c].useLimitMin);
        settingsFile->setValue("HardLimitMax" + axisList.at(c), (bool)coord[c].useLimitMax);

        settingsFile->setValue("SoftLimit" + axisList.at(c), (bool)coord[c].checkSoftLimits);
        settingsFile->setValue("SoftMin" + axisList.at(c), (double)coord[c].softLimitMin);
        settingsFile->setValue("SoftMax" + axisList.at(c), (double)coord[c].softLimitMax);

        settingsFile->setValue("Home" + axisList.at(c), (double)coord[c].home);
    }

    settingsFile->endGroup();

    settingsFile->sync();

    sendSettings();
}


// send settings to mk
void mk1Controller::sendSettings()
{
    setSettings = true;

    packD3();
    packAB();

    pack9F(); // set pulses per mm

    packA0(); // set acceleration
    packA1(); // set allowed limits

    packBF(coord[X].maxVelo, coord[Y].maxVelo, coord[Z].maxVelo, coord[A].maxVelo); // set max velocities

    packB5(spindleSetEnable); // spindle off

    //     mistSetEnable = false;
    //     fluidSetEnable = false;

    packB6(mistSetEnable, fluidSetEnable); // mist, fluid coolant

    packC2(); // unknown
    pack9D();

    pack9E(0x80);

    setSettings = false;
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
int mk1Controller::getSpindleMoveSpeed()
{
    return spindleMoveSpeed;
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
bool mk1Controller::isFluidOn()
{
    return fluidEnabled;
}

//
// mist is on?
//
bool mk1Controller::isMistOn()
{
    return mistEnabled;
}

//
// splindle is on?
//
bool mk1Controller::isSpindelOn()
{
    return spindleEnabled;
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
    int velo = (int)(((/*(readBuf[23] << 24)*/ + (readBuf[22] << 16) + (readBuf[21] << 8) + (readBuf[20]))) / 2.1);

    if (velo > 5000) {
        return;
    }

    spindleMoveSpeed = velo;
    // for mk2 instead 2.1 = > 1.341

    coord[X].actualPosPulses = ((readBuf[27] << 24) + (readBuf[26] << 16) + (readBuf[25] << 8) + (readBuf[24]));
    coord[Y].actualPosPulses = ((readBuf[31] << 24) + (readBuf[30] << 16) + (readBuf[29] << 8) + (readBuf[28]));
    coord[Z].actualPosPulses = ((readBuf[35] << 24) + (readBuf[34] << 16) + (readBuf[33] << 8) + (readBuf[32]));
    coord[A].actualPosPulses = ((readBuf[39] << 24) + (readBuf[38] << 16) + (readBuf[37] << 8) + (readBuf[36]));

    byte bb15 = readBuf[15];

    coord[X].actualLimitMin = (bb15 & 0x01) != 0;
    coord[X].actualLimitMax = (bb15 & 0x02) != 0;
    coord[Y].actualLimitMin = (bb15 & 0x04) != 0;
    coord[Y].actualLimitMax = (bb15 & 0x08) != 0;
    coord[Z].actualLimitMin = (bb15 & 0x10) != 0;
    coord[Z].actualLimitMax = (bb15 & 0x20) != 0;
    coord[A].actualLimitMin = (bb15 & 0x40) != 0;
    coord[A].actualLimitMax = (bb15 & 0x80) != 0;

    NumberCompleatedInstruction = ((readBuf[9] << 24) + (readBuf[8] << 16) + (readBuf[7] << 8) + (readBuf[6]));

    byte bb19 = readBuf[19];

    spindleEnabled = (bb19 & 0x01) ? true : false;

    byte bb14 = readBuf[14];
    Estop = (bb14 & 0x80) ? true : false;

    mistEnabled =  (bb14 & 0x10) ? false : true;
    fluidEnabled = (bb14 & 0x04) ? false : true;

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
    spindleSetEnable = true;
    packB5(spindleSetEnable);
}

//
// disable spindle
//
void mk1Controller::spindleOFF()
{
    spindleSetEnable = false;
    packB5(spindleSetEnable);
}


void mk1Controller::fluidON()
{
    fluidSetEnable = true;
    packB6(mistSetEnable, fluidSetEnable);
}


void mk1Controller::fluidOFF()
{
    fluidSetEnable = false;
    packB6(mistSetEnable, fluidSetEnable);
}


void mk1Controller::mistON()
{
    mistSetEnable = true;
    packB6(mistSetEnable, fluidSetEnable);
}


void mk1Controller::mistOFF()
{
    mistSetEnable = false;
    packB6(mistSetEnable, fluidSetEnable);
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

    byte axesDirection = 0x00;

    // set the bits
    if (x == "-") {
        axesDirection |= 0x01;
    }

    if (x == "+") {
        axesDirection |= 0x02;
    }

    if (y == "-") {
        axesDirection |= 0x04;
    }

    if (y == "+") {
        axesDirection |= 0x08;
    }

    if (z == "-") {
        axesDirection |= 0x10;
    }

    if (z == "+") {
        axesDirection |= 0x20;
    }

    if (a == "-") {
        axesDirection |= 0x40;
    }

    if (a == "+") {
        axesDirection |= 0x80;
    }

    packBE(axesDirection, speed);
}


void mk1Controller::stopManualMove()
{
    if (!isConnected()) {
        return;
    }

    packBE(0x00, 0, false);
    setByte(22, 0x01); //TODO: unknown
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
byte mk1Data::readBuf[BUFFER_SIZE];
byte mk1Data::writeBuf[BUFFER_SIZE];



void mk1Data::setByte(byte offset, byte data)
{
    if (offset >= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset] = data;
}


byte mk1Data::getByte(byte offset)
{
    if (offset >= BUFFER_SIZE) {
        return 0;
    }

    return readBuf[offset];
}


void mk1Data::cleanBuf(byte *m)
{
    memset(m, 0x0, BUFFER_SIZE);
}


void mk1Data::packTwoBytes(byte offset, int val)
{
    if ((offset + 1) >= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset + 0] = (byte)val;
    writeBuf[offset + 1] = (byte)(val >> 8);
}


void mk1Data::packFourBytes(byte offset, int val)
{
    if ((offset + 3) >= BUFFER_SIZE) {
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
void mk1Data::sendBinaryData(bool checkBuffSize)
{
    if (checkBuffSize) {
        if (FreebuffSize < 2) {
            //тут нужно зависнуть пока буфер не освободится
        }

        //TODO: check buffer....
    }

    QString st;

    for (int i = 0; i < 64; i++) {
        st += QString().sprintf("%2x ", writeBuf[i]);
    }

    qDebug() << "send" << st;

    if (!DEMO_DEVICE) {
        //         _error_code = _usb->write(rawData);//, 2000, bytesWritten);
        if (handle != 0) {
            int transferred = 0;
            int e = libusb_bulk_transfer(handle, BULK_ENDPOINT_IN, writeBuf, BUFFER_SIZE, &transferred, 300); // timeout 300 msecons

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
// value = 0x80 settings
void mk1Data::pack9D(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9d;

    if (setSettings == true) {
        writeBuf[4] = 0x80; // settings
        writeBuf[5] = 0x01; // unknown
    }

    if (send == true) {
        sendBinaryData();
    }
}


void mk1Data::pack9E(byte value, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9e;

    if (setSettings == true) { // if settings
        writeBuf[4] = 0x80;
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
// impulses per mm, LCD only?
void mk1Data::pack9F( bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9f;
    writeBuf[4] = 0x80; // settings
    writeBuf[5] = 0xb1; //TODO:unknown

    packFourBytes(6, coord[X].pulsePerMm);
    packFourBytes(10, coord[Y].pulsePerMm);
    packFourBytes(14, coord[Z].pulsePerMm);
    packFourBytes(18, coord[A].pulsePerMm);

    if (send == true) {
        sendBinaryData();
    }
}


// acceleration settings
void mk1Data::packA0(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa0;
    writeBuf[4] = 0x80; // settings

    writeBuf[5] = 0x12;

    int AccelX = 0;

    if ((coord[X].acceleration > 0) && (coord[X].pulsePerMm > 0)) {
        AccelX = (int)3186.7 * 3600.0 / sqrt(coord[X].acceleration * coord[X].pulsePerMm);
    }

    packFourBytes(6, AccelX);

    int AccelY = 0;

    if ((coord[Y].acceleration > 0) && (coord[Y].pulsePerMm > 0)) {
        AccelY = (int)3186.7 * 3600.0  / sqrt(coord[Y].acceleration * coord[Y].pulsePerMm);
    }

    packFourBytes(10, AccelY);

    int AccelZ = 0;

    if ((coord[Z].acceleration > 0) && (coord[Z].pulsePerMm > 0)) {
        AccelZ = (int)3186.7 * 3600.0  / sqrt(coord[Z].acceleration * coord[Z].pulsePerMm);
    }

    packFourBytes(14, AccelZ);

    int AccelA = 0;

    if ((coord[A].acceleration > 0) && (coord[A].pulsePerMm > 0)) {
        AccelA = (int)3186.7 * 3600.0 / sqrt(coord[A].acceleration * coord[A].pulsePerMm);
    }

    packFourBytes(18, AccelA);

    writeBuf[42] = 0x60;// unknown byte
    writeBuf[43] = 0x09;// unknown byte

    writeBuf[46] = 0x08;// unknown byte

    // reverse of axis.: 0xff no reverse, 0xfe axis x, 0xfd axis y, 0xfb axis z
    byte r = 0xff;
    r &= (coord[X].invertDirection == true) ? 0xfe : 0xff;
    r &= (coord[Y].invertDirection == true) ? 0xfd : 0xff;
    r &= (coord[Z].invertDirection == true) ? 0xfb : 0xff;
    r &= (coord[A].invertDirection == true) ? 0xf7 : 0xff;

    writeBuf[57] = r;// reverse axis
    writeBuf[58] = 0x01;// unknown byte

    // reverse motor steps, bitmask: 0 no inverting, 1 invert step X, 2 invert step Y, 4 invert step Z
    r = 0x0;
    r |= (coord[X].invertPulses == true) ? 0x01 : 0x00;
    r |= (coord[Y].invertPulses == true) ? 0x02 : 0x00;
    r |= (coord[Z].invertPulses == true) ? 0x04 : 0x00;
    r |= (coord[A].invertPulses == true) ? 0x08 : 0x00;

    writeBuf[59] = r; //
    writeBuf[60] = 0x00; // for mk2 ?

    if (send == true) {
        sendBinaryData();
    }
}


// limits activate
void mk1Data::packA1( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa1;

    writeBuf[4] = 0x80; // settings

    // allow limits: bit 7 a+; bit 6 a-, bit 5 z+, bit 4 z-, bit 3 y+, bit 2 y-, bit 1 x+, bit 0 x-
    byte limits = 0x0;
    limits |= (coord[X].useLimitMin == true) ? 0x01 : 0x00;
    limits |= (coord[X].useLimitMax == true) ? 0x02 : 0x00;
    limits |= (coord[Y].useLimitMin == true) ? 0x04 : 0x00;
    limits |= (coord[Y].useLimitMax == true) ? 0x08 : 0x00;
    limits |= (coord[Z].useLimitMin == true) ? 0x10 : 0x00;
    limits |= (coord[Z].useLimitMax == true) ? 0x20 : 0x00;
    limits |= (coord[A].useLimitMin == true) ? 0x40 : 0x00;
    limits |= (coord[A].useLimitMax == true) ? 0x80 : 0x00;

    writeBuf[42] = limits;
    writeBuf[48] = 0xff; // unknown, for mk2?

    if (send == true) {
        sendBinaryData();
    }
}

//
// emergency STOP
//
void mk1Data::packAA(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xaa;
    writeBuf[4] = 0x80; // settings

    if (send == true) {
        sendBinaryData();
    }
}


// unknown settings
void mk1Data::packAB( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xab;

    writeBuf[4] = 0x80; // settings

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
void mk1Data::packB5(bool spindleON, int numShimChanel, TypeSignal ts, int SpeedShim, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xb5;
    writeBuf[4] = 0x80; // settings


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

    packFourBytes(10, SpeedShim);

    if (send == true) {
        sendBinaryData();
    }
}


// mist/fluid settings
void mk1Data::packB6( bool mist, bool fluid, bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xb6;

    writeBuf[4] = 0x80; // settings

    if (fluid) {
        writeBuf[5] = 0x02;
    } else {
        writeBuf[5] = 0x01;
    }

    writeBuf[6] = 0x02; //TODO:unknown

    if (mist) {
        writeBuf[7] = 0x02;
    } else {
        writeBuf[7] = 0x01;
    }

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
void mk1Data::packBE(byte direction, int speed, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbe;
    writeBuf[4] = 0x80; // settings
    writeBuf[6] = direction;

    int inewSpd = 0;

    if (speed != 0) {
        float dnewSpd = (1800 / (float)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //velocity
    packFourBytes(10, inewSpd);

#if 0

    if (Setting.DeviceModel == DeviceModel.MK2) {
        //TODO: Для МК2 немного иные посылки данных

        if (speed != 0) {
            double dnewSpd = (9000 / (double)speed) * 1000;
            inewSpd = (int)dnewSpd;
        }

        //скорость
        buf[10] = (byte)(inewSpd);
        buf[11] = (byte)(inewSpd >> 8);
        buf[12] = (byte)(inewSpd >> 16);

        if (speed == 0) {
            buf[14] = 0x00;
            buf[18] = 0x01;
            buf[22] = 0x01;

            //x
            buf[26] = 0x00;
            buf[27] = 0x00;
            buf[28] = 0x00;
            buf[29] = 0x00;

            //y
            buf[30] = 0x00;
            buf[31] = 0x00;
            buf[32] = 0x00;
            buf[33] = 0x00;

            //z
            buf[34] = 0x00;
            buf[35] = 0x00;
            buf[36] = 0x00;
            buf[37] = 0x00;

            //a
            buf[38] = 0x00;
            buf[39] = 0x00;
            buf[40] = 0x00;
            buf[41] = 0x00;


        } else {
            buf[14] = 0xC8; //TODO: WTF??
            buf[18] = 0x14; //TODO: WTF??
            buf[22] = 0x14; //TODO: WTF??

            if (x == "+") {
                buf[26] = 0x40;
                buf[27] = 0x0D;
                buf[28] = 0x03;
                buf[29] = 0x00;
            }

            if (x == "-") {
                buf[26] = 0xC0;
                buf[27] = 0xF2;
                buf[28] = 0xFC;
                buf[29] = 0xFF;
            }

            if (y == "+") {
                buf[30] = 0x40;
                buf[31] = 0x0D;
                buf[32] = 0x03;
                buf[33] = 0x00;
            }

            if (y == "-") {
                buf[30] = 0xC0;
                buf[31] = 0xF2;
                buf[32] = 0xFC;
                buf[33] = 0xFF;
            }

            if (z == "+") {
                buf[34] = 0x40;
                buf[35] = 0x0D;
                buf[36] = 0x03;
                buf[37] = 0x00;
            }

            if (z == "-") {
                buf[34] = 0xC0;
                buf[35] = 0xF2;
                buf[36] = 0xFC;
                buf[37] = 0xFF;
            }

            if (a == "+") {
                buf[38] = 0x40;
                buf[39] = 0x0D;
                buf[40] = 0x03;
                buf[41] = 0x00;
            }

            if (a == "-") {
                buf[38] = 0xC0;
                buf[39] = 0xF2;
                buf[40] = 0xFC;
                buf[41] = 0xFF;
            }
        }
    }

#endif

    if (send == true) {
        sendBinaryData();
    }
}



//
// set velocity limit
//
void mk1Data::packBF(int speedLimitX, int speedLimitY, int speedLimitZ, int speedLimitA, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbf;

    if ( setSettings == true) {
        writeBuf[4] = 0x80; // settings
    }

    if (coord[X].enabled == true) {
        float dnewSpdX  = 3600; // 3584?

        if (speedLimitX != 0 && coord[X].pulsePerMm != 0) {
            dnewSpdX = 7.2e8 / ((float)speedLimitX * coord[X].pulsePerMm);
        }

        packFourBytes(7, (int)dnewSpdX);
    }

    if (coord[Y].enabled == true) {
        float dnewSpdY = 3600;

        if (speedLimitY != 0 && coord[Y].pulsePerMm != 0) {
            dnewSpdY = 7.2e8 / ((float)speedLimitY * coord[Y].pulsePerMm);
        }

        packFourBytes(11, (int)dnewSpdY);
    }

    if (coord[Z].enabled == true) {
        float dnewSpdZ = 3600;

        if (speedLimitZ != 0 && coord[Z].pulsePerMm != 0) {
            dnewSpdZ = 7.2e8 / ((float)speedLimitZ * coord[Z].pulsePerMm);
        }

        packFourBytes(15, (int)dnewSpdZ);
    }

    if (coord[A].enabled == true) {
        float dnewSpdA = 3600;

        if (speedLimitA != 0 && coord[A].pulsePerMm != 0) {
            dnewSpdA = 7.2e8 / ((float)speedLimitA * coord[A].pulsePerMm);
        }

        packFourBytes(19, (int)dnewSpdA);
    }

    if (send == true) {
        sendBinaryData();
    }
}


//
// command unknown....
//
// param "byte05"
//
void mk1Data::packC0(byte byte05, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xC0;
    writeBuf[5] = byte05;

    if (send == true) {
        sendBinaryData();
    }
}


// unknown settings
void mk1Data::packC2( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xc2;

    writeBuf[4] = 0x80; // settings
    writeBuf[5] = 0x03;

    if (send == true) {
        sendBinaryData();
    }
}


//
// set the coordinates without moving
//
void mk1Data::packC8(int x, int y, int z, int a, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xc8;

    //num of pulses
    packFourBytes(6, x);
    packFourBytes(10, y);
    packFourBytes(14, z);
    packFourBytes(18, a);

    if (send == true) {
        sendBinaryData();
    }
}

// recalculate from mm/inch to pulses
void mk1Data::packCA(float _posX, float _posY, float _posZ, float _posA, int _speed, int _NumberInstruction, float distance, int _pause, bool send)
{
    int xPulses = coord[X].posPulse(_posX);
    int yPulses = coord[Y].posPulse(_posY);
    int zPulses = coord[Z].posPulse(_posZ);
    int aPulses = coord[A].posPulse(_posA);

    packCA(xPulses, yPulses, zPulses, aPulses, _speed, _NumberInstruction, distance, _pause, send);
}


//
// moving to the point
//
void mk1Data::packCA(int _posX, int _posY, int _posZ, int _posA, int _speed, int _NumberInstruction, float distance, int _pause, bool send)
{
    int newInst = _NumberInstruction;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xca;

    //save the number instruction
    packFourBytes(1, newInst);

    if (distance > 0.0 && distance < 5.0) {
        writeBuf[5] = 0x03;
    } else {
        writeBuf[5] = _pause; //TODO: delay in µs? was 0x39
    }

    //how many pulses
    packFourBytes(6, _posX);
    packFourBytes(10, _posY);
    packFourBytes(14, _posZ);
    packFourBytes(18, _posA);


    //     int inewSpd = 2328; //TODO: default velocity
    //
    //     if (_speed != 0) {
    //         float dnewSpd = (3600.0 / (float)_speed) * 1000.0;
    //         inewSpd = (int)dnewSpd;
    //     }

    int inewSpd = 3600;

    if (_speed != 0 && coord[X].pulsePerMm != 0) {
        inewSpd = 7.2e8 / ((float)_speed * coord[X].pulsePerMm);
    }

    //axes xspeed
    packFourBytes(43, inewSpd);

    // todo: unknown 4 byte from offset 46
    int unk = 0;
    packFourBytes(46, unk);

    writeBuf[54] = 0x40;  //TODO: unknown byte

    if (send == true) {
        sendBinaryData();
    }
}


//
// check length of tool
//
void mk1Data::packD2(int speed, float returnDistance, bool send)
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

    //unknown
    writeBuf[46] = 0x10;

    //
    int inewReturn = (int)(returnDistance * (float)coord[Z].pulsePerMm);

    //return to position
    packFourBytes(50, inewReturn);

    //unknown
    writeBuf[55] = 0x12;
    writeBuf[56] = 0x7A;

    if (send == true) {
        sendBinaryData();
    }
}


// unknown settings
void mk1Data::packD3( bool send )
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
void mk1Data::packFF(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xff;

    if (send == true) {
        sendBinaryData();
    }
}


