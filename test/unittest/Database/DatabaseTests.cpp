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

#include <database/database.hpp>
#include <database/entities.hpp>

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include "gtest/gtest.h"

#include <string>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

class DataBaseTest : public Database
{
public:
    const std::map<EntityId, std::shared_ptr<Host>>& hosts()
    {
        return hosts_;
    }

    const std::map<EntityId, std::shared_ptr<User>>& users()
    {
        return users_;
    }

    const std::map<EntityId, std::shared_ptr<Process>>& processes()
    {
        return processes_;
    }

    const std::map<EntityId, std::shared_ptr<Domain>>& domains()
    {
        return domains_;
    }
};

TEST(database, insert_host)
{
    /* Insert a host */
    DataBaseTest db;
    std::string host_name = "test_host";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);

    /* Check that the host is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 1);
    ASSERT_NE(hosts.find(host_id), hosts.end());
    ASSERT_EQ(host_name, hosts[host_id]->name);
}

TEST(database, insert_host_two)
{
    /* Insert two hosts */
    DataBaseTest db;
    std::string host_name = "test_host";
    std::string host_name_2 = "test_host_2";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    std::shared_ptr<Host> host_2 = std::make_shared<Host>(host_name_2);
    EntityId host_id = db.insert(host);
    EntityId host_id_2 = db.insert(host_2);

    /* Check that the hosts are inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 2);
    ASSERT_NE(hosts.find(host_id), hosts.end());
    ASSERT_NE(hosts.find(host_id_2), hosts.end());
    ASSERT_EQ(host_name, hosts[host_id]->name);
    ASSERT_EQ(host_name_2, hosts[host_id_2]->name);
}

TEST(database, insert_host_duplicated)
{
    /* Insert a host twice */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);
    ASSERT_THROW(db.insert(host), BadParameter);
}

TEST(database, insert_user_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::string user_name = "test_user";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    EntityId user_id = db.insert(user);

    /* Check that the user is correctly inserted in host */
    ASSERT_EQ(host->users.size(), 1);
    ASSERT_EQ(host->users[user_id].get(), user.get());

    /* Check that the user is correctly inserted users_ */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 1);
    ASSERT_NE(users.find(user_id), users.end());
    ASSERT_EQ(user_name, users[user_id]->name);
}

TEST(database, insert_user_two_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert two users */
    std::string user_name = "test_user";
    std::string user_name_2 = "test_user_2";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    std::shared_ptr<User> user_2 = std::make_shared<User>(user_name_2, host);
    EntityId user_id = db.insert(user);
    EntityId user_id_2 = db.insert(user_2);

    /* Check that the users are correctly inserted in host */
    ASSERT_EQ(host->users.size(), 2);
    ASSERT_EQ(host->users[user_id].get(), user.get());
    ASSERT_EQ(host->users[user_id_2].get(), user_2.get());

    /* Check that the users are correctly inserted users_ */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 2);
    ASSERT_NE(users.find(user_id), users.end());
    ASSERT_NE(users.find(user_id_2), users.end());
    ASSERT_EQ(user_name, users[user_id]->name);
    ASSERT_EQ(user_name_2, users[user_id_2]->name);
}

TEST(database, insert_user_duplicated)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user twice */
    std::shared_ptr<User> user = std::make_shared<User>("test_user", host);
    db.insert(user);
    ASSERT_THROW(db.insert(user), BadParameter);
}

TEST(database, insert_user_wrong_host)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert user with a non-inserted host */
    std::string user_name = "test_user";
    std::shared_ptr<Host> host_2 = std::make_shared<Host>("non-inserter_host");
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host_2);
    ASSERT_THROW(db.insert(user), BadParameter);
}

TEST(database, insert_user_duplicated_name)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::string user_name = "test_user";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    db.insert(user);

    /* Insert a user with duplicated name */
    std::shared_ptr<User> user_2 = std::make_shared<User>(user_name, host);
    ASSERT_THROW(db.insert(user_2), BadParameter);
}

TEST(database, insert_process_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::shared_ptr<User> user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_name = "test_process";
    std::string process_pid = "test_pid";
    std::shared_ptr<Process> process = std::make_shared<Process>(process_name, process_pid, user);
    EntityId process_id = db.insert(process);

    /* Check that the process is correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 1);
    ASSERT_EQ(user->processes[process_id].get(), process.get());

    /* Check that the process is correctly inserted processes_s */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 1);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_EQ(process_name, processes[process_id]->name);
    ASSERT_EQ(process_pid, processes[process_id]->pid);
}

TEST(database, insert_process_two_valid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert two processes */
    std::string process_name = "test_process";
    std::string process_name_2 = "test_process_2";
    std::string process_pid = "test_pid";
    std::string process_pid_2 = "test_pid_2";
    auto process = std::make_shared<Process>(process_name, process_pid, user);
    auto process_2 = std::make_shared<Process>(process_name_2, process_pid_2, user);
    EntityId process_id = db.insert(process);
    EntityId process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 2);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_name, processes[process_id]->name);
    ASSERT_EQ(process_name_2, processes[process_id_2]->name);
}

TEST(database, insert_process_duplicated)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process twice */
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST(database, insert_process_wrong_user)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert process with a non-inserted user */
    auto user_2 = std::make_shared<User>("non-inserter_user", host);
    auto process = std::make_shared<Process>("test_process", "test_pid", user_2);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST(database, insert_process_empty_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process with empty pid */
    auto process = std::make_shared<Process>("test_user", "", user);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST(database, insert_process_two_same_user_diff_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_pid = "test_pid";
    std::string process_pid_2 = "test_pid_2";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    auto process_2 = std::make_shared<Process>("test_process", process_pid_2, user);
    auto process_id = db.insert(process);
    auto process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 2);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_pid, processes[process_id]->pid);
    ASSERT_EQ(process_pid_2, processes[process_id_2]->pid);
}

TEST(database, insert_process_two_same_user_same_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_pid = "test_pid";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    db.insert(process);

    /* Insert a process in the same user with a duplicated pid */
    auto process_2 = std::make_shared<Process>("test_process", process_pid, user);
    ASSERT_THROW(db.insert(process_2), BadParameter);
}

TEST(database, insert_process_two_diff_user_same_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert two users */
    auto user = std::make_shared<User>("test_user", host);
    auto user_2 = std::make_shared<User>("test_user_2", host);
    db.insert(user);
    db.insert(user_2);

    /* Insert a process */
    std::string process_pid = "test_pid";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    auto process_id = db.insert(process);

    /* Insert a process in the same user with a duplicated pid */
    auto process_2 = std::make_shared<Process>("test_process", process_pid, user_2);
    auto process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 1);
    ASSERT_EQ(user_2->processes.size(), 1);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user_2->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_pid, processes[process_id]->pid);
    ASSERT_EQ(process_pid, processes[process_id_2]->pid);
}

TEST(database, insert_domain_valid)
{
    /* Insert a domain */
    DataBaseTest db;
    std::string domain_name = "test_domain";
    auto domain = std::make_shared<Domain>(domain_name);
    EntityId domain_id = db.insert(domain);

    /* Check that the domain is inserted correctly */
    auto domains = db.domains();
    ASSERT_EQ(domains.size(), 1);
    ASSERT_NE(domains.find(domain_id), domains.end());
    ASSERT_EQ(domain_name, domains[domain_id]->name);
}

TEST(database, insert_domain_two_valid)
{
    /* Insert two domains */
    DataBaseTest db;
    std::string domain_name = "test_domain";
    std::string domain_name_2 = "test_domain_2";
    auto domain = std::make_shared<Domain>(domain_name);
    auto domain_2 = std::make_shared<Domain>(domain_name_2);
    EntityId domain_id = db.insert(domain);
    EntityId domain_id_2 = db.insert(domain_2);

    /* Check that the domains are inserted correctly */
    auto domains = db.domains();
    ASSERT_EQ(domains.size(), 2);
    ASSERT_NE(domains.find(domain_id), domains.end());
    ASSERT_NE(domains.find(domain_id_2), domains.end());
    ASSERT_EQ(domain_name, domains[domain_id]->name);
    ASSERT_EQ(domain_name_2, domains[domain_id_2]->name);
}

TEST(database, insert_domain_duplicated)
{
    /* Insert a domain twice */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    ASSERT_THROW(db.insert(domain), BadParameter);
}

TEST(database, insert_domain_empty_name)
{
    /* Insert a domain with empty name */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("");
    ASSERT_THROW(db.insert(domain), BadParameter);
}

TEST(database, insert_domain_same_name)
{
    /* Insert two domains with same name */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_2 = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    ASSERT_THROW(db.insert(domain_2), BadParameter);
}

TEST(database, insert_invalid)
{
    /* Insert an entity */
    DataBaseTest db;
    std::shared_ptr<Entity> entity = std::make_shared<Entity>();
    EntityId entity_id = db.insert(entity);
    ASSERT_EQ(entity_id, EntityId::invalid());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
