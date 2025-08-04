#!/bin/bash
# ------------------------------------------------------------------------------
# /*PGR-GNU*****************************************************************
# File: pg_prove_tests.sh
# Copyright (c) 2016 pgRouting developers
# Mail: project@pgrouting.org
# ------
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# ********************************************************************PGR-GNU*/

# Copyright(c) pgRouting Contributors
#
# Test pgRouting
# ------------------------------------------------------------------------------

set -e

PGDATABASE="___vrp___test___"

while [[ $# -gt 0 ]]; do
  case $1 in
    -U)
      PGUSER=(-U "$2")
      shift
      shift
      ;;
    -d)
      PGDATABASE="$2"
      shift
      shift
      ;;
    -c|--clean)
      CLEANDB=YES
      shift
      ;;
    -p)
      PGPORT=(-p "$2")
      shift
      shift
      ;;
    -*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
        POSITIONAL_ARGS+=("$1") # save positional arg
        shift # past argument
        ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

echo PGUSER= "${PGUSER[@]}"
echo PGPORT= "${PGPORT[@]}"
echo PGDATABASE= "${PGDATABASE}"
echo CLEANDB= "${CLEANDB}"

if [ -n "${CLEANDB}" ]; then
    echo "Recreating database ${PGDATABASE}"
    dropdb --if-exists "${PGPORT[@]}" "${PGUSER[@]}" "${PGDATABASE}"
    createdb "${PGPORT[@]}" "${PGUSER[@]}" "${PGDATABASE}"
fi

cd ./tools/testers/
psql "${PGPORT[@]}" "${PGUSER[@]}" -d "${PGDATABASE}" -X -q -v ON_ERROR_STOP=1 --pset pager=off -f setup_db.sql

pg_prove --failures --recurse "${PGPORT[@]}" "${PGUSER[@]}" -d "${PGDATABASE}" ../../pgtap/

# database wont be removed unless script does not fails
if [ -n "$CLEANDB" ]; then
    dropdb --if-exists "${PGPORT[@]}" "${PGUSER[@]}" "${PGDATABASE}"
fi
