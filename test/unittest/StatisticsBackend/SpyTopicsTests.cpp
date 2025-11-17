// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <Monitor.hpp>
#include <types/Alerts.hpp>
#include <types/app_names.h>
#include <types/JSONTags.h>
#include <types/Notifiers.hpp>
#include <types/types.hpp>
#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

class statistics_backend_spy_tests : public ::testing::TestWithParam<std::tuple<EntityKind, size_t,
            std::vector<size_t>>>
{
public:

    using TestId = PopulateDatabase::TestId;

    void SetUp()
    {
        db = new DataBaseTest;
        entities = PopulateDatabase::populate_database(*db);
    }

    void TearDown()
    {
        if (!StatisticsBackendTest::unset_database())
        {
            delete db;
        }
    }

    DataBaseTest* db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};


// Check the get_info StatisticsBackend method
TEST_F(statistics_backend_spy_tests, spy_check_datareader_creation)
{
    StatisticsBackendTest::set_database(db);

    ASSERT_ANY_THROW(StatisticsBackendTest::get_info(entities[0]->id));

    // Erase invalid entity
    entities.erase(0);

    for (auto pair : entities)
    {
        std::shared_ptr<const Entity> entity = pair.second;
        Info info = StatisticsBackendTest::get_info(entity->id);

        // Check generic info
        // Once the info is checked, it is erased so the final check is confirm that the info is empty (there is no
        // more information than the expected)
        EXPECT_EQ(entity->id, EntityId(info[ID_TAG]));
        info.erase(ID_TAG);
        EXPECT_EQ(entity_kind_str[(int)entity->kind], info[KIND_TAG]);
        info.erase(KIND_TAG);
        EXPECT_EQ(entity->name, info[NAME_TAG]);
        info.erase(NAME_TAG);
        EXPECT_EQ(entity->alias, info[ALIAS_TAG]);
        info.erase(ALIAS_TAG);
        EXPECT_EQ(entity->active, info[ALIVE_TAG]);
        info.erase(ALIVE_TAG);
        EXPECT_EQ(entity->metatraffic, info[METATRAFFIC_TAG]);
        info.erase(METATRAFFIC_TAG);
        EXPECT_EQ(discovery_source_str[(int)entity->discovery_source], info[DISCOVERY_SOURCE_TAG]);
        info.erase(DISCOVERY_SOURCE_TAG);
        EXPECT_EQ(status_level_str[(int)entity->status], info[STATUS_TAG]);
        info.erase(STATUS_TAG);

        // Check specific info
        switch (entity->kind)
        {
            case EntityKind::PROCESS:
            {
                std::shared_ptr<const Process> process =
                        std::dynamic_pointer_cast<const Process>(entity);
                EXPECT_EQ(process->pid, info[PID_TAG]);
                info.erase(PID_TAG);
                break;
            }
            case EntityKind::TOPIC:
            {
                std::shared_ptr<const Topic> topic =
                        std::dynamic_pointer_cast<const Topic>(entity);
                EXPECT_EQ(topic->data_type, info[DATA_TYPE_TAG]);
                info.erase(DATA_TYPE_TAG);
                break;
            }
            case EntityKind::PARTICIPANT:
            {
                std::shared_ptr<const DomainParticipant> participant =
                        std::dynamic_pointer_cast<const DomainParticipant>(entity);
                EXPECT_EQ(participant->guid, info[GUID_TAG]);
                info.erase(GUID_TAG);
                EXPECT_EQ(participant->qos, info[QOS_TAG]);
                info.erase(QOS_TAG);
                EXPECT_EQ(app_id_str[(int)participant->app_id], info[APP_ID_TAG]);
                info.erase(APP_ID_TAG);
                EXPECT_EQ(participant->app_metadata, info[APP_METADATA_TAG]);
                info.erase(APP_METADATA_TAG);
                EXPECT_EQ(participant->dds_vendor, info[DDS_VENDOR_TAG]);
                info.erase(DDS_VENDOR_TAG);
                EXPECT_EQ(participant->original_domain, info[ORIGINAL_DOMAIN_TAG]);
                info.erase(ORIGINAL_DOMAIN_TAG);

                // Obtain the locators list associated to the participant's endpoints
                std::vector<std::string> locators;
                for (auto reader : participant->data_readers)
                {
                    for (auto locator : reader.second.get()->locators)
                    {
                        locators.push_back(locator.second.get()->name);
                    }
                }
                for (auto writer : participant->data_writers)
                {
                    for (auto locator : writer.second.get()->locators)
                    {
                        locators.push_back(locator.second.get()->name);
                    }
                }
                // Remove duplicates
                auto last = std::unique(locators.begin(), locators.end(), [](
                                    const std::string& first,
                                    const std::string& second)
                                {
                                    return first.compare(second) == 0;
                                });
                locators.erase(last, locators.end());

                // Check that every locator is included in the Info object
                for (auto locator_name : locators)
                {
                    auto locator_it = std::find(info[LOCATOR_CONTAINER_TAG].begin(),
                                    info[LOCATOR_CONTAINER_TAG].end(), locator_name);
                    ASSERT_NE(locator_it, info[LOCATOR_CONTAINER_TAG].end());
                    info[LOCATOR_CONTAINER_TAG].erase(locator_it);
                }
                EXPECT_TRUE(info[LOCATOR_CONTAINER_TAG].empty());
                info.erase(LOCATOR_CONTAINER_TAG);
                break;
            }
            case EntityKind::DOMAIN:
            {
                std::shared_ptr<const Domain> domain =
                        std::dynamic_pointer_cast<const Domain>(entity);
                EXPECT_EQ(domain->domain_id, info[DOMAIN_ID_TAG]);
                info.erase(DOMAIN_ID_TAG);
                break;
            }
            case EntityKind::DATAWRITER:
            case EntityKind::DATAREADER:
            {
                std::shared_ptr<const DDSEntity> dds_entity =
                        std::dynamic_pointer_cast<const DDSEntity>(entity);
                EXPECT_EQ(dds_entity->guid, info[GUID_TAG]);
                info.erase(GUID_TAG);
                EXPECT_EQ(dds_entity->qos, info[QOS_TAG]);
                info.erase(QOS_TAG);
                EXPECT_EQ(app_id_str[(int)dds_entity->app_id], info[APP_ID_TAG]);
                info.erase(APP_ID_TAG);
                EXPECT_EQ(dds_entity->app_metadata, info[APP_METADATA_TAG]);
                info.erase(APP_METADATA_TAG);
                EXPECT_EQ(dds_entity->dds_vendor, info[DDS_VENDOR_TAG]);
                info.erase(DDS_VENDOR_TAG);
                break;
            }
            default:
            {
                break;
            }
        }
        EXPECT_TRUE(info.empty());
    }
}

TEST_F(statistics_backend_tests, spy_check_datareader_creation)
{

}
