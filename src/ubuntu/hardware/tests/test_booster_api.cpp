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

#include <csignal>
#include <cstdio>
#include <ctime>

namespace
{
void wait_for_sigint()
{
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);

    int sig = 0;
    do 
    {
        if (0 != sigwait(&signals, &sig))
            break;
    } while(sig != SIGINT);    
}
}

int main()
{
    auto booster = u_hardware_booster_new();

    if (not booster)
    {
        std::cerr << "Failed to acquire performance booster instance, aborting." << std::endl;
        return 1;
    }

    std::cout << "Enabling boosting, hit CTRL-C to disable ... ";
    {
        u_hardware_booster_enable_scenario(booster, U_HARDWARE_BOOSTER_SCENARIO_USER_INTERACTION);
        wait_for_sigint();
        u_hardware_booster_disable_scenario(booster, U_HARDWARE_BOOSTER_SCENARIO_USER_INTERACTION);
    }
    std::cout << "exiting";
    
    return 0;
}
