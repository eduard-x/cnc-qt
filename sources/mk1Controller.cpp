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

#include <QDebug>
#include <QIODevice>

#include "includes/Settings.h"
#include "includes/mk1Controller.h"


#include <QString>


/******************************************************************************
** mk1Controller
*/

// #define CLASS_ID LIBUSB_HOTPLUG_MATCH_ANY

// static
libusb_device_handle *mk1Data::handle = 0;

libusb_hotplug_callback_handle mk1Controller::hotplug[2];

libusb_device_descriptor mk1Data::desc = {0};

QString mk1Controller::devDescriptor;


/**
 * @brief callback function for hotplug
 *
 */
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

/**
 * @brief callback function for detach
 *
 */
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

/**
 * @brief constructor
 *
 */
mk1Controller::mk1Controller(QObject *parent) : QObject(parent)
{
    int rc;

    //     vendor_id  =  0x2121;
    //     product_id = 0x2130;

    handle = NULL;

    devConnected = false;

    //     rc = libusb_init (NULL);
    //
    //     if (rc < 0) {
    //         qDebug() << QString("failed to initialise libusb: %1").arg(libusb_error_name(rc));
    //     }

    //     hotplugThread = 0;

    spindleSetEnable = false;
    fluidSetEnable = false;
    mistSetEnable = false;

    readThread = 0;
}


/**
 * @brief destructor
 *
 */
mk1Controller::~mk1Controller()
{
    //   int class_id   = LIBUSB_HOTPLUG_MATCH_ANY;
    //
    //   libusb_hotplug_deregister_callback (NULL, hotplug[0]);
    //    libusb_hotplug_deregister_callback (NULL, hotplug[1]);

    if (handle) {
        libusb_close(handle);
        handle = 0;
    }

    //          int r = libusb_release_interface(handle, 0); //release the claimed interface
    //
    //         if(r != 0) {
    //             qDebug() << "Cannot Release Interface" << endl;
    //         } else {
    //             qDebug() << "Released Interface" << endl;
    //             libusb_close(handle);
    //         }

    if (readThread) {

        readThread->quit();

        //             disconnect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));
        //             delete readThread;
        readThread = 0;
    }

#if 1

    //         if (hotplugThread) {
    //             //             hotplugThread->quit();
    //             disconnect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(onHandleHotplug()));
    //             delete hotplugThread;
    //         }

#endif
    //     }

    //     libusb_exit(NULL);
}


// Event:
// Our device appeared in the system
void mk1Controller::onDeviceConnected()
{
#if 0
    // List storing all detected devices
    libusb_device **devs;

    // Get all devices connected to system
    if (libusb_get_device_list(NULL, &devs) < 0) {
        qDebug() << "Failed to get devices list";
        return;
    }

    // Scan for LaunchpadDevice VID & PID
    libusb_device *dev;
    // retdev stores the first device detected
    libusb_device *retdev = NULL;
    int i = 0;
    int count = 0;

    while ( (dev = devs[i++]) != NULL ) {
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            qDebug() << "failed to get device descriptor";
            continue;
        }

        // Is this device the one we are looking for?
        if ( (desc.idVendor == VENDOR_ID) && (desc.idProduct == PRODUCT_ID) ) {

            count++;

            if ( retdev == NULL ) {
                retdev = dev;
            }
        }
    }

    if (count <= 0) {
        qDebug() << "Warning! No device found";
        return;
    }

    if (count > 1 ) {
        qDebug() << "Warning! More than one device found. Using first from the list";
    }

    if (libusb_open(retdev, &this->LaunchpadDevice) < 0) {
        qDebug() << "Error! Failed to open device";
        return;
    }

    libusb_free_device_list(devs, 1);

    // now connect to interface

    if (libusb_claim_interface(this->LaunchpadDevice, 0) != LIBUSB_SUCCESS) {
        qDebug() << "Error! Failed to claim interface";
        return;
    }

#endif

    qDebug() << "DeviceConnected";
    devConnected = false;


    if (readThread != 0) {
        disconnect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));

        readThread->quit();

        delete readThread;
        readThread = 0;
    }

#if 0
    // If all ok enable control
    this->ui->slider_R->setEnabled(true);
    this->ui->slider_G->setEnabled(true);
    this->ui->slider_B->setEnabled(true);
    this->ui->button_ONOFF->setEnabled(true);

    // Send some data for embedded device initial conditions
    slider_Change();
    USBSendONOFF(false);

    // Start the timer to pool updates from device
    this->queryTimer->start(500);

    ui->statusBar->showMessage("Device connected.");
#endif
    int rc;
    bool devAlreadyConnected = false;

    handle = libusb_open_device_with_vid_pid(NULL, MK1_VENDOR_ID, MK1_PRODUCT_ID); //these are vendorID and productID I found for my usb device

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
        //         libusb_exit (NULL);
    } else {
#if 0
        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_ENUMERATE, MK1_VENDOR_ID,
                                               MK1_PRODUCT_ID, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL, &hotplug[0]);

        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "Error registering callback 0";
            //             libusb_exit (NULL);
        } else {
            qDebug() << "Device registering attach callback";
        }

        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, MK1_VENDOR_ID,
                                               MK1_PRODUCT_ID, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback_detach, NULL, &hotplug[1]);

        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "Error registering callback 1";
            //             libusb_exit (NULL);
            //         } else {
            //             qDebug() << "Device registering detach callback";
            //             hotplugThread = new usbHotplugThread();
            //             connect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(onHandleHotplug()));
            //             hotplugThread->start();
        }

#endif

        if (devAlreadyConnected == true) {
            if (handle != 0) {
                libusb_close(handle);
            }

            handle = libusb_open_device_with_vid_pid(NULL, MK1_VENDOR_ID, MK1_PRODUCT_ID); //these are vendorID and productID I found for my usb device
            int e = libusb_set_configuration (handle, 1);

            if(e < 0) {
                qDebug() << "Cannot Claim Interface";
                libusb_close(handle);
                handle = 0;
            } else    {
                qDebug() << "Claimed Interface";
                devConnected = true;

                readThread = new usbReadThread(this);

                connect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));

                readThread->start();
            }

            getDeviceInfo();

            pack9D(); // to get actual info from device
        }
    }
}

// Event:
// Device was disconnected
void mk1Controller::onDeviceDisconnected()
{
    qDebug() << "DeviceDisconnected";

    handle = 0;

    devConnected = false;

    if (readThread != 0) {
        disconnect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));
        //         readThread->quit();

        delete readThread;
        readThread = 0;
    }

    // Disable timer
#if 0
    this->queryTimer->stop();

    // Disable controls and reset them to initial state

    this->ui->slider_R->setValue(200);
    this->ui->slider_G->setValue(200);
    this->ui->slider_B->setValue(200);
    this->ui->button_ONOFF->setChecked(false);

    this->ui->slider_R->setEnabled(false);
    this->ui->slider_G->setEnabled(false);
    this->ui->slider_B->setEnabled(false);
    this->ui->button_ONOFF->setEnabled(false);

    if(this->LaunchpadDevice != NULL) {
        libusb_release_interface(this->LaunchpadDevice, 0);
        libusb_close(this->LaunchpadDevice);
        this->LaunchpadDevice = NULL;
    }

    ui->statusBar->showMessage("Awaiting device connection.");
#endif

}



/**
 * @brief read device description if connected
 *
 */
int mk1Controller::getDeviceInfo()
{
    int rc;
    QString descrStr;

    if (handle == 0) {
        devDescriptor = "Device not comnnected";
        return -1;
    }

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


/**
 * @brief clean device desription
 *
 */
void mk1Controller::resetDescription()
{
    devDescriptor.clear();
}

/**
 * @brief get controller description string
 *
 */
QString mk1Controller::getDescription()
{
    return devDescriptor;
}

/**
 * @brief
 *
 */
void mk1Controller::setUseHome(bool b)
{
    if (b == true) {
    }
}

/**
 * @brief  startpos in mm
 *
 */
void mk1Controller::setStartPos(float x, float y, float z, float a)
{
}

/**
 * @brief set controller description string
 *
 */
void mk1Controller::setDescription(const QString &c)
{
    devDescriptor = c;
}

/**
 * @brief slot from readThread
 *
 */
void mk1Controller::onReadNewData()
{
    //     qDebug() << "new data from usb";
    parseBinaryInfo();
}

/**
 * @brief
 *
 */
void mk1Controller::onBufFree()
{
    qDebug() << "signal: read buffer is free";
}

#if 0
/**
 * @brief slot from hotplugThread
 *
 */
void mk1Controller::onHandleHotplug()
{
    disconnect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(onHandleHotplug()));

    if (handle) {
        if (devConnected == false) {
            qDebug() << "hotplug attach" << handle;

            emit hotplugSignal();
        }

        if (readThread == 0) {
            readThread = new usbReadThread(this);

            connect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));

            readThread->start();

            pack9D(); // to get tha actual info from device
        }

        devConnected = true;

    } else {
        if (devConnected == true) {
            qDebug() << "hotplug detach" << handle;

            readThread->exit();

            disconnect(readThread, SIGNAL(readEvent()), this, SLOT(onReadNewData()));

            emit hotplugSignal();
        }

        readThread = 0;

        devConnected = false;
    }

    connect(hotplugThread, SIGNAL(hotplugEvent()), this, SLOT(onHandleHotplug()));
}
#endif
/**
 * @brief send settings to mk
 *
 */
void mk1Controller::sendSettings()
{
    setSettings = true;

    packD3();
    packAB();

    pack9F(); // set pulses per mm

    packA0(); // set acceleration
    packA1(); // set allowed limits

    int limits[4] = {(int)Settings::coord[X].maxVeloLimit, (int)Settings::coord[Y].maxVeloLimit, (int)Settings::coord[Z].maxVeloLimit, (int)Settings::coord[A].maxVeloLimit};

    packBF(limits); // set max velocities

    packB5(spindleSetEnable); // spindle off

    //     mistSetEnable = false;
    //     fluidSetEnable = false;

    packB6(mistSetEnable, fluidSetEnable); // mist, fluid coolant

    packC2(); // unknown
    pack9D();

    pack9E(0x80);

    setSettings = false;
}

/**
 * @brief  info about connection
 *
 */
bool mk1Controller::isConnected()
{
    return devConnected;
}


/**
 * @brief  velocity of spindle
 *
 */
int mk1Data::getSpindleMoveSpeed()
{
    return spindleMoveSpeed;
}

/**
 * @brief
 *
 */
void mk1Data::setSpindleMoveSpeed(int i)
{
    spindleMoveSpeed = i;
}


/**
 * @brief current instruction number
 *
 */
long mk1Data::numberCompleatedInstructions()
{
    return numberCompleatedInstruction;
}


void mk1Data::setCompleatedInstructions(long i)
{
    numberCompleatedInstruction = i;
}


/**
 * @brief  splindle is on?
 *
 */
bool mk1Data::isFluidOn()
{
    return fluidEnabled;
}


/**
 * @brief
 *
 */
void mk1Data::setFluidOn(bool b)
{
    fluidEnabled = b;
}

/**
 * @brief mist is on?
 *
 */
bool mk1Data::isMistOn()
{
    return mistEnabled;
}

void mk1Data::setMistOn(bool b)
{
    mistEnabled = b;
}

/**
 * @brief splindle is on?
 *
 */
bool mk1Data::isSpindelOn()
{
    return spindleEnabled;
}


/**
 * @brief
 *
 */
void mk1Data::setSpindelOn(bool b)
{
    spindleEnabled = b;
}


/**
 * @brief was stopped because of emergency?
 *
 */
bool mk1Data::isEmergencyStopOn()
{
    return eStop;
}

/**
 * @brief
 *
 */
void mk1Data::setEmergencyStopOn(bool b)
{
    eStop = b;
}


/**
 * @brief check connection
 *
 */
bool mk1Controller::testAllowActions()
{
    if (!isConnected()) {
        return false;
    }

    return true;
}

/**
 * @brief free buffer size
 *
 */
int mk1Data::availableBufferSize()
{
    return freebuffSize;
}


/**
 * @brief
 */
void mk1Data::setBufferSize(int i)
{
    freebuffSize = (byte)i;
}


/**
 * @brief  parse binary data from controller and set the main widget variables and settings data
 *
 */
void mk1Controller::parseBinaryInfo()
{
    setBufferSize(readBuf[1]);

    int velo = (int)(((/*(readBuf[23] << 24)*/ + (readBuf[22] << 16) + (readBuf[21] << 8) + (readBuf[20]))) / 2.1);

    if (velo > 5000) {
        return;
    }

    setSpindleMoveSpeed (velo);
    // for mk2 instead 2.1 = > 1.341
    Settings::coord[X].actualPosPulses = ((readBuf[27] << 24) + (readBuf[26] << 16) + (readBuf[25] << 8) + (readBuf[24]));
    Settings::coord[Y].actualPosPulses = ((readBuf[31] << 24) + (readBuf[30] << 16) + (readBuf[29] << 8) + (readBuf[28]));
    Settings::coord[Z].actualPosPulses = ((readBuf[35] << 24) + (readBuf[34] << 16) + (readBuf[33] << 8) + (readBuf[32]));
    Settings::coord[A].actualPosPulses = ((readBuf[39] << 24) + (readBuf[38] << 16) + (readBuf[37] << 8) + (readBuf[36]));

    // mk2 part
    Settings::coord[B].actualPosPulses = ((readBuf[43] << 24) + (readBuf[42] << 16) + (readBuf[41] << 8) + (readBuf[40]));
    Settings::coord[C].actualPosPulses = ((readBuf[47] << 24) + (readBuf[46] << 16) + (readBuf[45] << 8) + (readBuf[44]));
    Settings::coord[U].actualPosPulses = ((readBuf[51] << 24) + (readBuf[50] << 16) + (readBuf[49] << 8) + (readBuf[48]));
    Settings::coord[V].actualPosPulses = ((readBuf[55] << 24) + (readBuf[54] << 16) + (readBuf[53] << 8) + (readBuf[52]));
    Settings::coord[W].actualPosPulses = ((readBuf[59] << 24) + (readBuf[58] << 16) + (readBuf[57] << 8) + (readBuf[56]));

    byte bb15 = readBuf[15];

    Settings::coord[X].actualLimitMin = (bb15 & 0x01) != 0;
    Settings::coord[X].actualLimitMax = (bb15 & 0x02) != 0;
    Settings::coord[Y].actualLimitMin = (bb15 & 0x04) != 0;
    Settings::coord[Y].actualLimitMax = (bb15 & 0x08) != 0;
    Settings::coord[Z].actualLimitMin = (bb15 & 0x10) != 0;
    Settings::coord[Z].actualLimitMax = (bb15 & 0x20) != 0;
    Settings::coord[A].actualLimitMin = (bb15 & 0x40) != 0;
    Settings::coord[A].actualLimitMax = (bb15 & 0x80) != 0;

    long num = ((readBuf[9] << 24) + (readBuf[8] << 16) + (readBuf[7] << 8) + (readBuf[6]));

    setCompleatedInstructions(num);

    Settings::bb16 = readBuf[16];
    Settings::bb19 = readBuf[19];

    setSpindelOn((Settings::bb19 & 0x01) ? true : false);

    Settings::bb14 = readBuf[14];

    setEmergencyStopOn((Settings::bb14 & 0x80) ? true : false);

    setMistOn((Settings::bb14 & 0x10) ? false : true);
    setFluidOn((Settings::bb14 & 0x04) ? false : true);

    emit newDataFromMK1Controller(); // signal to main program
}


/**
 * @brief send number of message to main class
 *
 */
void mk1Controller::ADDMessage(int num)
{
    emit Message(num);
}


/**
 * @brief enable spindle
 *
 */
void mk1Controller::spindleON()
{
    spindleSetEnable = true;
    packB5(spindleSetEnable);
}

/**
 * @brief disable spindle
 *
 */
void mk1Controller::spindleOFF()
{
    spindleSetEnable = false;
    packB5(spindleSetEnable);
}

/**
 * @brief enable fluid coolant
 *
 */
void mk1Controller::fluidON()
{
    fluidSetEnable = true;
    packB6(mistSetEnable, fluidSetEnable);
}

/**
 * @brief disable fluid coolant
 *
 */
void mk1Controller::fluidOFF()
{
    fluidSetEnable = false;
    packB6(mistSetEnable, fluidSetEnable);
}

/**
 * @brief enable mist coolant
 *
 */
void mk1Controller::mistON()
{
    mistSetEnable = true;
    packB6(mistSetEnable, fluidSetEnable);
}

/**
 * @brief disable fluid coolant
 *
 */
void mk1Controller::mistOFF()
{
    mistSetEnable = false;
    packB6(mistSetEnable, fluidSetEnable);
}


/**
 * @brief send emergency stop
 *
 */
void mk1Controller::emergyStop()
{
    packAA();
}

/**
 * @brief manual moving
 * input qstring parameter is "+", "0" or "-"
 *  speed: velocity
 */
void mk1Controller::startManualMove(QString x, QString y, QString z, QString a, int speed, int pulses)
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

    packBE(axesDirection, speed, pulses);
}

/**
 * @brief
 *
 */
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


/**
 * @brief new position in pulses
 *
 */
void mk1Controller::deviceNewPosition(int x, int y, int z, int a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(x, y, z, a);
}


/**
 * @brief new position in mm
 *
 */
void mk1Controller::deviceNewPosition(float x, float y, float z, float a)
{
    if (!testAllowActions()) {
        return;
    }

    packC8(Settings::coord[X].posPulse(x), Settings::coord[Y].posPulse(y), Settings::coord[Z].posPulse(z), Settings::coord[A].posPulse(a));
}


// You MUST declare it, because of static field
byte mk1Data::readBuf[BUFFER_SIZE];
byte mk1Data::writeBuf[BUFFER_SIZE];


/**
 * @brief set one byte in array
 *
 */
void mk1Data::setByte(byte offset, byte data)
{
    if (offset >= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset] = data;
}


/**
 * @brief get one byte from buffer
 *
 */
byte mk1Data::getByte(byte offset)
{
    if (offset >= BUFFER_SIZE) {
        return 0;
    }

    return readBuf[offset];
}


/**
 * @brief vlean buffer
 *
 */
void mk1Data::cleanBuf(byte *m)
{
    memset(m, 0x0, BUFFER_SIZE);
}


/**
 * @brief set two bytes in buffer
 *
 */
void mk1Data::packTwoBytes(byte offset, int val)
{
    if ((offset + 1) >= BUFFER_SIZE) {
        return;
    }

    writeBuf[offset + 0] = (byte)val;
    writeBuf[offset + 1] = (byte)(val >> 8);
}

/**
 * @brief set four bytes in buffer
 *
 */
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

/**
 * @brief send the data to controller
 *
 */
void mk1Data::sendBinaryData(bool checkBuffSize)
{
    if (checkBuffSize) {
        if (freebuffSize < 2) {
            //тут нужно зависнуть пока буфер не освободится
        }

        //TODO: check buffer....
    }

#if 0
    // for debugging od send data
    QString st;

    for (int i = 0; i < 64; i++) {
        st += QString().sprintf("%2x ", writeBuf[i]);
    }

    qDebug() << "send" << st;
#endif

    if (!Settings::DEMO_DEVICE) {
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


/**
 * @brief UNKNOWN COMMAND
 * value = 0x80 for writing of settings
 */
void mk1Data::pack9D(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9d;
#if 0 // ??? was enabled 

    if (setSettings == true) {
        writeBuf[4] = 0x80; // settings
        writeBuf[5] = 0x01; // unknown
    }

#endif

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief
 *
 */
void mk1Data::pack9E(byte value, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9e;

    if (setSettings == true) { // if settings
        writeBuf[4] = 0x01; // ??? was 0x80
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


/**
 * @brief settings
 * impulses per mm, LCD only?
 */
void mk1Data::pack9F( bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0x9f;
    writeBuf[4] = 0x80; // settings
    writeBuf[5] = 0xb1; //TODO:unknown 0xb3 ???

    packFourBytes(6, Settings::coord[X].pulsePerMm);
    packFourBytes(10, Settings::coord[Y].pulsePerMm);
    packFourBytes(14, Settings::coord[Z].pulsePerMm);
    packFourBytes(18, Settings::coord[A].pulsePerMm);

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief acceleration settings
 *
 */
void mk1Data::packA0(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa0;
    writeBuf[4] = 0x80; // settings

    writeBuf[5] = 0x12;

    int AccelX = 0;

    if ((Settings::coord[X].acceleration > 0) && (Settings::coord[X].pulsePerMm > 0)) {
        AccelX = (int)11446815.5 / qSqrt(Settings::coord[X].acceleration * Settings::coord[X].pulsePerMm);
    }

    packFourBytes(6, AccelX);

    int AccelY = 0;

    if ((Settings::coord[Y].acceleration > 0) && (Settings::coord[Y].pulsePerMm > 0)) {
        AccelY = (int)11446815.5  / qSqrt(Settings::coord[Y].acceleration * Settings::coord[Y].pulsePerMm);
    }

    packFourBytes(10, AccelY);

    int AccelZ = 0;

    if ((Settings::coord[Z].acceleration > 0) && (Settings::coord[Z].pulsePerMm > 0)) {
        AccelZ = (int)11446815.5  / qSqrt(Settings::coord[Z].acceleration * Settings::coord[Z].pulsePerMm);
    }

    packFourBytes(14, AccelZ);

    int AccelA = 0;

    if ((Settings::coord[A].acceleration > 0) && (Settings::coord[A].pulsePerMm > 0)) {
        AccelA = (int)11446815.5 / qSqrt(Settings::coord[A].acceleration * Settings::coord[A].pulsePerMm);
    }

    packFourBytes(18, AccelA);


    writeBuf[42] = 0xb0;// unknown byte 0xb0 ??? was 0x60
    writeBuf[43] = 0x04;// unknown byte 0x04 ??? was 0x09

    writeBuf[46] = 0x08;// unknown byte

    // reverse of axis.: 0xff no reverse, 0xfe axis x, 0xfd axis y, 0xfb axis z
    byte r = 0xff;
    // reset bits
    r &= (Settings::coord[X].invertDirection == true) ? 0xfe : 0xff;
    r &= (Settings::coord[Y].invertDirection == true) ? 0xfd : 0xff;
    r &= (Settings::coord[Z].invertDirection == true) ? 0xfb : 0xff;
    r &= (Settings::coord[A].invertDirection == true) ? 0xf7 : 0xff;

    writeBuf[57] = r;// reverse axis
    writeBuf[58] = 0x01;// unknown byte

    // reverse motor steps, bitmask: 0 no inverting, 1 invert step X, 2 invert step Y, 4 invert step Z
    r = 0x0;
    // set bits
    r |= (Settings::coord[X].invertPulses == true) ? 0x01 : 0x00;
    r |= (Settings::coord[Y].invertPulses == true) ? 0x02 : 0x00;
    r |= (Settings::coord[Z].invertPulses == true) ? 0x04 : 0x00;
    r |= (Settings::coord[A].invertPulses == true) ? 0x08 : 0x00;

    writeBuf[59] = r; //
    writeBuf[60] = 0x00; // for mk2 ?

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief hardware limits activate
 *
 */
void mk1Data::packA1( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xa1;

    writeBuf[4] = 0x80; // settings

    // allow limits: bit 7 a+; bit 6 a-, bit 5 z+, bit 4 z-, bit 3 y+, bit 2 y-, bit 1 x+, bit 0 x-
    byte limits = 0x0;
    // set bits
    limits |= (Settings::coord[X].useLimitMin == true) ? 0x01 : 0x00;
    limits |= (Settings::coord[X].useLimitMax == true) ? 0x02 : 0x00;
    limits |= (Settings::coord[Y].useLimitMin == true) ? 0x04 : 0x00;
    limits |= (Settings::coord[Y].useLimitMax == true) ? 0x08 : 0x00;
    limits |= (Settings::coord[Z].useLimitMin == true) ? 0x10 : 0x00;
    limits |= (Settings::coord[Z].useLimitMax == true) ? 0x20 : 0x00;
    limits |= (Settings::coord[A].useLimitMin == true) ? 0x40 : 0x00;
    limits |= (Settings::coord[A].useLimitMax == true) ? 0x80 : 0x00;

    writeBuf[42] = limits;
    writeBuf[48] = 0xff; // unknown, for mk2?

    if (send == true) {
        sendBinaryData();
    }
}

/**
 * @brief emergency STOP
 *
 */
void mk1Data::packAA(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xaa;
    writeBuf[4] = 0x80; // settings

    if (send == true) {
        sendBinaryData();
    }
}

/**
 * @brief unknown settings
 *
 */
void mk1Data::packAB( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xab;

    writeBuf[4] = 0x80; // settings

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief spindle commands
 *
 * spindleON: on/off
 * numPWMChanel chan number 1,2, or 3
 * to signal type
 * SpeedPWM signal form
 */
void mk1Data::packB5(bool spindleON, int numPWMChanel, TypeSignal ts, int SpeedPWM, bool send)
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
    // ??? was enabled

    switch (numPWMChanel) {
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

    packFourBytes(10, SpeedPWM);

    if (send == true) {
        sendBinaryData();
    }
}

/**
 * @brief mist/fluid settings
 *
 */
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

    // ???
    //     writeBuf[6] = 0x02; //TODO:unknown

    if (mist) {
        writeBuf[7] = 0x02;
    } else {
        writeBuf[7] = 0x01;
    }

    // ???
    //     writeBuf[8] = 0x03; //TODO:unknown

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief
 *
 */
// moving without stop (and stop)
//
// direction axes
// speed
//
void mk1Data::packBE(byte direction, int speed, int lenInPulses, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbe;
    writeBuf[4] = 0x80; // settings
    writeBuf[6] = direction;

#if 1
    float inewSpd  = 3600; // 3584?

    int pulsesPerMM = 0;

    if (direction & 0x03) {
        pulsesPerMM = Settings::coord[X].pulsePerMm;
    }

    if (direction & 0x0c) {
        pulsesPerMM = Settings::coord[Y].pulsePerMm;
    }

    if (direction & 0x30) {
        pulsesPerMM = Settings::coord[Z].pulsePerMm;
    }

    if (direction & 0xc0) {
        pulsesPerMM = Settings::coord[A].pulsePerMm;
    }

    if (speed != 0 && pulsesPerMM != 0) {
        inewSpd = 7.2e8 / ((float)speed * pulsesPerMM);
    }

    packFourBytes(10, (int)inewSpd);

    packFourBytes(22, lenInPulses);
#else

    int inewSpd = 0;

    if (speed != 0) {
        float dnewSpd = (1800 / (float)speed) * 1000;
        inewSpd = (int)dnewSpd;
    }

    //velocity
    packFourBytes(10, inewSpd);
#endif

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



/**
 * @brief set speed limits
 *
 */
void mk1Data::packBF(int speedLimit[4], bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xbf;

    if ( setSettings == true) {
        writeBuf[4] = 0x80; // settings
    }

    if (Settings::coord[X].enabled == true) {
        float dnewSpdX  = 3600; // 3584?

        if (speedLimit[X] != 0 && Settings::coord[X].pulsePerMm != 0) {
            dnewSpdX = 7.2e8 / ((float)speedLimit[X] * Settings::coord[X].pulsePerMm);
        }

        packFourBytes(7, (int)dnewSpdX);
    }

    if (Settings::coord[Y].enabled == true) {
        float dnewSpdY = 3600;

        if (speedLimit[Y] != 0 && Settings::coord[Y].pulsePerMm != 0) {
            dnewSpdY = 7.2e8 / ((float)speedLimit[Y] * Settings::coord[Y].pulsePerMm);
        }

        packFourBytes(11, (int)dnewSpdY);
    }

    if (Settings::coord[Z].enabled == true) {
        float dnewSpdZ = 3600;

        if (speedLimit[Z] != 0 && Settings::coord[Z].pulsePerMm != 0) {
            dnewSpdZ = 7.2e8 / ((float)speedLimit[Z] * Settings::coord[Z].pulsePerMm);
        }

        packFourBytes(15, (int)dnewSpdZ);
    }

    if (Settings::coord[A].enabled == true) {
        float dnewSpdA = 3600;

        if (speedLimit[A] != 0 && Settings::coord[A].pulsePerMm != 0) {
            dnewSpdA = 7.2e8 / ((float)speedLimit[A] * Settings::coord[A].pulsePerMm);
        }

        packFourBytes(19, (int)dnewSpdA);
    }

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief command unknown....
 *
 */
void mk1Data::packC0(byte byte05, bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xC0;
    writeBuf[5] = byte05;

    if (send == true) {
        sendBinaryData();
    }
}

/**
 * @brief  unknown settings
 *
 */
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


/**
 * @brief chnage the current coordinates without moving
 *
 */
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

#if 0
// recalculate from mm/inch to pulses
void mk1Data::packCA(float _posX, float _posY, float _posZ, float _posA, int _speed, int _NumberInstruction, int _code, bool send)
{
    int xPulses = Settings::coord[X].posPulse(_posX);
    int yPulses = Settings::coord[Y].posPulse(_posY);
    int zPulses = Settings::coord[Z].posPulse(_posZ);
    int aPulses = Settings::coord[A].posPulse(_posA);

    packCA(xPulses, yPulses, zPulses, aPulses, _speed, _NumberInstruction, _code, send);
}
#endif

/**
 * @brief moving to the point
 *
 */
void mk1Data::packCA(moveParameters *params, bool send)
{
    int newInst = params->numberInstruction;

    cleanBuf(writeBuf);

    writeBuf[0] = 0xca;

    // save the number instruction
    packFourBytes(1, newInst);

    writeBuf[5] = params->movingCode;

    int pulsesX  = Settings::coord[X].posPulse(params->pos[X]);
    int pulsesY  = Settings::coord[Y].posPulse(params->pos[Y]);
    int pulsesZ  = Settings::coord[Z].posPulse(params->pos[Z]);
    int pulsesA  = Settings::coord[A].posPulse(params->pos[A]);

    //how many pulses
    packFourBytes(6, pulsesX );
    packFourBytes(10, pulsesY );
    packFourBytes(14, pulsesZ );
    packFourBytes(18, pulsesA );

    //axes xspeed
    packFourBytes(43, params->speed); // vector speed

    // offset 46
    packFourBytes(46, params->restPulses); // if 0x31 or  0x39 should be zero

    writeBuf[54] = 0x40;  //TODO: unknown byte

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief check length of tool
 *
 */
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
    int inewReturn = (int)(returnDistance * (float)Settings::coord[Z].pulsePerMm);

    //return to position
    packFourBytes(50, inewReturn);

    //unknown
    writeBuf[55] = 0x12;
    writeBuf[56] = 0x7A;

    if (send == true) {
        sendBinaryData();
    }
}

/**
 * @brief unknown settings
 *
 */
void mk1Data::packD3( bool send )
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xd3;
    writeBuf[5] = 0x01;

    if (send == true) {
        sendBinaryData();
    }
}


/**
 * @brief break of all operations
 *
 */
void mk1Data::packFF(bool send)
{
    cleanBuf(writeBuf);

    writeBuf[0] = 0xff;

    if (send == true) {
        sendBinaryData();
    }
}


