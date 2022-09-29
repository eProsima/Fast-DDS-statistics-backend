// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file arg_configuration.h
 *
 */

#ifndef _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_ARG_CONFIGURATION_H_
#define _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_ARG_CONFIGURATION_H_

#include <iostream>
#include <string>

#include "optionparser.h"

struct Arg : public option::Arg
{
    static void print_error(
            const char* msg1,
            const option::Option& opt,
            const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(
            const option::Option& option,
            bool msg)
    {
        if (msg)
        {
            print_error("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(
            const option::Option& option,
            bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10))
        {
        }
        if (endptr != option.arg && *endptr == 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum optionIndex
{
    UNKNOWN_OPT,
    HELP,
    DOMAIN_ID,
    N_BINS,
    T_INTERVAL,
    EXPOSER_ADDR
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "", Arg::None, "Usage: ROS2Prometheus \n\nGeneral options:" },
    { HELP, 0, "h", "help", Arg::None, "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "", Arg::None, "\nMonitor options:"},
    { DOMAIN_ID, 0, "d", "domain", Arg::Numeric,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { N_BINS, 0, "b", "bins", Arg::Numeric,
      "  -b <num> \t--bins=<num>  \tNumber of bins in which a time interval is divided (Default: 1)"
      " (0 => no mean calculation, return raw data)." },
    { T_INTERVAL, 0, "t", "time", Arg::Numeric,
      "  -t <num> \t--time=<num>  \tDuration in seconds of each time frame (Default: 5)." },

    { UNKNOWN_OPT, 0, "", "", Arg::None, "\nPrometheus options:"},
    { EXPOSER_ADDR, 0, "a", "exposer-addr", Arg::String,
      "  -e <IP:port> \tPrometheus exposer address (Default 127.0.0.1:8080)." },

    { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_ARG_CONFIGURATION_H_ */
