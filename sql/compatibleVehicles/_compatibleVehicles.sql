/*PGR-GNU*****************************************************************
File: _compatibleVehicles.sql

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2021 Celia Virginia Vergara Castillo
Copyright (c) 2021 Joseph Emile Honour Percival

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

CREATE OR REPLACE FUNCTION _vrp_compatibleVehicles(
  TEXT, -- orders SQL
  TEXT, -- vehicles SQL
  TEXT, -- matrix SQL
  TEXT, -- time dependant multipliers SQL

  FLOAT, -- factor
  BOOLEAN, -- 0 for timestamps, 1 for raw

  OUT order_id BIGINT,
  OUT vehicle_id BIGINT)
RETURNS SETOF RECORD AS
'MODULE_PATHNAME'
LANGUAGE c VOLATILE STRICT;

-- COMMENTS

COMMENT ON FUNCTION _vrp_compatibleVehicles(TEXT, TEXT, TEXT, TEXT, FLOAT, BOOLEAN)
IS '_vrp_compatibleVehicles is an internal function';
