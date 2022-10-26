// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"
#include "Monitor.h"

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER,
    MONITOR
};


int main(
        int argc,
        char** argv)
{
    int columns;

#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "COLUMNS") == 0 && buf != nullptr)
    {
        columns = strtol(buf, nullptr, 10);
        free(buf);
    }
    else
    {
        columns = 80;
    }
#else
    columns = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 80;
#endif // if defined(_WIN32)

    EntityType type = PUBLISHER;
    int count = 0;
    long sleep = 100;
    int domain = 0;
    // Monitor params
    int n_bins = 1;
    int t_interval = 5;
    bool reset = false;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "publisher"))
        {
            type = PUBLISHER;
        }
        else if (!strcmp(argv[1], "subscriber"))
        {
            type = SUBSCRIBER;
        }
        else if (!strcmp(argv[1], "monitor"))
        {
            type = MONITOR;
        }
        // check if first argument is help, needed because we skip it when parsing
        else if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }
        else
        {
            std::cerr << "ERROR: first argument can only be <publisher|subscriber|monitor>" << std::endl;
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        argc -= (argc > 0);
        argv += (argc > 0); // skip program name argv[0] if present
        --argc; ++argv; // skip pub/sub argument
        option::Stats stats(usage, argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

        if (parse.error())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        if (options[optionIndex::HELP])
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case optionIndex::HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case optionIndex::DOMAIN_ID:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::SAMPLES:
                    if (type == MONITOR)
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        count = strtol(opt.arg, nullptr, 10);
                    }
                    break;

                case optionIndex::INTERVAL:
                    if (type == PUBLISHER)
                    {
                        sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::N_BINS:
                    if (type == MONITOR)
                    {
                        n_bins = strtol(opt.arg, nullptr, 10);
                        if (n_bins < 0)
                        {
                            std::cerr << "ERROR: only nonnegative values accepted for --bins option." << std::endl;
                            option::printUsage(fwrite, stdout, usage, columns);
                            return 1;
                        }
                    }
                    else
                    {
                        print_warning("monitor", opt.name);
                    }
                    break;

                case optionIndex::T_INTERVAL:
                    if (type == MONITOR)
                    {
                        t_interval = strtol(opt.arg, nullptr, 10);
                        if (t_interval < 1)
                        {
                            std::cerr << "ERROR: only positive values accepted for --time option." << std::endl;
                            option::printUsage(fwrite, stdout, usage, columns);
                            return 1;
                        }
                    }
                    else
                    {
                        print_warning("monitor", opt.name);
                    }
                    break;

                case optionIndex::RESET:
                    if (type == MONITOR)
                    {
                        reset = true;
                    }
                    else
                    {
                        print_warning("monitor", opt.name);
                    }
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }
    else
    {
        std::cerr << "ERROR: <publisher|subscriber|monitor> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(static_cast<uint32_t>(domain)))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(static_cast<uint32_t>(count), static_cast<uint32_t>(domain)))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
        case MONITOR:
        {
            Monitor monitor;
            if (monitor.init(static_cast<uint32_t>(domain), static_cast<uint32_t>(n_bins),
                    static_cast<uint32_t>(t_interval), reset))
            {
                monitor.run();
            }
            break;
        }
    }
    return 0;
}
