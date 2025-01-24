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

#include <fastdds_statistics_backend/types/types.hpp>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <database/entities.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(ddsvendor, check_dds_vendor)
{
    std::map<std::string, DdsVendor> dds_guids_vendors = {
        {"01.0f.03.04.05.06.07.08.09.0A.0B.0C.00.00.00.02", DdsVendor::FASTDDS},
        {"01.15.13.14.15.16.17.18.19.1A.1B.1C.00.00.00.02", DdsVendor::SAFEDDS},
        {"01.01.23.24.25.26.27.28.29.2A.2B.2C.00.00.00.03", DdsVendor::UNKNOWN},
        {"01.02.33.34.35.36.37.38.39.3A.3B.3C.00.00.00.02", DdsVendor::UNKNOWN},
        {"01.15.43.44.45.46.47.48.49.4A.4B.4C.00.00.00.03", DdsVendor::SAFEDDS},
        {"01.0f.53.54.55.56.57.58.59.5A.5B.5C.00.00.00.02", DdsVendor::FASTDDS}
    };

    for (auto& dds_guid_vendor : dds_guids_vendors)
    {
        EXPECT_EQ(dds_guid_vendor.second, DDSEntity::dds_vendor_by_guid(dds_guid_vendor.first));
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
