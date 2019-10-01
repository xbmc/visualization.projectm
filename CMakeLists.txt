cmake_minimum_required(VERSION 3.5)
project(visualization.projectm)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
if(PACKAGE_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH} ${PACKAGE_CONFIG_PATH})
endif()

find_package(Kodi REQUIRED)
find_package(p8-platform REQUIRED)
find_package(ProjectM REQUIRED)

add_definitions(-DHAS_SDL_OPENGL)

if(APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM)
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

include_directories(${includes}
                    ${p8-platform_INCLUDE_DIRS}
                    ${PROJECTM_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways

if(CORE_SYSTEM_NAME STREQUAL "linux")
  list(APPEND DEPLIBS -Wl,--whole-archive ${PROJECTM_LIBS} -Wl,--no-whole-archive
                      ${extra_libs})
else()
  list(APPEND DEPLIBS ${PROJECTM_LIBS}
                      ${extra_libs})
endif()

set(PM_SOURCES src/Main.cpp)

include_directories(${INCLUDES})

set(PM_CUSTOM_DATA ${PROJECTM_DATA})
build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
