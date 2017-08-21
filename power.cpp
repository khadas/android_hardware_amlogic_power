/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_TAG "PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>
#define DEBUG 1

#define MP_GPU_CMD                      "/sys/class/mpgpu/mpgpucmd"
#define EARLY_SUSPEND_TRIGGER           "/sys/power/early_suspend_trigger"

struct private_power_module {
    power_module_t base;
    int gpuFp;
    int suspendFp;
};

namespace android {
namespace amlogic {

static void init (struct power_module *module) {
}

static void setInteractive (struct power_module *module, int on) {
    ALOGI("setInteractive ineractive:%s", (on==1)?"yes":"no");

    struct private_power_module *pm = (struct private_power_module *) module;
    if (pm->suspendFp < 0) {
        pm->suspendFp = open(EARLY_SUSPEND_TRIGGER, O_RDWR, 0644);
        if (pm->suspendFp < 0) {
            ALOGE("open %s fail, %s", EARLY_SUSPEND_TRIGGER, strerror(errno));
            return;
        }
    }

    //resume
    if (1 == on) {
        write(pm->suspendFp, "0", 1);
    }
    else {
        write(pm->suspendFp, "1", 1);
    }
}

static void powerHint(struct power_module *module, power_hint_t hint, void *data) {

    struct private_power_module *pm = (struct private_power_module *) module;

    const char *val = "preheat";
    static int bytes = 7;

    if (pm->gpuFp < 0) {
        pm->gpuFp = open(MP_GPU_CMD, O_RDWR, 0644);
        if (pm->gpuFp < 0) {
            ALOGE("open %s fail, %s", MP_GPU_CMD, strerror(errno));
            return;
        }
    }

    switch (hint) {
    case POWER_HINT_INTERACTION:
        if (pm->gpuFp >= 0) {
            int len = write(pm->gpuFp, val, bytes);
            if (DEBUG) {
                ALOGD("%s: write sucessfull, fd is %d\n", __FUNCTION__, pm->gpuFp);
            }

            if (len != bytes)
                ALOGE("write preheat faile");
        }
        break;

    default:
        break;
    }
}

/*
static void setFeature (struct power_module *module, feature_t feature, int state) {

}

static int getPlatformLowPowerStats (struct power_module *module,
        power_state_platform_sleep_state_t *list) {

    ALOGI("getPlatformLowPowerStats");
    return 0;
}

static ssize_t geNumberOfPlatformModes (struct power_module *module) {
    return 0;
}

static int getVoterList (struct power_module *module, size_t *voter) {
    return 0;
}
*/

} // namespace amlogic
} // namespace android


static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct private_power_module HAL_MODULE_INFO_SYM = {
    .base = {
        .common = {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = POWER_MODULE_API_VERSION_0_2,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = POWER_HARDWARE_MODULE_ID,
            .name = "AML Power HAL",
            .author = "aml",
            .methods = &power_module_methods,
        },
        .init = android::amlogic::init,
        .setInteractive = android::amlogic::setInteractive,
        .powerHint = android::amlogic::powerHint,
        //.setFeature = android::amlogic::setFeature,
        //.get_platform_low_power_stats = android::amlogic::getPlatformLowPowerStats,
        //.get_number_of_platform_modes = android::amlogic::geNumberOfPlatformModes,
        //.get_voter_list = android::amlogic::getVoterList,
    },
    .gpuFp = -1,
    .suspendFp = -1,
};
