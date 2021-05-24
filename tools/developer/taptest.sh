#!/bin/bash

set -e

if [[ -z  $1 ]]; then
    echo "Directory missing";
    echo "Usage"
    echo "tools/developer/taptest.sh <pgtap directory> [<postgresql options>]";
    echo "Examples:"
    echo "tools/developer/taptest.sh withPoints/undirected_equalityDD.sql -p 5433"
    exit 1;
fi

DIR="$1"
shift
PGFLAGS=$*

PGDATABASE="___vrp___test___"

dropdb --if-exists "${PGFLAGS}" "${PGDATABASE}"
createdb "${PGFLAGS}" "${PGDATABASE}"

echo "../../${DIR}"

cd tools/testers/
psql "$PGFLAGS"  -f setup_db.sql -d "${PGDATABASE}"
pg_prove -v --recurse --ext .sql "${PGFLAGS}"  -d "${PGDATABASE}" "../../${DIR}"
dropdb --if-exists "${PGFLAGS}" "${PGDATABASE}"
