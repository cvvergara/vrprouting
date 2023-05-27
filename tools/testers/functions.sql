
/*******
Creating an initial solution
*******/
CREATE OR REPLACE PROCEDURE initial_solution()
AS
$BODY$
DECLARE
  day TEXT;
  days TEXT[];
BEGIN
  UPDATE vehicles SET stops = NULL;

  days := ARRAY['2019-12-09', '2019-12-10', '2019-12-11', '2019-12-12','2019-12-13'];

  FOREACH day IN ARRAY days LOOP
    WITH
    new_stops AS (
        SELECT vehicle_id AS id, array_remove(array_agg(order_id ORDER BY seq)::BIGINT[],-1::BIGINT) AS stops
        FROM vrp_pickDeliverRaw(
            format($$SELECT * FROM shipments WHERE date_trunc('day', p_tw_open) = '%1$s 00:00:00' ORDER BY id$$, day),
            format($$SELECT * FROM vehicles  WHERE date_trunc('day', s_tw_open) = '%1$s 00:00:00' ORDER BY id$$, day),
            $$SELECT start_vid, end_vid, agg_cost FROM timeMatrix$$,
            format($$SELECT * FROM tdm_raw('%1$s'::TIMESTAMP)$$, day),
            optimize => true, factor => 1::float, max_cycles => 1, stop_on_all_served => true)
          GROUP BY vehicle_id
        )
    UPDATE vehicles AS v
    SET stops = n.stops
    FROM new_stops AS n WHERE v.id = n.id;
  END LOOP;
END;
$BODY$
LANGUAGE plpgsql;

CALL initial_solution();
