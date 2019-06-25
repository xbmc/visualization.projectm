project(ftgl)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

include(ExternalProject)

externalproject_add(ftgl
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND <SOURCE_DIR>/autogen.sh
                    CONFIGURE_COMMAND ./configure --prefix=${OUTPUT_DIR} --with-pic --disable-shared --enable-static --with-ft-prefix=${OUTPUT_DIR}
                    BUILD_COMMAND make -C src
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make -C src install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
