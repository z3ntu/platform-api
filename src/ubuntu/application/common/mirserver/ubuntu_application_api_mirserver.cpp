/*
 * Copyright (C) 2013-2014 Canonical Ltd
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
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

// C APIs
#include <ubuntu/application/init.h>
#include <ubuntu/application/instance.h>

extern "C"
{
void u_application_init(void *args)
{
    (void) args;
}

void u_application_finish()
{
}

UApplicationInstance* u_application_instance_new_from_description_with_options(UApplicationDescription* u_description, UApplicationOptions* u_options)
{
    (void) u_description;
    (void) u_options;
    return NULL;
}

void
u_application_instance_ref(UApplicationInstance *u_instance)
{
    (void) u_instance;
}

void
u_application_instance_unref(UApplicationInstance *u_instance)
{
    (void) u_instance;
}

void
u_application_instance_destroy(UApplicationInstance *instance)
{
    (void) instance;
}

void
u_application_instance_run(UApplicationInstance *instance)
{
    // TODO<papi>: What is this supposed to do? Seems to be no-op on hybris.
    (void) instance;
}

MirConnection*
u_application_instance_get_mir_connection(UApplicationInstance *instance)
{
    (void) instance;
    return nullptr;
}
}
