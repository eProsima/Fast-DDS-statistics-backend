/* Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file Notifiers.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_TYPES__NOTIFIERS_HPP
#define FASTDDS_STATISTICS_BACKEND_TYPES__NOTIFIERS_HPP

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>

// For script validation
#ifdef _WIN32
   #include <io.h>
   #define access    _access_s
#else
   #include <unistd.h>
#endif // ifdef _WIN32

namespace eprosima {
namespace statistics_backend {

typedef uint32_t NotifierId;
#define INVALID_NOTIFIER_ID (std::numeric_limits<uint32_t>::max())

// Available notifier kinds
enum class NotifierKind
{
    // Invalid notifier kind
    INVALID_NOTIFIER,
    // Script notifier
    SCRIPT_NOTIFIER
};

// Base class for notifiers, elements triggered by alerts
class Notifier
{
public:

    virtual ~Notifier() = default;

    // Pure virtual function for sending notifications
    virtual void notify(
            const std::string& message) = 0;

    // This function allows clean polymorphism
    virtual std::shared_ptr<Notifier> clone() const = 0;

    NotifierKind get_kind() const
    {
        return kind;
    }

protected:

    // Execute asynchronously if needed
    void execute_async(
            const std::string& command)
    {
        // We run the command in a detached thread to avoid blocking
        // and because we trust the notifier's implementation
        std::thread([command]()
                {
                    int ret = std::system(command.c_str());
                    (void) ret; // We ignore the return value
                }).detach();
    }

public:

    NotifierKind kind;
};


// Notifier implementation that calls a script with the message as argument
// when an alert is triggered
class ScriptNotifier : public Notifier
{
public:

    explicit ScriptNotifier(
            const std::string& scriptPath)
        : m_scriptPath(scriptPath)
    {
        kind = NotifierKind::SCRIPT_NOTIFIER;
    }

    void notify(
            const std::string& message) override
    {
        // Command is script + message as argument
        std::string command;

        #if defined(_WIN32)
        // Scripts in windows are called using cmd
        command = "cmd /C \"" + m_scriptPath + " \"" + message + "\"";
        #else
        // Linux execution way
        command = m_scriptPath + " \"" + message + "\"";
        #endif // if defined(_WIN32)

        // Execute the script asynchronously and result is not checked
        execute_async(command.c_str());
    }

    std::shared_ptr<Notifier> clone() const override
    {
        return std::make_shared<ScriptNotifier>(static_cast<const ScriptNotifier&>(*this));
    }

    static bool is_valid_script(
            const std::string& script_path)
    {
        if (script_path.empty())
        {
            return false;
        }

        #ifdef _WIN32
        // In Windows we dont check if the path is executable, only if it exists
        // TODO (ecuesta): Implement a more robust check for script validity
        return access(script_path.c_str(), 0) == 0;
        #else
        // In Linux we check if the file exists and is executable
        return access(script_path.c_str(), X_OK) == 0;
        #endif // ifdef _WIN32
    }

    std::string get_script_path() const
    {
        return m_scriptPath;
    }

private:

    std::string m_scriptPath;
};

// NOTE: Other notifiers shall be added here, like email specific, webhooks. etc.

class NotifierManager
{

public:

    NotifierId add_notifier(
            const Notifier& notifier)
    {
        NotifierId id = next_id++;
        // Making use of clone function so that each derived notifier is copied correctly
        m_notifiers[id] = notifier.clone();
        return id;
    }

    void remove_notifier(
            NotifierId notifier)
    {
        auto it = m_notifiers.find(notifier);
        if (it != m_notifiers.end())
        {
            m_notifiers.erase(it);
        }
    }

    void notify(
            const NotifierId& id,
            const std::string& message)
    {
        auto it = m_notifiers.find(id);
        if (it != m_notifiers.end())
        {
            it->second->notify(message);
        }
    }

    std::shared_ptr<Notifier> get_notifier(
            NotifierId id) const
    {
        auto it = m_notifiers.find(id);
        if (it != m_notifiers.end())
        {
            return it->second;
        }
        return nullptr;
    }

private:

    std::map<NotifierId, std::shared_ptr<Notifier>> m_notifiers;
    NotifierId next_id{0};
};

} // namespace statistics_backend
} // namespace eprosima


#endif // ifndef FASTDDS_STATISTICS_BACKEND_TYPES__NOTIFIERS_HPP
