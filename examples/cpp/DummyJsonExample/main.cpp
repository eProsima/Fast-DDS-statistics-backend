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

#include <iostream>
#include <chrono>
#include <thread>

#include <fastdds_statistics_backend/nlohmann-json/json.hpp>

void sleep_ms(unsigned int n)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(n));
}

void sleep_s(unsigned int n)
{
    sleep_ms(n * 1000);
}

int main(
        int argc,
        char** argv)
{
    unsigned int n_secs = 0;
    // Get N for seconds to wait in each step
    if(argc != 2)
    {
        return 1;
    }
    else
    {
        n_secs = std::strtol(argv[1], nullptr, 10);
    }

    std::cout << "STARTING" << std::endl;

    for (int i=0; i<3; i++)
    {
        sleep_s(n_secs);

        // Create json object
        std::cout << "Create object " << i << std::endl;
        nlohmann::json json_object;
        sleep_s(n_secs);

        // Populate json
        std::cout << "Populate object" << std::endl;
        json_object["value0"] = true;
        json_object["value1"] = 3;
        json_object["value2"] = nlohmann::json();
        json_object["value2"]["other value"] = "value";
        sleep_s(n_secs);

        // Populate again with many values
        std::cout << "Dense populate object" << std::endl;
        for (int j=0; j<1000; j++)
        {
            json_object[std::to_string(j)] =
                "veeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeery long string.";
        }
        sleep_s(n_secs);

        // Remove object
        // std::cout << "Remove object: " << json_object << std::endl;
        std::cout << "Remove object" << std::endl;
    }

    sleep_s(n_secs);
    std::cout << "FINISHING" << std::endl;

    return 0;
}
