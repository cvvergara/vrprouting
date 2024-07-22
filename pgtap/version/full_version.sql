
BEGIN;

UPDATE edge_table SET cost = sign(cost), reverse_cost = sign(reverse_cost);
SELECT plan(5);


SELECT has_function('vrp_full_version');
SELECT has_function('vrp_full_version', ARRAY[]::text[]);
SELECT function_returns('vrp_full_version', ARRAY[]::text[], 'record');

SELECT set_eq(
    $$SELECT  proargnames from pg_proc where proname = 'vrp_full_version'$$,
    $$SELECT  '{"version","build_type","compile_date","library","system","postgresql","compiler","hash"}'::TEXT[] $$
);

-- parameter types
SELECT set_eq(
    $$SELECT  proallargtypes from pg_proc where proname = 'vrp_full_version'$$,
    $$SELECT  '{25,25,25,25,25,25,25,25}'::OID[] $$
);


SELECT finish();
ROLLBACK;
