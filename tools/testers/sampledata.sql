-- CopyRight(c) pgORpy developers
-- Creative Commons Attribution-Share Alike 3.0
-- License : https://creativecommons.org/licenses/by-sa/3.0/

DROP TABLE IF EXISTS bin_packing;
DROP TABLE IF EXISTS knapsack;
DROP TABLE IF EXISTS multiple_knapsack;

-- activate_python_start
CREATE OR REPLACE PROCEDURE activate_python_venv(venv text)
LANGUAGE plpython3u AS
$BODY$
    import os
    import sys

    if sys.platform in ('win32', 'win64', 'cygwin'):
        activate_this = os.path.join(venv, 'Scripts', 'activate_this.py')
    else:
        activate_this = os.path.join(venv, 'bin', 'activate_this.py')

    exec(open(activate_this).read(), dict(__file__=activate_this))
$BODY$;

-- activate_python_end

CREATE TABLE bin_packing(
  id INTEGER,
  weight INTEGER);

INSERT INTO bin_packing(id, weight)
VALUES
(1, 48), (2, 30), (3, 19), (4, 36), (5, 36), (6, 27), (7, 42), (8, 42), (9, 36), (10, 24), (11, 30);


CREATE TABLE knapsack(
  id INTEGER,
  weight INTEGER,
  cost INTEGER);

INSERT INTO knapsack(id, weight, cost)
VALUES
(1, 12, 4),
(2, 2, 2),
(3, 1, 1),
(4, 4, 10),
(5, 1, 2);

CREATE TABLE multiple_knapsack(
  id INTEGER,
  weight INTEGER,
  cost INTEGER);

INSERT INTO multiple_knapsack(id, weight, cost)
VALUES
(1, 48, 10),
(2, 30, 30),
(3, 42, 25),
(4, 36, 50),
(5, 36, 35),
(6, 48, 30),
(7, 42, 15),
(8, 42, 40),
(9, 36, 30),
(10, 24, 35),
(11, 30, 45),
(12, 30, 10),
(13, 42, 20),
(14, 36, 30),
(15, 36, 25);
