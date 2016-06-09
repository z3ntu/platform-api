/*
 * Copyright (C) 2013 Canonical Ltd
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

#include "application_options_mir_priv.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <tuple>
#include <iostream>
#include <fstream>
#include <string>

#include <unistd.h>

extern int opterr;

namespace uam = ubuntu::application::mir;

UApplicationOptions* uam::Options::as_u_application_options()
{
    return static_cast<UApplicationOptions*>(this);
}
uam::Options* uam::Options::from_u_application_options(UApplicationOptions* options)
{
    return static_cast<uam::Options*>(options);
}

namespace
{

void print_help_and_exit()
{
    printf("Usage: executable "
           "[--desktop_file_hint=absolute_path_to_desktop_file]\n");
    exit(EXIT_SUCCESS);
}

}

extern "C"
{
UApplicationOptions*
u_application_options_new_from_cmd_line(int argc, char** argv)
{
    static const int uninteresting_flag_value = 0;
    static struct option long_options[] =
    {        
        {"desktop_file_hint", required_argument, NULL, uninteresting_flag_value},
        {"help", no_argument, NULL, uninteresting_flag_value},
        {0, 0, 0, 0}
    };

    static const int desktop_file_hint_index = 2;
    static const int help_index = 3;

    auto app_options = new uam::Options;

    // Construct legacy desktop file path
    std::string desktop_file_name;
    if (getenv("APP_ID"))
    {
        desktop_file_name = getenv("APP_ID");
        desktop_file_name.append(".desktop");
    }

    // Click desktop path
    std::string local_path;
    if (getenv("APP_DESKTOP_FILE_PATH"))
        local_path = getenv("APP_DESKTOP_FILE_PATH");;        

    std::ifstream infile;
    std::string search_paths[2] = {"/usr/share/applications/", local_path};
    search_paths[0].append(desktop_file_name);
    std::string line;
    for (std::string path : search_paths)
    {
        if (path.empty())
            continue;

        infile.open(path);
        if (!infile)
            continue;

        std::getline(infile, line);
        while (std::getline(infile, line))
        {
            if (infile.eof())
                break;
          
            // Check for non key-value line 
            if (line.find("=") == std::string::npos)
                continue;
            
            std::string lhs = line.substr(0,line.find("="));
            std::string rhs = line.substr(line.find("="));
        }
    }

    // Do not complain about options we don't know
    // the app may have other params
    int orig_opterr = opterr;
    opterr = 0;
    while(true)
    {
        int option_index = 0;

        int c = getopt_long(argc,
                            argv,
                            "",
                            long_options,
                            &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            // If this option set a flag, do nothing else now.
            if (long_options[option_index].flag != 0)
                break;
            if (option_index == help_index)
                print_help_and_exit();
            if (optarg)
            {
                switch(option_index)
                {
                case desktop_file_hint_index:
                    app_options->desktop_file = std::string(optarg);
                    break;                
                }
            }
            break;
        case '?':
            break;
        }
    }
    opterr = orig_opterr;
    
    return app_options->as_u_application_options();
}

void
u_application_options_destroy(UApplicationOptions* u_options)
{
    auto options = uam::Options::from_u_application_options(u_options);
    delete options;
}

UApplicationOperationMode
u_application_options_get_operation_mode(UApplicationOptions *u_options)
{
    auto options = uam::Options::from_u_application_options(u_options);
    return options->operation_mode;
}
}
