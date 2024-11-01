set(MOSEK_VERSION "unknown")
set(MOSEK_HOME
    $ENV{MOSEK_HOME}
    CACHE PATH "Path to MOSEK")
set(MOSEK_POSSIBLE_ROOTS ${MOSEK_HOME} /opt/mosek /usr/local/mosek
                         "C:/Program Files/Mosek")

foreach(ROOT_DIR IN LISTS MOSEK_POSSIBLE_ROOTS)
  if(IS_DIRECTORY "${ROOT_DIR}")
    file(GLOB MOSEK_VERSION_DIRS "${ROOT_DIR}/[0-9.]*")
    foreach(DIR IN LISTS MOSEK_VERSION_DIRS)
      if(IS_DIRECTORY "${DIR}/tools")
        get_filename_component(MOSEK_VERSION ${DIR} NAME)
        set(MOSEK_ROOT_DIR "${DIR}")
        break()
      endif()
    endforeach()

    if(NOT MOSEK_VERSION STREQUAL "unknown")
      break()
    endif()
  endif()
endforeach()

if(MOSEK_VERSION STREQUAL "unknown")
  message(FATAL_ERROR "Could NOT find MOSEK version directory")
endif()

string(REPLACE "." ";" MOSEK_VERSION_PARTS ${MOSEK_VERSION})
list(GET MOSEK_VERSION_PARTS 0 MOSEK_VERSION_MAJOR)
list(GET MOSEK_VERSION_PARTS 1 MOSEK_VERSION_MINOR)

if(WIN32)
  set(MOSEK_PLATFORM "win64x86")
  set(MOSEK_LIBRARY_NAME
      "mosek64_${MOSEK_VERSION_MAJOR}_${MOSEK_VERSION_MINOR}")
  set(MOSEK_DEBUG_LIBRARY_NAME
      "mosek64_${MOSEK_VERSION_MAJOR}_${MOSEK_VERSION_MINOR}_debug")
else()
  set(MOSEK_PLATFORM "linux64x86")
  set(MOSEK_LIBRARY_NAME "mosek64")
endif()

find_path(MOSEK_INCLUDE_DIR mosek.h
          PATHS ${MOSEK_ROOT_DIR}/tools/platform/${MOSEK_PLATFORM}/h)
message(STATUS "MOSEK_INCLUDE_DIR: ${MOSEK_INCLUDE_DIR}")
if(WIN32)
  find_library(
    MOSEK_LIBRARY
    NAMES ${MOSEK_LIBRARY_NAME}
    PATHS ${MOSEK_ROOT_DIR}/tools/platform/${MOSEK_PLATFORM}/bin)

  find_library(
    MOSEK_IMPLIB_RELEASE
    NAMES ${MOSEK_LIBRARY_NAME}
    PATHS ${MOSEK_ROOT_DIR}/tools/platform/${MOSEK_PLATFORM}/lib)

  find_library(
    MOSEK_IMPLIB_DEBUG
    NAMES ${MOSEK_DEBUG_LIBRARY_NAME}
    PATHS ${MOSEK_ROOT_DIR}/tools/platform/${MOSEK_PLATFORM}/lib)
else()
  find_library(
    MOSEK_LIBRARY
    NAMES ${MOSEK_LIBRARY_NAME}
    PATHS ${MOSEK_ROOT_DIR}/tools/platform/${MOSEK_PLATFORM}/bin)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MOSEK
  REQUIRED_VARS MOSEK_LIBRARY MOSEK_INCLUDE_DIR
  FAIL_MESSAGE "Could not find MOSEK library and/or headers")

if(MOSEK_FOUND)
  if(NOT TARGET MOSEK::MOSEK)
    add_library(MOSEK::MOSEK SHARED IMPORTED)

    if(WIN32)
      set_target_properties(
        MOSEK::MOSEK
        PROPERTIES IMPORTED_LOCATION_RELEASE "${MOSEK_LIBRARY_RELEASE}"
                   IMPORTED_IMPLIB_RELEASE "${MOSEK_IMPLIB_RELEASE}"
                   INTERFACE_INCLUDE_DIRECTORIES "${MOSEK_INCLUDE_DIR}"
                   INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${MOSEK_INCLUDE_DIR}"
                   IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    else()
      set_target_properties(
        MOSEK::MOSEK
        PROPERTIES IMPORTED_LOCATION "${MOSEK_LIBRARY}"
                   INTERFACE_INCLUDE_DIRECTORIES "${MOSEK_INCLUDE_DIR}"
                   INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${MOSEK_INCLUDE_DIR}")
    endif()

    message(
      STATUS
        "Found MOSEK: ${MOSEK_LIBRARY} (found version \"${MOSEK_VERSION}\")")
  endif()
endif()

mark_as_advanced(MOSEK_INCLUDE_DIR MOSEK_LIBRARY MOSEK_VERSION)
