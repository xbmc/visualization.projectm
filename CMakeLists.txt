cmake_minimum_required(VERSION 3.18)
project(visualization.projectm)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(projectM4 REQUIRED COMPONENTS Playlist)

set(DEPLIBS libprojectM::playlist)

include_directories(${includes}
                    ${PROJECTM_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways

set(PM_SOURCES src/Main.cpp)
set(PM_HEADERS src/Main.h)

build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
