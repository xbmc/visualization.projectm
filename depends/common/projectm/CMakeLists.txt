cmake_minimum_required(VERSION 3.5)
project(projectm)

if (APP_RENDER_SYSTEM STREQUAL "gles")
  set(PROJECTM_CONFIG "--enable-gles")
endif()

include(ExternalProject)
externalproject_add(projectm
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND autoreconf -vif
                    CONFIGURE_COMMAND CXXFLAGS=-I${OUTPUT_DIR}/include CFLAGS=-I${OUTPUT_DIR}/include LDFLAGS=-L${OUTPUT_DIR}/lib ./configure
                      --disable-shared
                      --disable-qt
                      --disable-threading
                      --enable-preset-subdirs
                      --prefix=${OUTPUT_DIR}
                      --with-pic ${PROJECTM_CONFIG}
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
