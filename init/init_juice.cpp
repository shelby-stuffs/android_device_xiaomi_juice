/*
 * Copyright (C) 2023 Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include <string.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/sysinfo.h>
#include <android-base/properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using std::string;

std::string fingerprint_to_description(std::string fingerprint);

void property_override(string prop, string value)
{
    auto pi = (prop_info*) __system_property_find(prop.c_str());

    if (pi != nullptr)
        __system_property_update(pi, value.c_str(), value.size());
    else
        __system_property_add(prop.c_str(), prop.size(), value.c_str(), value.size());
}

void set_ro_build_prop(const string &source, const string &prop,
                       const string &value, bool product = false) {
    string prop_name;

    if (product)
        prop_name = "ro.product." + source + prop;
    else
        prop_name = "ro." + source + "build." + prop;

    property_override(prop_name.c_str(), value.c_str());
}

void set_device_props(const string model, const string name, const string marketname,
                      const string mod_device, const string fingerprint, const string description) {
    // list of partitions to override props
    string source_partitions[] = { "", "bootimage.", "product.""system.",
                                   "system_ext.", "vendor." };

    for (const string &source : source_partitions) {
        set_ro_build_prop(source, "model", model, true);
        set_ro_build_prop(source, "name", name, true);
        set_ro_build_prop(source, "marketname", marketname, true);
    }
    property_override("ro.product.mod_device", mod_device.c_str());
    property_override("bluetooth.device.default_name", marketname.c_str());
    property_override("vendor.usb.product_string", marketname.c_str());
	property_override("ro.build.fingerprint", fingerprint.c_str());
	property_override("ro.build.description", description.c_str());
    property_override("ro.bootimage.build.fingerprint", fingerprint.c_str());
}

void vendor_load_properties()
{
    // Detect device and configure properties
    string hwname = GetProperty("ro.boot.product.hardware.sku", "");

    if (hwname == "citrus") {
        set_device_props("M2010J19CG", "citrus", "POCO M3", "citrus_global",
		"POCO/citrus_global/citrus:11/RKQ1.201004.002/V12.5.7.0.RJFMIXM:user/release-keys", "citrus_global-user 11 RKQ1.201004.002 V12.5.7.0.RJFMIXM release-keys");
    } else if (hwname == "lime") {
        set_device_props("M2010J19SG", "lime", "Redmi 9T", "lime_global",
		"Redmi/lime/lime:12/RKQ1.211130.001/V13.0.2.0.SJQMIXM:user/release-keys", "lime-user 12 RKQ1.211130.001 V13.0.2.0.SJQMIXM release-keys");
    } else if (hwname == "lemon") {
        set_device_props("M2010J19SY", "lemon", "Redmi 9T NFC", "lemon_global",
		"Redmi/lemon_global/lemon:12/RKQ1.211130.001/V13.0.2.0.SJQMIXM:user/release-keys", "lemon_global-user 12 RKQ1.211130.001 V13.0.2.0.SJQMIXM release-keys");
    } else if (hwname == "pomelo") {
        set_device_props("M2010J19SL", "pomelo", "Redmi Note 9 4G", "pomelo_global",
		"Redmi/pomelo_global/pomelo:12/RKQ1.211130.001/V13.0.2.0.SJQMIXM:user/release-keys", "pomelo_global-user 12 RKQ1.211130.001 V13.0.2.0.SJQMIXM release-keys");
    }

    // Set hardware revision
    property_override("ro.boot.hardware.revision", GetProperty("ro.boot.hwversion", "").c_str());

    // Set dalvik heap configuration
    string heapstartsize, heapgrowthlimit, heapsize, heapminfree,
			heapmaxfree, heaptargetutilization;

    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram > 5072ull * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram > 3072ull * 1024 * 1024) {
        // from - phone-xhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        // from - phone-xhdpi-2048-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.75";
        heapminfree = "512k";
        heapmaxfree = "8m";
    }

    property_override("dalvik.vm.heapstartsize", heapstartsize);
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_override("dalvik.vm.heapsize", heapsize);
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", heapminfree);
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);
}