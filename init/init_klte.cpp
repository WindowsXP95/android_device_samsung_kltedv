/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.
   Copyright (c) 2017-2020, The LineageOS Project. All rights reserved.

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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using android::init::property_set;

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[],
        char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void set_rild_libpath(char const *variant)
{
    std::string libpath("/system/vendor/lib/libsec-ril.");
    libpath += variant;
    libpath += ".so";

    property_override("rild.libpath", libpath.c_str());
}

void cdma_properties(char const *operator_alpha,
        char const *operator_numeric,
        char const *default_network,
        char const *rild_lib_variant)
{
    /* Dynamic CDMA Properties */
    property_set("ro.cdma.home.operator.alpha", operator_alpha);
    property_set("ro.cdma.home.operator.numeric", operator_numeric);
    property_set("ro.telephony.default_network", default_network);
    set_rild_libpath(rild_lib_variant);

    /* Static CDMA Properties */
    property_set("ril.subscription.types", "NV,RUIM");
    property_set("ro.telephony.default_cdma_sub", "1");
    property_set("telephony.lteOnCdmaDevice", "1");
}

void gsm_properties(char const *rild_lib_variant)
{
    set_rild_libpath(rild_lib_variant);

    property_set("telephony.lteOnGsmDevice", "1");
    property_set("ro.telephony.default_network", "9");
}

void vendor_load_properties()
{
    std::string bootloader = GetProperty("ro.bootloader", "");

    if (bootloader.find("G900I") == 0) {
        /* kltedv */
        property_override_dual("ro.build.fingerprint", "ro.vendor.build.fingerprint", "samsung/kltedv/klte:6.0.1/MMB29M/G900IDVS1CQE1:user/release-keys");
        property_override("ro.build.description", "kltedv-user 6.0.1 MMB29M G900IDVS1CQE1 release-keys");
        property_override_dual("ro.product.model", "ro.vendor.product.model", "SM-G900I");
        property_override_dual("ro.product.device", "ro.vendor.product.device", "klte");
        gsm_properties("dv");
    } else if (bootloader.find("G900P") == 0) {
        /* kltespr */
        property_override_dual("ro.build.fingerprint", "ro.vendor.build.fingerprint", "samsung/kltespr/kltespr:6.0.1/MMB29M/G900PVPS3CQD1:user/release-keys");
        property_override("ro.build.description", "kltespr-user 6.0.1 MMB29M G900PVPS3CQD1 release-keys");
        property_override_dual("ro.product.model", "ro.vendor.product.model", "SM-G900P");
        property_override_dual("ro.product.device", "ro.vendor.product.device", "kltespr");
        property_set("telephony.sms.pseudo_multipart", "1");
        cdma_properties("Sprint", "310120", "8", "spr");
    }

    std::string device = GetProperty("ro.product.device", "");
    LOG(ERROR) << "Found bootloader id " << bootloader <<  " setting build properties for "
        << device <<  " device" << std::endl;
}
