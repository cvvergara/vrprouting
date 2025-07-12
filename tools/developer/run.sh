#!/bin/bash

set -e

DIR=$(git rev-parse --show-toplevel)
pushd "${DIR}" > /dev/null || exit 1

# The next two lines need to be executed only once
# pushd tools/testers/ ; tar -xf matrix_new_values.tar.gz; popd
# sudo apt-get install libssl-dev libasio-dev libglpk-dev

# copy this file into the root of your repository
# adjust to your needs

VERSION=$(grep -Po '(?<=project\(PORTOOLS_PY VERSION )[^;]+' CMakeLists.txt)
echo "PORTOOLS_PY VERSION ${VERSION}"

VENV="$DIR/../env-ort"

# set up your postgres version, port and compiler (if more than one)
PGVERSION="15"
PGPORT="5432"
PGBIN="/usr/lib/postgresql/${PGVERSION}/bin"
PGINC="/usr/include/postgresql/${PGVERSION}/server"
# When more than one compiler is installed
GCC=""

QUERIES_DIRS=$(ls docqueries -1)
TAP_DIRS=$(ls pgtap -1)


QUERIES_DIRS="
"

TAP_DIRS="
"

function set_cmake {
    cmake "-DPOSTGRESQL_BIN=${PGBIN}" "-DPostgreSQL_INCLUDE_DIR=${PGINC}" \
        -DWITH_DOC=ON -DBUILD_DOXY=ON ..
}

function tap_test {
    echo --------------------------------------------
    echo pgTap test all
    echo --------------------------------------------

    dropdb --if-exists -p $PGPORT ___por___test___
    createdb  -p $PGPORT ___por___test___
    bash tools/testers/pg_prove_tests.sh vicky $PGPORT
}

function action_tests {
    echo --------------------------------------------
    echo  Update signatures
    echo --------------------------------------------

    bash tools/scripts/get_signatures.sh -p ${PGPORT}
    tools/scripts/notes2news.pl
    bash tools/scripts/test_signatures.sh
    bash tools/scripts/test_shell.sh
    bash tools/scripts/test_license.sh
    bash tools/scripts/code_checker.sh
    tools/testers/doc_queries_generator.pl  -documentation  -pgport $PGPORT
}

function set_compiler {
    echo ------------------------------------
    echo ------------------------------------
    echo "Compiling with G++-$1"
    echo ------------------------------------

    if [ -n "$1" ]; then
        update-alternatives --set gcc "/usr/bin/gcc-$1"
    fi
}

function build_doc {
    pushd build > /dev/null || exit 1
    #rm -rf doc/*
    #make doc
    #make linkcheck
    rm -rf doxygen/*
    make doxy
    popd > /dev/null || exit 1
}

function build {
    pushd build > /dev/null || exit 1
    set_cmake
    make #-j 16
    #make VERBOSE=1
    sudo make install
    popd > /dev/null || exit 1

}

function check {
    pushd build > /dev/null || exit 1
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
    cppcheck --project=compile_commands.json -q
    popd > /dev/null || exit 1
}

function test_compile {

    set_compiler "${GCC}"

    build

    echo --------------------------------------------
    echo  Execute documentation queries
    echo --------------------------------------------
    for d in ${QUERIES_DIRS}
    do
        #tools/testers/doc_queries_generator.pl  -alg "${d}" -doc  -pgport "${PGPORT}" -venv "${VENV}"
        #tools/testers/doc_queries_generator.pl  -alg "${d}" -debug1  -pgport "${PGPORT}" -venv "${VENV}"
        tools/testers/doc_queries_generator.pl  -alg "${d}" -pgport "${PGPORT}" -venv "${VENV}"
    done


    echo --------------------------------------------
    echo  Execute tap_directories
    echo --------------------------------------------
    for d in ${TAP_DIRS}
    do
        bash taptest.sh  "${d}" "-p ${PGPORT}"
    done

    tap_test
    exit 0
    tools/testers/doc_queries_generator.pl -pgport "${PGPORT}" -venv "${VENV}"
    build_doc
    tap_test

    action_tests
}
test_compile
