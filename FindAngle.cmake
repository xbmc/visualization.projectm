#.rst:
# FindAngle
# ------------
# Finds the Angle library
#
# As Angle brings a GLES support are the given values in GLES style.
#
# This will define the following variables:
#
# OPENGLES_FOUND - system has OpenGLES
# OPENGLES_INCLUDE_DIRS - the OpenGLES include directory
# OPENGLES_LIBRARIES - the OpenGLES libraries
# OPENGLES_DEFINITIONS - the OpenGLES definitions
#
# Note:
# On Windows with angle the *_INCLUDE_DIRS and
# *_DEFINITIONS are undefined, but are set
# global by the kodi-angle package.

find_package(kodi-angle REQUIRED)
set(OPENGLES_LIBRARIES kodi::angle::libGLESv2 kodi::angle::libEGL)
set(OPENGLES_FOUND ${kodi-angle_FOUND})
set(OPENGLES_DEFINITIONS -DHAS_GLES=3)
