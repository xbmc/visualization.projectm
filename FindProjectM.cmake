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

if(NOT PROJECTM_FOUND)
  find_path(PROJECTM_INCLUDE_DIRS libprojectM/projectM.hpp)
  find_library(PROJECTM_LIBRARIES projectM)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ProjectM DEFAULT_MSG PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES)

mark_as_advanced(PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES)
