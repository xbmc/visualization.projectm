PROJECT(visualization.projectm)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(xbmc REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(ProjectM REQUIRED)

add_definitions(-DHAS_SDL_OPENGL)
set(DATA_PATH "/usr/share/fymc/addons/visualization.projectm/resources")
add_definitions("-DDATAPATH=\"${DATAPATH}\"")

set(INCLUDES ${OpenGL_INCLUDE_DIR}
             ${GLEW_INCLUDE_DIR}
             ${XBMC_INCLUDE_DIR}
             ${PROJECTM_INCLUDE_DIRS})

set(DEPLIBS ${OPENGL_LIBRARIES}
            ${GLEW_LIBRARIES}
            ${PROJECTM_LDFLAGS})

set(PM_SOURCES src/Main.cpp)

include_directories(${INCLUDES})

build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
