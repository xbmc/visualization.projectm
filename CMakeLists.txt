project(visualization.projectm)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
if(PACKAGE_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH} ${PACKAGE_CONFIG_PATH})
endif()

find_package(Kodi REQUIRED)
find_package(p8-platform REQUIRED)

find_package(ProjectM REQUIRED)

add_definitions(-DHAS_SDL_OPENGL)

set(INCLUDES ${KODI_INCLUDE_DIR}
             ${p8-platform_INCLUDE_DIRS}
             ${PROJECTM_INCLUDE_DIRS})

if(CORE_SYSTEM_NAME STREQUAL "linux")
  set(DEPLIBS -Wl,--whole-archive ${PROJECTM_LIBS} -Wl,--no-whole-archive
              ${extra_libs})
else()
  set(DEPLIBS ${PROJECTM_LIBS}
              ${extra_libs})
endif()

if(APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM)
  find_package(OpenGL REQUIRED)
  set(DEPLIBS ${DEPLIBS} ${OPENGL_LIBRARIES})
  set(APP_RENDER_SYSTEM gl)
elseif(APP_RENDER_SYSTEM STREQUAL "gles")
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${DEPLIBS} ${OPENGLES_LIBRARIES})
  set(APP_RENDER_SYSTEM gles)
else()
  message(FATAL_ERROR "APP_RENDER_SYSTEM ${APP_RENDER_SYSTEM} isn't supported.")
endif()

set(PM_SOURCES src/Main.cpp)

include_directories(${INCLUDES})

set(PM_CUSTOM_DATA ${PROJECTM_DATA})
build_addon(visualization.projectm PM DEPLIBS)

include(CPack)
