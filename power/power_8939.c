/*
 * Copyright (C) 2015-2016 The CyanogenMod Project
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

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define LOG_TAG "MSM8939 PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define NODE_MAX (64)

#define CPUFREQ_PATH "/sys/devices/system/cpu/cpu4/cpufreq/"
#define INTERACTIVE_PATH "/sys/devices/system/cpu/cpu4/cpufreq/interactive/"

#define LOW_POWER "/sys/module/cluster_plug/parameters/low_power_mode"
#define BIG_PATH "/sys/devices/system/cpu/cpu0/cpufreq/interactive/"
#define BOOST_FREQ "/sys/module/cpu_boost/parameters/input_boost_freq"
#define THERMAL_PATH "/sys/module/msm_thermal/parameters/"

#define CORE_TEMP_FAST "82"
#define CORE_TEMP_BLNC "78"

#define FREQ_TEMP_FAST "78"
#define FREQ_TEMP_BLNC "75"

#define BIG_TARGET_LOADS_FAST "40 800000:50 1113600:60 1344000:70"
#define BIG_TARGET_LOADS_BLNC "60 800000:70 1113600:80 1344000:85"

#define BIG_HISPEED_LOAD_FAST "60"
#define BIG_HISPEED_LOAD_BLNC "75"

#define BIG_HISPEED_FREQ_FAST "1497600"
#define BIG_HISPEED_FREQ_BLNC "1113600"

#define BOOST_FREQ_FAST "1344000"
#define BOOST_FREQ_BLNC "960000"
#define BOOST_FREQ_SLOW "800000"

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE,
    PROFILE_MAX
};

static int current_power_profile = -1;

static int sysfs_write2(char *path, char *s, bool verbose)
{
    char buf[80];
    int len;
    int ret = 0;
    int fd;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return -1 ;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        if (verbose) {
            strerror_r(errno, buf, sizeof(buf));
            ALOGE("Error writing to %s: %s\n", path, buf);
        }
        ret = -1;
    }

    close(fd);

    return ret;
}

static int sysfs_write(char *path, char *s)
{
    return sysfs_write2(path, s, true);
}

static void power_init(__attribute__((unused)) struct power_module *module)
{
    ALOGI("%s", __func__);
}

static bool check_governor(void)
{
    struct stat s;
    int err = stat(BIG_PATH, &s);
    if (err != 0) return false;
    if (S_ISDIR(s.st_mode)) return true;
    return false;
}

static void power_set_interactive(__attribute__((unused)) struct power_module *module, int on)
{
    if (current_power_profile != PROFILE_BALANCED)
        return;

    if (on) {
        sysfs_write(LOW_POWER, "0");
    } else {
        sysfs_write(LOW_POWER, "1");
    }
}

static void set_power_profile(int profile)
{
    bool gov_interactive;

    if (profile == current_power_profile)
        return;

    gov_interactive = check_governor();

    switch (profile) {
    case PROFILE_BALANCED:
        if (gov_interactive) {
            sysfs_write(BIG_PATH "go_hispeed_load", BIG_HISPEED_LOAD_BLNC);
            sysfs_write(BIG_PATH "hispeed_freq", BIG_HISPEED_FREQ_BLNC);
            sysfs_write(BIG_PATH "target_loads", BIG_TARGET_LOADS_BLNC);
            sysfs_write(THERMAL_PATH "core_limit_temp_degC", CORE_TEMP_BLNC);
            sysfs_write(THERMAL_PATH "limit_temp_degC", FREQ_TEMP_BLNC);
            sysfs_write(INTERACTIVE_PATH "go_hispeed_load",
                            profiles[PROFILE_BALANCED].go_hispeed_load);
            sysfs_write(INTERACTIVE_PATH "hispeed_freq",
                            profiles[PROFILE_BALANCED].hispeed_freq);
            sysfs_write(INTERACTIVE_PATH "min_sample_time",
                            profiles[PROFILE_BALANCED].min_sample_time);
            sysfs_write(INTERACTIVE_PATH "timer_rate",
                            profiles[PROFILE_BALANCED].timer_rate);
            sysfs_write(INTERACTIVE_PATH "above_hispeed_delay",
                            profiles[PROFILE_BALANCED].above_hispeed_delay);
            sysfs_write(INTERACTIVE_PATH "target_loads",
                            profiles[PROFILE_BALANCED].target_loads);
            sysfs_write(CPUFREQ_PATH "scaling_max_freq",
                            profiles[PROFILE_BALANCED].scaling_max_freq);
            sysfs_write(CPUFREQ_PATH "scaling_min_freq",
                            profiles[PROFILE_BALANCED].scaling_min_freq);
        }

        sysfs_write(LOW_POWER, "0");
        sysfs_write(BOOST_FREQ, BOOST_FREQ_BLNC);
        ALOGD("%s: set balanced mode", __func__);
        break;
    case PROFILE_HIGH_PERFORMANCE:
        if (gov_interactive) {
            sysfs_write(BIG_PATH "go_hispeed_load", BIG_HISPEED_LOAD_FAST);
            sysfs_write(BIG_PATH "hispeed_freq", BIG_HISPEED_FREQ_FAST);
            sysfs_write(BIG_PATH "target_loads", BIG_TARGET_LOADS_FAST);
            sysfs_write(THERMAL_PATH "core_limit_temp_degC", CORE_TEMP_FAST);
            sysfs_write(THERMAL_PATH "limit_temp_degC", FREQ_TEMP_FAST);
            sysfs_write(INTERACTIVE_PATH "go_hispeed_load",
                            profiles[PROFILE_HIGH_PERFORMANCE].go_hispeed_load);
            sysfs_write(INTERACTIVE_PATH "hispeed_freq",
                            profiles[PROFILE_HIGH_PERFORMANCE].hispeed_freq);
            sysfs_write(INTERACTIVE_PATH "min_sample_time",
                            profiles[PROFILE_HIGH_PERFORMANCE].min_sample_time);
            sysfs_write(INTERACTIVE_PATH "timer_rate",
                            profiles[PROFILE_HIGH_PERFORMANCE].timer_rate);
            sysfs_write(INTERACTIVE_PATH "above_hispeed_delay",
                            profiles[PROFILE_HIGH_PERFORMANCE].above_hispeed_delay);
            sysfs_write(INTERACTIVE_PATH "target_loads",
                            profiles[PROFILE_HIGH_PERFORMANCE].target_loads);
            sysfs_write(CPUFREQ_PATH "scaling_max_freq",
                            profiles[PROFILE_HIGH_PERFORMANCE].scaling_max_freq);
            sysfs_write(CPUFREQ_PATH "scaling_min_freq",
                            profiles[PROFILE_HIGH_PERFORMANCE].scaling_min_freq);

        }

        sysfs_write(LOW_POWER, "0");
        sysfs_write(BOOST_FREQ, BOOST_FREQ_FAST);
        ALOGD("%s: set performance mode", __func__);
        break;
    case PROFILE_POWER_SAVE:
        sysfs_write(LOW_POWER, "1");
        sysfs_write(BOOST_FREQ, BOOST_FREQ_SLOW);
        ALOGD("%s: set powersave mode", __func__);
        break;
    default:
        ALOGE("%s: unknown profile: %d", __func__, profile);
    }

    current_power_profile = profile;
}

static void power_hint( __attribute__((unused)) struct power_module *module,
                        __attribute__((unused)) power_hint_t hint,
                        void *data)
{
    switch (hint) {
    case POWER_HINT_INTERACTION:
        break;
    case POWER_HINT_SET_PROFILE:
        set_power_profile(*(int32_t *)data);
        break;
    case POWER_HINT_LOW_POWER:
        /* This hint is handled by the framework */
        break;
    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};


static int get_feature(__attribute__((unused)) struct power_module *module,
                       feature_t feature)
{
    if (feature == POWER_FEATURE_SUPPORTED_PROFILES) {
        return PROFILE_MAX;
    }
    return -1;
}

void set_feature(__attribute__((unused)) struct power_module *module, feature_t feature, int state)
{
    char tmp_str[NODE_MAX];
#ifdef TAP_TO_WAKE_NODE
    if (feature == POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        snprintf(tmp_str, NODE_MAX, "%d", state);
        sysfs_write(TAP_TO_WAKE_NODE, tmp_str);
    }
#endif
}

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_3,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "MSM8939 Power HAL",
        .author = "The CyanogenMod Project",
        .methods = &power_module_methods,
    },

    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
    .getFeature = get_feature,
    .setFeature = set_feature
};
