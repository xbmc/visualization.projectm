project(visualization.projectm)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
if(PACKAGE_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH} ${PACKAGE_CONFIG_PATH})
endif()

find_package(kodi REQUIRED)
find_package(OpenGL REQUIRED)

if(NOT APPLE)
  find_package(GLEW REQUIRED)
  set(EXTRA_INCLUDES ${GLEW_INCLUDE_DIR})
  set(EXTRA_LIBRARIES ${GLEW_LIBRARIES})
endif()

find_package(ProjectM REQUIRED)

add_definitions(-DHAS_SDL_OPENGL)

set(INCLUDES ${OpenGL_INCLUDE_DIR}
             ${EXTRA_INCLUDES}
             ${KODI_INCLUDE_DIR}
             ${PROJECTM_INCLUDE_DIRS})

set(DEPLIBS ${OPENGL_LIBRARIES}
            ${EXTRA_LIBRARIES}
            ${PROJECTM_LIBS}
            ${extra_libs})

set(PM_SOURCES src/Main.cpp)

include_directories(${INCLUDES})

build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
