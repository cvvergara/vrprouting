cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove CMakeCache.txt.")
endif()

if(POLICY CMP0144)
  cmake_policy(SET CMP0144 OLD)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()

#---------------------------------------------
# Setting kind of build
#---------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


#---------------------------------------------
#---------------------------------------------
#---------------------------------------------

project(VRPROUTING VERSION 0.0.0
)
set(PROJECT_VERSION_DEV "")
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

set(MINORS 0.0)
set(OLD_SIGNATURES
)


include(pgr/BuildType)
include(pgr/GitInfo)
include(pgr/Version)

option(PROJECT_DEBUG "Set ON|OFF to get more output" OFF)

add_definitions(-DPROJECT_VERSION="v${PROJECT_VERSION}${PROJECT_VERSION_DEV}")
set(PROJECT_LIB_NAME "${PROJECT_NAME_LOWER}-${PROJECT_LIB_VERSION}")

#=============================================
# Set the working directories
#=============================================
include(pgr/Configure)


#---------------------------------------------
# minimum versions
#---------------------------------------------
set(SPHINX_MINIMUM_VERSION "4.0.0")
set(POSTGRESQL_MINIMUM_VERSION "12.0")
set(ORTOOLS_VERSION "9.10.4067")
string(TIMESTAMP YEAR "%Y")

message(STATUS "ORTOOLS_VERSION=${ORTOOLS_VERSION}")
message(STATUS "DOXYGEN_MINIMUM_VERSION=${DOXYGEN_MINIMUM_VERSION}")
message(STATUS "SPHINX_MINIMUM_VERSION=${SPHINX_MINIMUM_VERSION}")
message(STATUS "POSTGRESQL_MINIMUM_VERSION=${POSTGRESQL_MINIMUM_VERSION}")

#-----------------------------------------------------------------------------
# Finding prerequisites
#-----------------------------------------------------------------------------
find_package(Perl REQUIRED)

#---------------------------------------------
# PostgreSQL
#---------------------------------------------
set(PostgreSQL_ADDITIONAL_VERSIONS "18" "17" "16")
find_package(PostgreSQL ${POSTGRESQL_MINIMUM_VERSION} REQUIRED)

if(POSTGRESQL_BIN)
  # Checking POSTGRESQL_PG_CONFIG
  find_program(POSTGRESQL_PG_CONFIG NAMES pg_config
    PATHS
    ${POSTGRESQL_BIN}
    NO_DEFAULT_PATH
  )
else()
  find_program(POSTGRESQL_PG_CONFIG NAMES pg_config)
endif()

#---------------------------------------------
# extension directory
#---------------------------------------------

execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --sharedir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE EXTENSION_DIR)

#-------
if(EXTENSION_DIR)
  set(EXTENSION_DIR "${EXTENSION_DIR}/extension")
else(EXTENSION_DIR)
  message(FATAL_ERROR "pg_config --sharedir failed to return a value.
  Please check your PostgreSQL installation!")
endif(EXTENSION_DIR)

#---------------------------------------------

message(STATUS "EXTENSION_DIR=${EXTENSION_DIR}")

#-----------------------------------------------------------------------------
# Build and install the extension
#-----------------------------------------------------------------------------
add_subdirectory(sql)

install(FILES
  ${PROJECT_FILES_TO_INSTALL}
  DESTINATION "${EXTENSION_DIR}"
)

#-----------------------------------------------------------------------------
# DOCUMENTATION
# Not installing
#-----------------------------------------------------------------------------
add_subdirectory(doc)
