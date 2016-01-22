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
#include <utils/RefBase.h>

struct UbuntuHardwareBooster : public android::RefBase
{
    typedef void (*BoosterEnableScenario)(int);
    typedef void (*BoosterDisableScenario)(int);

    static BoosterEnableScenario load_booster_enable_scenario(void* dl_handle)
    {
        if (!dl_handle)
            return NULL;

        return static_cast<BoosterEnableScenario>(::dlsym(dl_handle, booster_enable_scenario_from_property()));
    }

    static BoosterDisableScenario load_booster_disable_scenario(void* dl_handle)
    {
        if (!dl_handle)
            return NULL;

        return static_cast<BoosterDisableScenario>(::dlsym(dl_handle, booster_disable_scenario_from_property()));
    }

    static const char* dl_path_from_property()
    {
        static char value[PROP_VALUE_MAX];
        static int rc = property_get("ubuntu.booster.dl", value, NULL);

        return rc == 0 ? NULL : &value[0];
    }

    static const char* booster_enable_scenario_from_property()
    {
        static char value[PROP_VALUE_MAX];
        static int rc = property_get("ubuntu.booster.enable", value, NULL);

        return rc == 0 ? NULL : &value[0];
    }

    static const char* booster_disable_scenario_from_property()
    {
        static char value[PROP_VALUE_MAX];
        static int rc = property_get("ubuntu.booster.disable", value, NULL);

        return rc == 0 ? NULL : &value[0];
    }
    
    UbuntuHardwareBooster()
            : dl_handle(dl_path_from_property(), RTLD_NOW),
              booster_enable_scenario(load_booster_enable_scenario(dl_handle)),
              booster_disable_scenario(load_booster_disable_scenario(dl_handle))
    {
    }

    ~UbuntuHardwareBooster()
    {
        if (dl_handle)
            dlclose(dl_handle);
    }
    
    void enable_scenario(UHardwareBoosterScenario scenario)
    {
        if (booster_enable_scenario)
            booster_enable_scenario(scenario);
    }

    void disable_scenario(UHardwareBoosterScenario)
    {
        if (booster_disable_scenario)
            booster_disable_scenario(scenario);
    }

    void* dl_handle;
    BoosterEnableScenario booster_enable_scenario;
    BoosterDisableScenario booster_disable_scenario;
};

UHardwareBooster*
u_hardware_booster_new()
{
    return new UbuntuHardwareBooster();
}

void
u_hardware_booster_ref(UHardwareBooster* booster)
{
    if (booster)
        booster->incStrong(NULL);
}

void
u_hardware_booster_unref(UHardwareBooster*)
{
    if (booster)
        booster->decStrong(NULL);
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
