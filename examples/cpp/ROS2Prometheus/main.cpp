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
 * @file main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "Monitor.h"


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

    // Monitor params
    int domain = 0;
    int n_bins = 1;
    int t_interval = 5;
    // Prometheus exporter params
    std::string exposer_addr = "127.0.0.1:8080";

    if (argc > 1)
    {
        // check if first argument is help, needed because we skip it when parsing
        if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
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

                case optionIndex::N_BINS:
                    n_bins = strtol(opt.arg, nullptr, 10);
                    if (n_bins < 0)
                    {
                        std::cerr << "ERROR: only non-negative values accepted for --bins option." << std::endl;
                        option::printUsage(fwrite, stdout, usage, columns);
                        return 1;
                    }
                    break;

                case optionIndex::T_INTERVAL:
                    t_interval = strtol(opt.arg, nullptr, 10);
                    if (t_interval < 1)
                    {
                        std::cerr << "ERROR: only positive values accepted for --time option." << std::endl;
                        option::printUsage(fwrite, stdout, usage, columns);
                        return 1;
                    }
                    break;

                case optionIndex::EXPOSER_ADDR:
                    exposer_addr = opt.arg;
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }


    Monitor monitor(static_cast<uint32_t>(domain), static_cast<uint32_t>(n_bins),
            static_cast<uint32_t>(t_interval), exposer_addr);
    if (monitor.init())
    {
        monitor.run();
    }

    return 0;
}
