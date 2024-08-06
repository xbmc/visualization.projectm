cmake_minimum_required(VERSION 3.21)

project(visualization.projectm)

find_package(Kodi REQUIRED)
find_package(projectM4 REQUIRED COMPONENTS Playlist)

list(APPEND DEPLIBS libprojectM::playlist)

include_directories(${includes}
                    ${PROJECTM_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways

set(PM_SOURCES src/Main.cpp)
set(PM_HEADERS src/Main.h)
set(PM_CUSTOM_DATA ${PROJECTM_PKGDATADIR})

build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
