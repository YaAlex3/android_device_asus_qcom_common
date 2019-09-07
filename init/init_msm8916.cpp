/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.
   Copyright (C) 2016 The CyanogenMod Project.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/properties.h>
#include "vendor_init.h"
#include "property_service.h"

using android::base::GetProperty;
using android::init::property_set;

char const *device;
char const *family;
char const *heapstartsize;
char const *heapgrowthlimit;
char const *heapsize;
char const *heapminfree;

void check_device()
{
    if (is_target_8916()) {
      device = "Z010D"; // 8916
    } else {
      device = "Z010DD"; // 8939
    }
    // from - phone-xhdpi-2048-dalvik-heap.mk
    heapstartsize = "8m";
    heapgrowthlimit = "192m";
    heapsize = "512m";
    heapminfree = "512k";
    if (sys.totalram > 2048ull * 1024 * 1024) {
      // from - phone-xxhdpi-3072-dalvik-heap.mk
      heapstartsize = "8m";
      heapgrowthlimit = "288m";
      heapsize = "768m";
      heapminfree = "512k";
    } else if (sys.totalram > 1024ull * 1024 * 1024) {
      // from - phone-xxhdpi-2048-dalvik-heap.mk
      heapstartsize = "16m";
      heapgrowthlimit = "192m";
      heapsize = "512m";
      heapminfree = "2m";
    } else {
      // from - phone-xhdpi-1024-dalvik-heap.mk
      heapstartsize = "8m";
      heapgrowthlimit = "96m";
      heapsize = "256m";
      heapminfree = "2m";
    }
}

bool is_target_8916()
{
    int fd;
    int soc_id = -1;
    char buf[10] = { 0 };

    if (access("/sys/devices/soc0/soc_id", F_OK) == 0)
        fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
    else
        fd = open("/sys/devices/system/soc/soc0/id", O_RDONLY);

    if (fd >= 0 && read(fd, buf, sizeof(buf) - 1) != -1)
        soc_id = atoi(buf);

    close(fd);
    return soc_id == 206 || (soc_id >= 247 && soc_id <= 250);
}

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void vendor_load_properties()
{
    char b_description[PROP_VALUE_MAX], b_fingerprint[PROP_VALUE_MAX];
    char p_carrier[PROP_VALUE_MAX], p_device[PROP_VALUE_MAX];

    std::string platform = GetProperty("ro.board.platform", "");
    if (platform != ANDROID_TARGET)
        return;

    check_device();

    property_override_dual("ro.build.fingerprint", "ro.vendor.build.fingerprint", "asus/WW_Phone/ASUS_Z010_2:6.0.1/MMB29P/13.8.26.80-20161230:user/release-keys");
    property_override_dual("ro.product.device", "ro.vendor.product.device", device);
    property_override_dual("ro.product.model", "ro.vendor.product.model", "ZC550KL");

    if (is_target_8916()) {
        property_set("debug.hwui.use_buffer_age", "false");
        property_set("ro.opengles.version", "196608");
    } else {
        property_set("ro.opengles.version", "196610");
        property_set("debug.hwui.use_buffer_age", "false");
    }
}