# - Try to find ASS
# Once done this will define
#
# ASS_FOUND - system has libass
# ASS_INCLUDE_DIRS - the libass include directory
# ASS_LIBRARIES - The libass libraries

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules (PROJECTM libprojectM)
  list(APPEND PROJECTM_INCLUDE_DIRS ${PROJECTM_INCLUDEDIR})
endif()

#if(NOT PROJECTM_FOUND)
#  find_path(PROJECTM_INCLUDE_DIRS libprojectM/projectM.hpp)
#  find_library(PROJECTM_LIBRARIES projectM)
#endif()

include(FindPackageHandleStandardArgs)

  # this var is not set when using system libs on linux
if(PROJECTM_LIBRARY_DIRS)
  find_package_handle_standard_args(ProjectM DEFAULT_MSG PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES PROJECTM_LIBRARY_DIRS)

  if(APPLE)
    set(EXTRA_LDFLAGS "-framework CoreFoundation")
  else()
    set(EXTRA_LDFLAGS -Wl,-rpath='$ORIGIN')
  endif()

  set(PROJECTM_LIBS ${EXTRA_LDFLAGS} -L${PROJECTM_LIBRARY_DIRS} ${PROJECTM_LIBRARIES})

  file(GLOB PROJECTM_SOLIB  ${PROJECTM_LIBRARY_DIRS}/lib${PROJECTM_LIBRARIES}.so*)
  set(COPY_SOLIB true)
else()
  find_package_handle_standard_args(ProjectM DEFAULT_MSG PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES)
  set(PROJECTM_LIBS ${PROJECTM_LIBRARIES})
endif()
set(PROJECTM_DATA ${PROJECTM_PREFIX}/share/projectM)

mark_as_advanced(PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES PROJECTM_DATA)
