#
# Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
#
# This file is part of OTB-SIRIUS
#
#     https://github.com/CS-SI/OTB-SIRIUS
#
# OTB-SIRIUS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OTB-SIRIUS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OTB-SIRIUS.  If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.2)
project(FrequencyResample)

set(FREQUENCY_RESAMPLE_VERSION "1.0.0")

message(STATUS "FreqZoom ${FREQUENCY_RESAMPLE_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
        # this definition is required to build with OTB
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

# set CXX flags
set(FREQUENCY_RESAMPLE_CXX_FLAGS "")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Clang or GCC
    set(FREQUENCY_RESAMPLE_CXX_FLAGS  "-Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # MSVC compiler
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FREQUENCY_RESAMPLE_CXX_FLAGS}")

add_subdirectory(third_party)

if(NOT OTB_SOURCE_DIR)
    find_package(OTB REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${OTB_CMAKE_DIR})
    include(OTBModuleExternal)
else()
    otb_module_impl()
endif()
