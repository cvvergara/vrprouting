#---------------------------------------------
# Checking for prerequisites
#---------------------------------------------
find_package(Sphinx ${SPHINX_MINIMUM_VERSION})

if (NOT SPHINX_FOUND)
  message(WARNING "Sphinx not found.
  building without documentation")
  return()
endif()


option(BUILD_HTML "Set ON|OFF (default=OFF) to build Documentation library tree as HTML" ON)
option(BUILD_LOCALE "Set ON|OFF (default=OFF) to build Documentation library tree as EPUB page" OFF)
option(BUILD_PDF "Set ON|OFF (default=OFF) to build Documentation library tree as PDF" OFF)
option(LINKCHECK "Set ON|OFF (default=OFF) to build Documentation library tree as LINKCHECK page" OFF)

#[[ following options have not been tested
option(BUILD_LATEX "Set ON|OFF (default=OFF) to build Documentation library tree as PDF" OFF)
option(SINGLEHTML "Set ON|OFF (default=OFF) to build Documentation library tree as SINGLE HTML" OFF)
option(BUILD_MAN "Set ON|OFF (default=OFF) to build Documentation library tree as MAN page" OFF)
option(EPUB "Set ON|OFF (default=OFF) to build Documentation library tree as EPUB page" OFF)
option(DUMMY "Set ON|OFF (default=OFF) for debugging purposes" OFF)
#]]

foreach(opt BUILD_HTML BUILD_LOCALE LINKCHECK)
  if (${${opt}})
    string(TOLOWER ${opt} val)
    string(REPLACE "build_" "" val ${val})
    list(APPEND PROJECT_DOC_TARGETS ${val})
  endif()
endforeach()


message(STATUS "PROJECT_DOC_TARGETS = ${PROJECT_DOC_TARGETS}")
if("${PROJECT_DOC_TARGETS}" STREQUAL "")
  message(WARNING "No documentation targets found
  Building without documentation")
  return()
endif()



set(PROJECT_LANGUAGES "en")

foreach(lang ${PROJECT_LANGUAGES})
  string(TOUPPER ${lang} val)
  if (lang STREQUAL "en")
    option(${val} "Set ON|OFF (default=ON) build ${lang} Documentation" ON)
  else()
    option(${val} "Set ON|OFF (default=OFF) build ${lang} Documentation" OFF)
  endif()
endforeach()

#---------------------------------------------
# Catching the language options to be build
#---------------------------------------------
foreach(lang ${PROJECT_LANGUAGES})
  string(TOUPPER ${lang} opt)
  if (${opt})
    list(APPEND PROJECT_BUILD_LANGUAGES ${lang})
  endif()
endforeach()

message(STATUS "PROJECT_BUILD_LANGUAGES = ${PROJECT_BUILD_LANGUAGES}")
if("${PROJECT_BUILD_LANGUAGES}" STREQUAL "")
  message(WARNING "No documentation languages found
  Building without documentation")
  return()
endif()

message(STATUS "PROJECT_BUILD_LANGUAGES = ${PROJECT_BUILD_LANGUAGES}")


# configured documentation tools and intermediate build results
# TODO for 3.0 Use _build
# set(PGR_DOCUMENTATION_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
set(PGR_DOCUMENTATION_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(PGR_DOCUMENTATION_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

message(STATUS "PGR_DOCUMENTATION_BUILD_DIR = ${PGR_DOCUMENTATION_BUILD_DIR}")
message(STATUS "PGR_DOCUMENTATION_SOURCE_DIR = ${PGR_DOCUMENTATION_SOURCE_DIR}")


#---------------------------------------------
# Adding documentation directories
#---------------------------------------------
foreach (subdir ${PROJECT_DOC_DIRECTORIES} "general")
  add_subdirectory("${subdir}")
  if (EXISTS "${CMAKE_SOURCE_DIR}/docqueries/${subdir}")
    add_subdirectory("${CMAKE_SOURCE_DIR}/docqueries/${subdir}" "build/doc/${subdir}")
  endif()
endforeach()


configure_file("../tools/testers/sampledata.sql" "${PGR_DOCUMENTATION_SOURCE_DIR}/sampledata/sampledata.sql")


#TODO ???
#add_subdirectory("static")
#add_subdirectory("themes")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/_static" DESTINATION "${PGR_DOCUMENTATION_SOURCE_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/_themes" DESTINATION "${PGR_DOCUMENTATION_SOURCE_DIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_templates/layout.html" "${PGR_DOCUMENTATION_SOURCE_DIR}/_templates/layout.html" @ONLY)


configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${PGR_DOCUMENTATION_SOURCE_DIR}/conf.py"
    @ONLY)

# Add documentation to targets


set(ENV{SPHINXINTL_LANGUAGE} ${SPHINXINTL_LANGUAGE})
message(STATUS "SPHINXINTL_LANGUAGE=${SPHINXINTL_LANGUAGE}")

if (BUILD_LOCALE)
    add_custom_target(locale
        COMMAND ${SPHINX_EXECUTABLE}
        -b gettext
        -c "${PGR_DOCUMENTATION_SOURCE_DIR}"
        "${PGR_DOCUMENTATION_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/locale/pot"

        COMMAND sphinx-intl update -d ${CMAKE_SOURCE_DIR}/locale -l en > locale_changes.txt
    #COMMAND sphinx-intl update -p ${CMAKE_SOURCE_DIR}/locale/pot -d ${CMAKE_SOURCE_DIR}/locale --language=${SPHINXINTL_LANGUAGE}
    #COMMAND sphinx-intl update-txconfig-resources --locale-dir ${CMAKE_SOURCE_DIR}/locale --pot-dir ${CMAKE_SOURCE_DIR}/locale/pot --transifex-project-name pgrouting


    COMMENT "Generating POT files ..."
    SOURCES ${PROJECT_DOC_FILES}
    )
endif()


# The doc target will build all documentation targets.
add_custom_target(doc
    COMMENT "Building documentation with Sphinx")


foreach (target ${PROJECT_DOC_TARGETS})
    add_custom_target("${target}"
        COMMENT "Building ${target} documentation with Sphinx")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/forward.html"
        "${PGR_DOCUMENTATION_BUILD_DIR}/${target}/index.html")

    foreach (lang ${PROJECT_BUILD_LANGUAGES})
        add_custom_target(
            "${target}-${lang}"
            COMMAND ${SPHINX_EXECUTABLE}
            #-q  # Enable for quiet mode
            #-Q  # Enable for even more quiet mode
            -v   # increase verbosity
            -W
            --keep-going
            #-j auto
            #-E  # don't use a saved environment, always read all file
            #-a  # write all files; default is to only write new and changed files
            -b ${target}
            -D language='${lang}'
            -c "${PGR_DOCUMENTATION_SOURCE_DIR}"
            "${PGR_DOCUMENTATION_SOURCE_DIR}"
            "${PGR_DOCUMENTATION_BUILD_DIR}/${target}/${lang}"

            COMMENT "Generating ${target}/${lang} documentation ..."
            SOURCES ${PROJECT_DOC_FILES}  ${PROJECT_IMG_FILES}
            DEPENDS ${PROJECT_DOC_FILES}  ${PROJECT_IMG_FILES}
            )
        add_dependencies(${target} "${target}-${lang}")

    endforeach()
    add_dependencies(doc "${target}")
endforeach()

