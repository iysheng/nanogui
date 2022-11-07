# Locate ROCKCHIPmpp library
# This module defines
# ROCKCHIPmpp_LIBRARY, the name of the library to link against
# ROCKCHIPmpp_FOUND, if false, do not try to link to ROCKCHIPmpp
# ROCKCHIPmpp_INCLUDE_DIR, where to find SDL.h
#
# This module responds to the the flag:
# ROCKCHIPmpp_BUILDING_LIBRARY
# If this is defined, then no ROCKCHIPmppmain will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned ROCKCHIPmpp_LIBRARY variable.
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the
# OS X framework based version. (Other versions link to -lROCKCHIPmppmain which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
#
# Additional Note: If you see an empty ROCKCHIPmpp_LIBRARY_TEMP in your configuration
# and no ROCKCHIPmpp_LIBRARY, it means CMake did not find your ROCKCHIPmpp library
# (ROCKCHIPmpp.dll, libsdl2.so, ROCKCHIPmpp.framework, etc).
# Set ROCKCHIPmpp_LIBRARY_TEMP to point to your ROCKCHIPmpp library, and configure again.
# Similarly, if you see an empty ROCKCHIPmppMAIN_LIBRARY, you should set this value
# as appropriate. These values are used to generate the final ROCKCHIPmpp_LIBRARY
# variable, but when these values are unset, ROCKCHIPmpp_LIBRARY does not get created.
#
#
# $ROCKCHIPmppDIR is an environment variable that would
# correspond to the ./configure --prefix=$ROCKCHIPmppDIR
# used in building ROCKCHIPmpp.
# l.e.galup  9-20-02
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for ROCKCHIPmppmain which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# ROCKCHIPmpp_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from ROCKCHIPmpp/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <ROCKCHIPmpp/SDL.h>. This is done for portability
# reasons because not all systems place things in ROCKCHIPmpp/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

SET(ROCKCHIPmpp_SEARCH_PATHS
	/opt/red_aarch64/mpp
)

FIND_PATH(ROCKCHIPmpp_INCLUDE_DIR mpp_buffer.h
	HINTS
	$ENV{ROCKCHIPmppDIR}
	PATH_SUFFIXES include/rockchip
	PATHS ${ROCKCHIPmpp_SEARCH_PATHS}
)

FIND_LIBRARY(ROCKCHIPmpp_LIBRARY_TEMP
	NAMES rockchip_mpp
	HINTS
	$ENV{ROCKCHIPmppDIR}
	PATH_SUFFIXES lib/ 
	PATHS ${ROCKCHIPmpp_SEARCH_PATHS}
	NO_CACHE
	NO_DEFAULT_PATH
)

# ROCKCHIPmpp may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
	FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

IF(ROCKCHIPmpp_LIBRARY_TEMP)
	# For threads, as mentioned Apple doesn't need this.
	# In fact, there seems to be a problem if I used the Threads package
	# and try using this line, so I'm just skipping it entirely for OS X.
	IF(NOT APPLE)
		SET(ROCKCHIPmpp_LIBRARY_TEMP ${ROCKCHIPmpp_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
	ENDIF(NOT APPLE)

	# Set the final string here so the GUI reflects the final state.
	#SET(ROCKCHIPmpp_LIBRARY ${ROCKCHIPmpp_LIBRARY_TEMP} CACHE STRING "Where the ROCKCHIPmpp Library can be found")
	# Set the temp variable to INTERNAL so it is not seen in the CMake GUI
    message("red ROCKCHIPmpp_LIBRARY ${ROCKCHIPmpp_LIBRARY_TEMP}")
	SET(ROCKCHIPmpp_LIBRARY "${ROCKCHIPmpp_LIBRARY_TEMP}")
ENDIF(ROCKCHIPmpp_LIBRARY_TEMP)

INCLUDE(FindPackageHandleStandardArgs)
message("ROCKCHIPmpp_LIBRARY ${ROCKCHIPmpp_LIBRARY}")
message("ROCKCHIPmpp_INCLUDE_DIR ${ROCKCHIPmpp_INCLUDE_DIR}")

# 
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ROCKCHIPmpp REQUIRED_VARS ROCKCHIPmpp_LIBRARY ROCKCHIPmpp_INCLUDE_DIR)
