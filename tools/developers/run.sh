#!/bin/bash

set -e

DIR=$(git rev-parse --show-toplevel)
pushd "${DIR}" > /dev/null || exit 1

# The next two lines need to be executed only once
# pushd tools/testers/ ; tar -xf matrix_new_values.tar.gz; popd
# sudo apt-get install libssl-dev libasio-dev libglpk-dev

# copy this file into the root of your repository
# adjust to your needs

VERSION=$(grep -Po '(?<=project\(PGORPY VERSION )[^;]+' CMakeLists.txt)
echo "PGORPY VERSION ${VERSION}"

VENV="$DIR/../env-ort"

# set up your postgres version, port and compiler (if more than one)
PGVERSION="17"
PGPORT="5432"
PGBIN="/usr/lib/postgresql/${PGVERSION}/bin"
PGINC="/usr/include/postgresql/${PGVERSION}/server"

QUERIES_DIRS="
version
"

TAP_DIRS="
version
"

function set_cmake {
    cmake "-DPOSTGRESQL_BIN=${PGBIN}" "-DPostgreSQL_INCLUDE_DIR=${PGINC}" \
        -DBUILD_HTML=ON -DPROJECT_DEBUG=ON ..
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

    #bash tools/scripts/get_signatures.sh -p ${PGPORT}
    .github/scripts/notes2news.pl
    bash .github/scripts/test_signatures.sh
    bash .github/scripts/test_shell.sh
    bash .github/scripts/test_license.sh
    tools/testers/doc_queries_generator.pl --documentation  -pgport $PGPORT
}

function build_doc {
    pushd build > /dev/null || exit 1
    rm -rf doc/*
    make doc
    popd > /dev/null || exit 1
}

function build {
    pushd build > /dev/null || exit 1
    set_cmake
    make
    sudo make install
    popd > /dev/null || exit 1

}

function test_compile {
    build

    echo --------------------------------------------
    echo  Execute documentation queries
    echo --------------------------------------------
    for d in ${QUERIES_DIRS}
    do
        tools/testers/doc_queries_generator.pl  --alg "${d}" --doc  --port "${PGPORT}" -venv "${VENV}"
        tools/testers/doc_queries_generator.pl  --alg "${d}" --level=DEBUG3  --port "${PGPORT}" -venv "${VENV}"
        tools/testers/doc_queries_generator.pl  --alg "${d}" --port "${PGPORT}" -venv "${VENV}"
    done


    echo --------------------------------------------
    echo  Execute tap_directories
    echo --------------------------------------------
    for d in ${TAP_DIRS}
    do
        bash taptest.sh  "${d}" "-p ${PGPORT}"
    done

    echo "build doc"
    build_doc
    echo "action tests"
    action_tests
    echo "tap tests"
    tap_test
}

test_compile
