project(ftgl)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

include(ExternalProject)

externalproject_add(ftgl
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND cd unix && autoreconf -vif 
                    CONFIGURE_COMMAND cd unix && CXXFLAGS=-fpermissive ./configure --prefix=${OUTPUT_DIR} --with-pic
                    BUILD_COMMAND cd unix && make
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unix)")
