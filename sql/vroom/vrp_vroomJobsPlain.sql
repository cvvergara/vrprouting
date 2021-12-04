/*PGR-GNU*****************************************************************
File: vrp_vroomJobsPlain.sql

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438@gmail.com

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

/*
signature start

.. code-block:: none

    vrp_vroomJobsPlain(
      Jobs SQL, Jobs Time Windows SQL,
      Vehicles SQL, Breaks SQL, Breaks Time Windows SQL,
      Matrix SQL [, exploration_level] [, timeout])  -- Experimental on v0.2

    RETURNS SET OF
    (seq, vehicle_seq, vehicle_id, step_seq, step_type, task_id,
     arrival, travel_time, service_time, waiting_time, load)

signature end

default signature start

.. code-block:: none

    vrp_vroomJobsPlain(
      Jobs SQL, Jobs Time Windows SQL,
      Vehicles SQL, Breaks SQL, Breaks Time Windows SQL,
      Matrix SQL)

    RETURNS SET OF
    (seq, vehicle_seq, vehicle_id, step_seq, step_type, task_id,
     arrival, travel_time, service_time, waiting_time, load)

default signature end
*/

-- v0.2
CREATE FUNCTION vrp_vroomJobsPlain(
    TEXT,  -- jobs_sql (required)
    TEXT,  -- jobs_time_windows_sql (required)
    TEXT,  -- vehicles_sql (required)
    TEXT,  -- breaks_sql (required)
    TEXT,  -- breaks_time_windows_sql (required)
    TEXT,  -- matrix_sql (required)

    exploration_level SMALLINT DEFAULT 5,
    timeout INTEGER DEFAULT -1,

    OUT seq BIGINT,
    OUT vehicle_seq BIGINT,
    OUT vehicle_id BIGINT,
    OUT step_seq BIGINT,
    OUT step_type INTEGER,
    OUT task_id BIGINT,
    OUT arrival INTEGER,
    OUT travel_time INTEGER,
    OUT service_time INTEGER,
    OUT waiting_time INTEGER,
    OUT load BIGINT[])
RETURNS SETOF RECORD AS
$BODY$
BEGIN
    IF exploration_level < 0 OR exploration_level > 5 THEN
        RAISE EXCEPTION 'Invalid value found on ''exploration_level'''
        USING HINT = format('Value found: %s. It must lie in the range 0 to 5 (inclusive)', exploration_level);
    END IF;

    RETURN QUERY
    SELECT *
    FROM _vrp_vroom(_pgr_get_statement($1), _pgr_get_statement($2), NULL, NULL,
                    _pgr_get_statement($3), _pgr_get_statement($4),
                    _pgr_get_statement($5), _pgr_get_statement($6),
                    exploration_level, timeout, 1::SMALLINT, true);
END;
$BODY$
LANGUAGE plpgsql VOLATILE;


-- COMMENTS

COMMENT ON FUNCTION vrp_vroomJobsPlain(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, SMALLINT, INTEGER)
IS 'vrp_vroomJobsPlain
 - EXPERIMENTAL
 - Parameters:
   - Jobs SQL with columns:
       id, location_index [, service, delivery, pickup, skills, priority, time_windows]
   - Jobs Time Windows SQL with columns:
       id, tw_open, tw_close
   - Vehicles SQL with columns:
       id, start_index, end_index
       [, service, delivery, pickup, skills, priority, time_window, breaks_sql, steps_sql]
   - Breaks SQL with columns:
       id [, service]
   - Breaks Time Windows SQL with columns:
       id, tw_open, tw_close
   - Matrix SQL with columns:
       start_vid, end_vid, agg_cost
- Optional parameters
   - exploration_level := 5::SMALLINT
   - timeout := -1
 - Documentation:
   - ${PROJECT_DOC_LINK}/vrp_vroomJobsPlain.html
';
