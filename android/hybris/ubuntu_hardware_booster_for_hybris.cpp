/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <ubuntu/hardware/booster.h>

#include <cutils/properties.h>

struct UbuntuHardwareBooster
{
    typedef void (*PerfServiceEnableScenario)(int);
    typedef void (*PerfServiceDisableScenario)(int);

    static PerfServiceEnableScenario load_perf_service_enable_scenario(void* dl_handle)
    {
        if (!dl_handle)
            return NULL;

        return static_cast<PerfServiceEnableScenario>(::dlsym(dl_handle, "PerfServiceNative_boostEnable"));
    }

    static PerfServiceDisableScenario load_perf_service_disable_scenario(void* dl_handle)
    {
        if (!dl_handle)
            return NULL;

        return static_cast<PerfServiceDisableScenario>(::dlsym(dl_handle, "PerfServiceNative_boostDisable"));
    }

    static const char* dl_path_from_property()
    {
        static char value[PROP_VALUE_MAX];

        if (property_get("ubuntu.perfservice.dl", value, NULL) == 0)
            return NULL;

        return &value[0];
    }
    
    UbuntuHardwareBooster()
            : dl_handle(dl_path_from_property(), RTLD_NOW),
              perf_service_enable_scenario(load_perf_service_enable_scenario(dl_handle)),
              perf_service_disable_scenario(load_perf_service_disable_scenario(dl_handle))
    {
    }

    ~UbuntuHardwareBooster()
    {
        if (dl_handle)
            dlclose(dl_handle);
    }
    
    void enable_scenario(UHardwareBoosterScenario scenario)
    {
        if (perf_service_enable_scenario)
            perf_service_enable_scenario(scenario);
    }

    void disable_scenario(UHardwareBoosterScenario)
    {
        if (perf_service_disable_scenario)
            perf_service_disable_scenario(scenario);
    }

    void* dl_handle;
    PerfServiceEnableScenario perf_service_enable_scenario;
    PerfServiceDisableScenario perf_service_disable_scenario;
};

UHardwareBooster*
u_hardware_booster_new()
{
    return new UbuntuHardwareBooster();
}

void
u_hardware_booster_ref(UHardwareBooster*)
{
    // TODO
}

void
u_hardware_booster_unref(UHardwareBooster*)
{
    // TODO
}

void
u_hardware_booster_enable_scenario(UHardwareBooster* booster, UHardwareBoosterScenario scenario)
{
    if (booster)
        booster->enable_scenario(scenario);
}

void
u_hardware_booster_disable_scenario(UHardwareBooster* booster, UHardwareBoosterScenario scenario)
{
    if (booster)
        booster->disable_scenario(scenario);
}
