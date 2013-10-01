PROJECT(visualization.projectm)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

FIND_PACKAGE(xbmc REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)

ADD_DEFINITIONS(-DHAS_SDL_OPENGL)
SET(DATA_PATH "/usr/share/xbmc/addons/visualization.projectm/resources")
ADD_DEFINITIONS("-DDATAPATH=\"${DATAPATH}\"")

SET(INCLUDES ${OpenGL_INCLUDE_DIR}
             ${GLEW_INCLUDE_DIR}
             ${XBMC_INCLUDE_DIR})

SET(DEPLIBS ${OPENGL_LIBRARIES}
            ${GLEW_LIBRARIES})

SET(PM_SOURCES src/Main.cpp)

FIND_PACKAGE(ProjectM)
IF (PROJECTM_FOUND)
  SET(DEPLIBS ${DEPLIBS} ${PROJECTM_LIBRARY})
  SET(INCLUDES ${INCLUDES} ${PROJECTM_INCLUDE_DIR})
ELSE(PROJECTM_FOUND)
  MESSAGE(STATUS "Building libprojectM")
  INCLUDE(ExternalProject)
  ExternalProject_ADD(ftgl
                      URL http://heanet.dl.sourceforge.net/project/ftgl/FTGL%20Source/2.1.2/ftgl-2.1.2.tar.gz
                      PREFIX ftgl
                      UPDATE_COMMAND cd unix && autoreconf -vif 
                      CONFIGURE_COMMAND cd unix && CXXFLAGS=-fpermissive ./configure --prefix=${CMAKE_BINARY_DIR}/installed --with-pic
                      BUILD_COMMAND cd unix && make
                      INSTALL_COMMAND cd unix && make install
                      BUILD_IN_SOURCE 1)
  ExternalProject_ADD(projectM
                      URL http://heanet.dl.sourceforge.net/project/projectm/libprojectM/libprojectM-2.0.0/libprojectM-2.0.0-Source.tar.gz
                      PATCH_COMMAND rm CMakeFiles CMakeCache.txt -r &&
                                    patch -p0 < ${PROJECT_SOURCE_DIR}/01-pm-fix-freetype.patch &&
                                    patch -p0 < ${PROJECT_SOURCE_DIR}/02-pm-fix-order.patch
                                    CONFIGURE_COMMAND CXXFLAGS=-fPIC PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/installed/lib/pkgconfig cmake ${CMAKE_BINARY_DIR}/projectM-prefix/src/projectM -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_PROJECTM_STATIC=1 -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/installed)

  ADD_DEPENDENCIES(projectM ftgl)
  SET(DEPLIBS ${DEPLIBS} ${CMAKE_BINARY_DIR}/installed/lib/libprojectM.a)
  SET(INCLUDES ${INCLUDES} ${CMAKE_BINARY_DIR}/installed/include/libprojectM)
ENDIF(PROJECTM_FOUND)

INCLUDE_DIRECTORIES(${INCLUDES})

build_addon(visualization.projectm PM DEPLIBS)
add_dependencies(visualization.projectm projectM)

include(CPack)
