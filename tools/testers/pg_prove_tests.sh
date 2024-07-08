#!/bin/bash
# ------------------------------------------------------------------------------
# Travis CI scripts
# Copyright(c) pgRouting Contributors
#
# Test pgRouting
# ------------------------------------------------------------------------------

set -e

PGUSER=$1
PGPORT=$2
if [ "b$PGPORT" = "b" ]
then
    PGPORT="5432"
else
    PGPORT="$PGPORT"
fi
PGDATABASE="___vrp___test___"
echo "$PGPORT"

dropdb --if-exists ___vrp___test___
createdb ___vrp___test___

echo "cd ./tools/testers/"
cd ./tools/testers/
echo "psql -p $PGPORT -U $PGUSER  -d $PGDATABASE -X -q -v ON_ERROR_STOP=1 --pset pager=off -f setup_db.sql"
psql -p "$PGPORT" -U "$PGUSER"  -d "$PGDATABASE" -X -q -v ON_ERROR_STOP=1 --pset pager=off -f setup_db.sql

pg_prove --failures --quiet --recurse "$PGPORT" -d "$PGDATABASE"  -U "$PGUSER"  ../../pgtap/

dropdb --if-exists ___vrp___test___
