/*
 * Copyright © 2013 Canonical Ltd.
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
#ifndef UBUNTU_HARDWARE_BOOSTER_H_
#define UBUNTU_HARDWARE_BOOSTER_H_

#include <ubuntu/visibility.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        /** @brief Boost device performance to account for user interacting with device. */
        U_HARDWARE_BOOSTER_SCENARIO_USER_INTERACTION
    } UbuntuHardwareBoosterScenario;

    /** @brief Enumerates all known performance boosting scenarios. */
    typedef UbuntuHardwareBoosterScenario UHardwareBoosterScenario;

    /** @cond */ struct UbuntuHardwareBooster; /** @endcond */
    /** @brief UbuntuHardwareBooster is an opaque type providing access to the performance booster capabilities of the HW and drivers. */
    typedef UbuntuHardwareBooster UHardwareBooster;

    /** @brief u_hardware_booster_new creates a new UHardwareBooster* instance with an initial referrence count of 1. */
    UBUNTU_DLL_PUBLIC UHardwareBooster*
    u_hardware_booster_new();
    /** @brief u_hardware_booster_ref increases the reference count of booster by 1. */
    UBUNTU_DLL_PUBLIC void
    u_hardware_booster_ref(UHardwareBooster*);
    /** @brief u_hardware_booster_unref decreases the reference count of booster by 1. */
    UBUNTU_DLL_PUBLIC void
    u_hardware_booster_unref(UHardwareBooster*);

    /** u_hardware_booster_enable_scenario informs booster to ramp up system performance to support scenario. */
    UBUNTU_DLL_PUBLIC void
    u_hardware_booster_enable_scenario(UHardwareBooster*, UHardwareBoosterScenario);
    /** u_hardware_booster_enable_scenario informs booster that scenario is no longer active. */
    UBUNTU_DLL_PUBLIC void
    u_hardware_booster_disable_scenario(UHardwareBooster*, UHardwareBoosterScenario);
        
#ifdef __cplusplus
}
#endif
    
#endif // UBUNTU_HARDWARE_BOOSTER_H_
