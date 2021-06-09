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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DLL_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DLL_HPP_

#include "config.h"

// normalize macros
#if !defined(FASTDDS_STATISTICS_BACKEND_DYN_LINK) && !defined(FASTDDS_STATISTICS_BACKEND_STATIC_LINK) \
    && !defined(EPROSIMA_ALL_DYN_LINK) && !defined(EPROSIMA_ALL_STATIC_LINK)
#define FASTDDS_STATISTICS_BACKEND_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_DYN_LINK) && !defined(FASTDDS_STATISTICS_BACKEND_DYN_LINK)
#define FASTDDS_STATISTICS_BACKEND_DYN_LINK
#endif

#if defined(FASTDDS_STATISTICS_BACKEND_DYN_LINK) && defined(FASTDDS_STATISTICS_BACKEND_STATIC_LINK)
#error Must not define both FASTDDS_STATISTICS_BACKEND_DYN_LINK and FASTDDS_STATISTICS_BACKEND_STATIC_LINK
#endif

#if defined(EPROSIMA_ALL_NO_LIB) && !defined(FASTDDS_STATISTICS_BACKEND_NO_LIB)
#define FASTDDS_STATISTICS_BACKEND_NO_LIB
#endif

// enable dynamic linking

#if defined(_WIN32)
#if defined(EPROSIMA_ALL_DYN_LINK) || defined(FASTDDS_STATISTICS_BACKEND_DYN_LINK)
#if defined(FASTDDS_STATISTICS_BACKEND_SOURCE)
#define FASTDDS_STATISTICS_BACKEND_DllAPI __declspec( dllexport )
#else
#define FASTDDS_STATISTICS_BACKEND_DllAPI __declspec( dllimport )
#endif // FASTDDS_STATISTICS_BACKEND_SOURCE
#else
#define FASTDDS_STATISTICS_BACKEND_DllAPI
#endif
#else
#define FASTDDS_STATISTICS_BACKEND_DllAPI
#endif // _WIN32

// enabling user dynamic linking
#if defined(_WIN32) && defined(FASTDDS_STATISTICS_BACKEND_USER_DLL_EXPORT)
  #define FASTDDS_STATISTICS_BACKEND_USERDllExport __declspec(dllexport)
#else
  #define FASTDDS_STATISTICS_BACKEND_USERDllExport
#endif

// Auto linking.

#if !defined(FASTDDS_STATISTICS_BACKEND_SOURCE) && !defined(EPROSIMA_ALL_NO_LIB) \
    && !defined(FASTDDS_STATISTICS_BACKEND_NO_LIB)

// Set properties.
#define EPROSIMA_LIB_NAME fastdds_statistics_backend

#if defined(EPROSIMA_ALL_DYN_LINK) || defined(FASTDDS_STATISTICS_BACKEND_DYN_LINK)
#define EPROSIMA_DYN_LINK
#endif

#include "eProsima_auto_link.h"
#endif // auto-linking disabled

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DLL_HPP_
