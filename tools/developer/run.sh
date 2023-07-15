#!/bin/bash

set -e

DIR=$(git rev-parse --show-toplevel)
pushd "${DIR}" > /dev/null || exit 1

# The next two lines need to be executed only once
# pushd tools/testers/ ; tar -xf matrix_new_values.tar.gz; popd
# sudo apt-get install libssl-dev libasio-dev libglpk-dev

# copy this file into the root of your repository
# adjust to your needs

VERSION=$(grep -Po '(?<=project\(VRPROUTING VERSION )[^;]+' CMakeLists.txt)
echo "pgRouting VERSION ${VERSION}"

# set up your postgres version, port and compiler (if more than one)
PGVERSION="15"
PGPORT="5432"
#PGBIN="/usr/lib/postgresql/${PGVERSION}/bin"
PGINC="/usr/include/postgresql/${PGVERSION}/server"
# When more than one compiler is installed
GCC=""

QUERIES_DIRS=$(ls docqueries -1)
TAP_DIRS=$(ls pgtap -1)

QUERIES_DIRS=QUERIES_DIRS=$(ls docqueries -1)
TAP_DIRS=$(ls pgtap -1)

function install_vroom {
    cd "${DIR}"
    rm -rf "./vroom-${VROOMVER}"
    git clone --depth 1 --branch "${VROOMVER}" https://github.com/VROOM-Project/vroom "./vroom-${VROOMVER}"
    pushd "./vroom-${VROOMVER}"
    git submodule update --init
    cd src/
    sed -i 's/CXXFLAGS = /CXXFLAGS = -fPIC /' makefile
    make
    popd
}

function install_data {
    cd "${DIR}"
    pushd tools/testers
    tar -xf matrix_new_values.tar.gz
    popd
}

function set_cmake {
    # Using all defaults
    #cmake ..

    # Options Release RelWithDebInfo MinSizeRel Debug
    #cmake  -DCMAKE_BUILD_TYPE=Debug ..

    # Additional debug information
    #cmake -DPgRouting_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug ..

    # with documentation (like the one the website)
    #cmake  -DDOC_USE_BOOTSTRAP=ON -DWITH_DOC=ON ..

    # with developers documentation
    #cmake  -DWITH_DOC=ON -DBUILD_DOXY=ON ..

    #CXX=clang++ CC=clang cmake -DPOSTGRESQL_BIN=${PGBIN} -DCMAKE_BUILD_TYPE=Debug -DVROOM_INSTALL_PATH="${DIR}/vroom-${VROOMVER}" ..
    CXX=clang++ CC=clang cmake "-DPostgreSQL_INCLUDE_DIR=${PGINC}" -DCMAKE_BUILD_TYPE=Debug  -DVROOM_INSTALL_PATH="${DIR}/vroom-${VROOMVER}" ..

    #cmake "-DPostgreSQL_INCLUDE_DIR=${PGINC}" -DCMAKE_BUILD_TYPE=Debug -DWITH_DOC=ON -DVROOM_INSTALL_PATH="${DIR}/vroom-${VROOMVER}" ..
}

function tap_test {
    echo --------------------------------------------
    echo pgTap test all
    echo --------------------------------------------

    PGDATABASE="___vrp___pgtaptest___"
    dropdb --if-exists -p $PGPORT "$PGDATABASE"
    createdb  -p $PGPORT "$PGDATABASE"
    echo "testing on port $PGPORT"

    pushd ./tools/testers/
    psql -p "$PGPORT" -U vicky -d  "$PGDATABASE" -X  -v ON_ERROR_STOP=1 --pset pager=off -f setup_db.sql
    popd
    pg_prove -U vicky --failures --quiet --recurse -p "$PGPORT" -d "$PGDATABASE"  pgtap/
    #dropdb  -p $PGPORT "$PGDATABASE"
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
    make doc
    #make linkcheck
    #rm -rf doxygen/*
    #make doxy
    popd > /dev/null || exit 1
}

function build {
    pushd build > /dev/null || exit 1
    set_cmake
    make -j 16
    #make VERBOSE=1
    sudo make install
    popd > /dev/null || exit 1

}

function test_compile {

    set_compiler "${GCC}"

    #install_vroom
    install_data
    build

    echo --------------------------------------------
    echo  Execute documentation queries
    echo --------------------------------------------
    for d in ${QUERIES_DIRS}
    do
        #tools/testers/doc_queries_generator.pl  -alg "${d}" -documentation  -pgport "${PGPORT}"
        #tools/testers/doc_queries_generator.pl  -alg "${d}" -debug1  -pgport "${PGPORT}"
        tools/testers/doc_queries_generator.pl  -alg "${d}" -pgport "${PGPORT}"
    done


    echo --------------------------------------------
    echo  Execute tap_directories
    echo --------------------------------------------
    for d in ${TAP_DIRS}
    do
        bash taptest.sh  "${d}" "-p ${PGPORT}"
    done

    exit 0
    tap_test
    build_doc
    tools/testers/doc_queries_generator.pl -pgport $PGPORT

    action_tests
}
test_compile
