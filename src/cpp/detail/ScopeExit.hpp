// Copyright 2015-2020 Open Source Robotics Foundation, Inc.
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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_SCOPE_EXIT_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_SCOPE_EXIT_HPP_

#include <utility>

namespace eprosima {
namespace statistics_backend {
namespace details {

template<typename CallableT>
class ScopeExit final
{
public:

    explicit ScopeExit(
            CallableT&& callable)
        : callable_(std::forward<CallableT>(callable))
    {
    }

    ScopeExit(
            const ScopeExit&) = delete;
    ScopeExit(
            ScopeExit&&) = default;

    ScopeExit& operator =(
            const ScopeExit&) = delete;
    ScopeExit& operator =(
            ScopeExit&&) = default;

    ~ScopeExit()
    {
        if (!cancelled_)
        {
            callable_();
        }
    }

    void cancel()
    {
        cancelled_ = true;
    }

private:

    CallableT callable_;
    bool cancelled_{false};
};

template<typename CallableT>
ScopeExit<CallableT>
make_scope_exit(
        CallableT&& callable)
{
    return ScopeExit<CallableT>(std::forward<CallableT>(callable));
}

}  // namespace details
}  // namespace statistics_backend
}  // namespace eprosima

#define EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(code) \
    eprosima::statistics_backend::details::make_scope_exit([&]() {code;})

#endif  // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_SCOPE_EXIT_HPP_
