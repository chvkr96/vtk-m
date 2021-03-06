##============================================================================
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##  Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
##  Copyright 2017 UT-Battelle, LLC.
##  Copyright 2017 Los Alamos National Security.
##
##  Under the terms of Contract DE-NA0003525 with NTESS,
##  the U.S. Government retains certain rights in this software.
##
##  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
##  Laboratory (LANL), the U.S. Government retains certain rights in
##  this software.
##============================================================================

# Used to determine the version for VTK-m source using "git describe", if git
# is found. On success sets the following variables in caller's scope:
#   ${var_prefix}_VERSION
#   ${var_prefix}_VERSION_MAJOR
#   ${var_prefix}_VERSION_MINOR
#   ${var_prefix}_VERSION_PATCH
#   ${var_prefix}_VERSION_PATCH_EXTRA
#   ${var_prefix}_VERSION_FULL
#   ${var_prefix}_VERSION_IS_RELEASE is true, if patch-extra is empty.
#
# If git is not found, or git describe cannot be run successfully, then these
# variables are left unchanged and a status message is printed.
#
# Arguments are:
#   source_dir : Source directory
#   git_command : git executable
#   var_prefix : prefix for variables e.g. "VTKm".
function(determine_version source_dir git_command var_prefix)
  if ("$Format:$" STREQUAL "")
    # We are in an exported tarball and should use the shipped version
    # information. Just return here to avoid the warning message at the end of
    # this function.
    return ()
  elseif (NOT VTKm_GIT_DESCRIBE)
    if(EXISTS ${git_command} AND EXISTS ${source_dir}/.git)
      execute_process(
        COMMAND ${git_command} describe
        WORKING_DIRECTORY ${source_dir}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)
    endif()
  else()
    set(result 0)
    set(output ${VTKm_GIT_DESCRIBE})
  endif()
  extract_version_components("${output}" tmp)
  if(DEFINED tmp_VERSION)
    message(STATUS "Determined ${PROJECT_NAME} Source Version: ${tmp_VERSION_FULL}")
    if (NOT "${tmp_VERSION}" STREQUAL "${${var_prefix}_VERSION}")
      message(WARNING
        "Version from git (${tmp_VERSION}) disagrees with hard coded version (${${var_prefix}_VERSION}). Either update the git tags or version.txt.")
    endif()
    foreach(suffix VERSION VERSION_MAJOR VERSION_MINOR VERSION_PATCH
                   VERSION_PATCH_EXTRA VERSION_FULL VERSION_IS_RELEASE)
      set(${var_prefix}_${suffix} ${tmp_${suffix}} PARENT_SCOPE)
    endforeach()
  else()
    message(STATUS
      "Could not use git to determine source version, using version ${${var_prefix}_VERSION_FULL}")
  endif()
endfunction()

# Extracts components from a version string. See determine_version() for usage.
function(extract_version_components version_string var_prefix)
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)[-]*(.*)"
    version_matches "${version_string}")
  if(CMAKE_MATCH_0)
    set(full ${CMAKE_MATCH_0})
    set(major ${CMAKE_MATCH_1})
    set(minor ${CMAKE_MATCH_2})
    set(patch ${CMAKE_MATCH_3})
    set(patch_extra ${CMAKE_MATCH_4})

    set(${var_prefix}_VERSION "${major}.${minor}" PARENT_SCOPE)
    set(${var_prefix}_VERSION_MAJOR ${major} PARENT_SCOPE)
    set(${var_prefix}_VERSION_MINOR ${minor} PARENT_SCOPE)
    set(${var_prefix}_VERSION_PATCH ${patch} PARENT_SCOPE)
    set(${var_prefix}_VERSION_PATCH_EXTRA ${patch_extra} PARENT_SCOPE)
    set(${var_prefix}_VERSION_FULL ${full} PARENT_SCOPE)
    if("${major}.${minor}.${patch}" VERSION_EQUAL "${full}")
      set(${var_prefix}_VERSION_IS_RELEASE TRUE PARENT_SCOPE)
    else()
      set(${var_prefix}_VERSION_IS_RELEASE FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()
