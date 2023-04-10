\set QUIET 1

-- Tests for pgroutng.

-- Format the output for nice TAP.
\pset format unaligned
\pset tuples_only true
\pset pager

-- Revert all changes on failure.
\set ON_ERROR_ROLLBACK 1
\set ON_ERROR_STOP true

SET client_min_messages = WARNING;

CREATE EXTENSION IF NOT EXISTS pgtap;
CREATE EXTENSION IF NOT EXISTS vrprouting CASCADE;

BEGIN;

    \i tools/testers/sampledata.sql
    \i tools/testers/matrix_new_values.sql
    \i tools/testers/vrppdtw_data.sql
    \i tools/testers/solomon_100_rc101.data.sql
    \i tools/testers/no_crash_test.sql
    \i tools/testers/general_pgtap_tests.sql
    \i tools/testers/vroomdata.sql

END;
