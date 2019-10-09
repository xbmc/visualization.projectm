# - Try to find ProjectM
# Once done this will define
#
# PROJECTM_FOUND - system has libprojectM
# PROJECTM_INCLUDE_DIRS - the libprojectM include directory
# PROJECTM_PKGDATADIR - the libprojectM directory with required data, e.g. presets
# PROJECTM_LIBRARIES - The libprojectM libraries

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_PROJECTM libprojectM QUIET)

  if(EXISTS ${PC_PROJECTM_PREFIX}/share/projectM)
    set(PC_PROJECTM_PKGDATADIR ${PC_PROJECTM_PREFIX}/share/projectM)
  else()
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=pkgdatadir libprojectM --prefix-variable=${PC_PROJECTM_PREFIX}
                    OUTPUT_VARIABLE PC_PROJECTM_PKGDATADIR
                    RESULT_VARIABLE _pkgconfig_failed)
    if (_pkgconfig_failed)
      message(FATAL_ERROR "Missing libprojectM pkgdatadir")
    endif()

    string(REGEX REPLACE "[\r\n]" "" PC_PROJECTM_PKGDATADIR "${PC_PROJECTM_PKGDATADIR}")
  endif()
endif()

find_path(PROJECTM_INCLUDE_DIRS libprojectM/projectM.hpp
                                PATHS ${PC_PROJECTM_INCLUDEDIR})
find_path(PROJECTM_PKGDATADIR NAMES config.inp presets/presets_projectM
                              PATH_SUFFIXES projectM
                              PATHS ${PC_PROJECTM_PKGDATADIR})
find_library(PROJECTM_LIBRARIES NAMES projectM
                                PATHS ${PC_PROJECTM_LIBDIR})

if(APPLE)
  set(PROJECTM_LIBRARIES "-framework CoreFoundation" ${PROJECTM_LIBRARIES})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(projectM REQUIRED_VARS PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES PROJECTM_PKGDATADIR)

mark_as_advanced(PROJECTM_INCLUDE_DIRS PROJECTM_LIBRARIES PROJECTM_PKGDATADIR)
