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

#include "GData.h"
#include "usbwatcher.h"

class USBWatcher;

int LIBUSB_CALL hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                                 libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc;

    (void)libusb_get_device_descriptor(dev, &desc);

    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
        emit reinterpret_cast<USBWatcher *>(user_data)->USBConnected();
        return 0;
    }

    if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
        emit reinterpret_cast<USBWatcher *>(user_data)->USBDisconnected();
    }

    return 0;
}


USBWatcher::USBWatcher(QObject *p) : QThread(p)
{
    hotplug_register();
}


USBWatcher::~USBWatcher()
{
    if (handle) {
        libusb_hotplug_deregister_callback(NULL, handle);
        handle = 0;
    }

    libusb_exit(NULL);

    exit(0);
}


void USBWatcher::run()
{
    struct timeval zero_tv;
    zero_tv.tv_sec = 1;
    zero_tv.tv_usec = 0;

    while (usb_thread_loop) {
        libusb_handle_events_timeout_completed(usb_ctx, const_cast<timeval *>(&zero_tv), NULL);
    }

    exit(0);
}


void USBWatcher::hotplug_register()
{
    int rc;
    usb_thread_loop = true;

    libusb_init(&usb_ctx);
    rc = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                                          LIBUSB_HOTPLUG_ENUMERATE,
                                          MK1_VENDOR_ID,
                                          MK1_PRODUCT_ID,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          hotplug_callback,
                                          this,
                                          &handle);

    if (LIBUSB_SUCCESS != rc) {
        libusb_exit(NULL);

        exit(0);
    }
}

