cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

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

project(PGORPY VERSION 0.0.0
)
set(PROJECT_VERSION_DEV "")
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

set(MINORS 0.0)
set(OLD_SIGNATURES
)


include(pgr/BuildType)
include(pgr/GitInfo)
include(pgr/Version)

add_definitions(-DPROJECT_VERSION="v${PROJECT_VERSION}${PROJECT_VERSION_DEV}")
set(PROJECT_LIB_NAME "${PROJECT_NAME_LOWER}-${PROJECT_LIB_VERSION}")

option(PROJECT_DEBUG "Set ON|OFF to get more output" OFF)

#=============================================
# Set the working directories
#=============================================
include(pgr/Configure)


#---------------------------------------------
# minimum versions
#---------------------------------------------
set(SPHINX_MINIMUM_VERSION "1.8")
set(POSTGRESQL_MINIMUM_VERSION "12.0")
set(ORTOOLS_VERSION "9.10.4067")
string(TIMESTAMP YEAR "%Y")

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
set(PostgreSQL_ADDITIONAL_VERSIONS "12" "13" "14")
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

if(POSTGRESQL_PG_CONFIG)
  execute_process(
    COMMAND ${POSTGRESQL_PG_CONFIG} --bindir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE T_POSTGRESQL_BIN)
endif(POSTGRESQL_PG_CONFIG)

# search for POSTGRESQL_EXECUTABLE _only_ in the dir specified by pg_config
find_program(POSTGRESQL_EXECUTABLE NAMES postgres
  PATHS
  ${T_POSTGRESQL_BIN}
  NO_DEFAULT_PATH
)

# for XbetaY XalphaY XrcY -> X.Y
string(REGEX REPLACE "^([0-9]+)[beta|alpha|rc|devel].*" "\\1.0" PostgreSQL_VERSION_STRING ${PostgreSQL_VERSION_STRING})

#for X.Y.Z -> XY  Y<10
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1" PostgreSQL_VERSION_MAYOR ${PostgreSQL_VERSION_STRING})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" PostgreSQL_VERSION_MINOR ${PostgreSQL_VERSION_STRING})
math(EXPR PostgreSQL_VERSION_NUM "${PostgreSQL_VERSION_MAYOR} * 10000 + ${PostgreSQL_VERSION_MINOR}")


#-------
include_directories(SYSTEM ${PostgreSQL_INCLUDE_DIRS})
if(WIN32)
  include_directories(SYSTEM ${PostgreSQL_INCLUDE_DIRS}/port/win32)
  if(MSVC)
    include_directories(SYSTEM ${PostgreSQL_INCLUDE_DIRS}/port/win32_msvc/)
  endif()
endif()

#[[
#---------------------------------------------
# Definitions for full_version
#---------------------------------------------
add_definitions(-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")
add_definitions(-DPROJECT_LIB_NAME="${PROJECT_NAME_LOWER}-${PROJECT_VERSION}")
add_definitions(-DPROJECT_GIT_HASH="${PROJECT_GIT_HASH}")
string(TIMESTAMP COMPILATION_DATE "\"%Y/%m/%d\"")
add_definitions(-DCOMPILATION_DATE=${COMPILATION_DATE})
add_definitions(-DCOMPILER_VERSION="${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
add_definitions(-DPostgreSQL_VERSION_STRING="${PostgreSQL_VERSION_STRING}")
add_definitions(-DSYSTEM_NAME="${CMAKE_SYSTEM}")
# Definitions
add_definitions(-DPostgreSQL_VERSION_NUM=${PostgreSQL_VERSION_NUM})

if (PROJECT_DEBUG)
  message(STATUS "PostgreSQL_FOUND ${PostgreSQL_FOUND}")
  message(STATUS "PostgreSQL_INCLUDE_DIRS ${PostgreSQL_INCLUDE_DIRS}")
  message(STATUS "PostgreSQL_LIBRARIES ${PostgreSQL_LIBRARIES}")
  message(STATUS "PostgreSQL_LIBRARY_DIRS ${PostgreSQL_LIBRARY_DIRS}")
  message(STATUS "PostgreSQL_VERSION_STRING ${PostgreSQL_VERSION_STRING}")
  message(STATUS "PostgreSQL_VERSION_NUM ${PostgreSQL_VERSION_NUM}")
  message(STATUS "POSTGRESQL_PG_CONFIG ${POSTGRESQL_PG_CONFIG}")
endif()
#]]


#---------------------------------------------
# library directory
#---------------------------------------------
execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --pkglibdir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE LIBRARY_INSTALL_PATH)

if(NOT LIBRARY_INSTALL_PATH)
  message(FATAL_ERROR "pg_config --pkglibdir failed to return a value. Please check your PostgreSQL installation!")
endif()

message(STATUS "LIBRARY_INSTALL_PATH ${LIBRARY_INSTALL_PATH}")

#---------------------------------------------
# extension directory
#---------------------------------------------

execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --sharedir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE SHARE_DIR)

#-------
if(SHARE_DIR)
  set(SHARE_DIR "${SHARE_DIR}/extension")
else(SHARE_DIR)
  message(FATAL_ERROR "pg_config --sharedir failed to return a value. Please check your PostgreSQL installation!")
endif(SHARE_DIR)

#---------------------------------------------

message(STATUS "PostgreSQL_LIBRARIES=${PostgreSQL_LIBRARIES}")
message(STATUS "LIBRARY_INSTALL_PATH=${LIBRARY_INSTALL_PATH}")
message(STATUS "SHARE_DIR=${SHARE_DIR}")


#---------------------------------------------
# Special cases for windows
#---------------------------------------------

if(WIN32)
  link_directories(${PostgreSQL_LIBRARIES})
  link_libraries(${POSTGRES_EXECUTABLE})
endif()

#-----------------------------------------------------------------------------

if (PROJECT_DEBUG)
  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  message(STATUS "INCLUDE_DIRECTORIES='${dirs}'")
endif()


#-----------------------------------------------------------------------------
# sql subdirectory creates the files:
#  ${PROJECT_NAME_LOWER}--(version).sql
#  ${PROJECT_NAME_LOWER}--(oldVersion)--(version).sql
#-----------------------------------------------------------------------------

add_subdirectory(sql)

#-----------------------------------------------------------------------------
# INSTALLATION
#-----------------------------------------------------------------------------

install(FILES
  ${PROJECT_FILES_TO_INSTALL}
  DESTINATION "${SHARE_DIR}"
)

#-----------------------------------------------------------------------------
# DOCUMENTATION
# We are not installing documentation
#-----------------------------------------------------------------------------
add_subdirectory(doc)
