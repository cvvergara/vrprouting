..
   ****************************************************************************
    vrpRouting Manual
    Copyright(c) vrpRouting Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************

|

* `Documentation <https://vrp.pgrouting.org/>`__ → `vrpRouting v0 <https://vrp.pgrouting.org/v0>`__
* Supported Versions
  `Latest <https://vrp.pgrouting.org/latest/en/concepts.html>`__
  (`v0 <https://vrp.pgrouting.org/v0/en/concepts.html>`__)

Concepts
===============================================================================

.. contents::

General documentation
-------------------------------------------------------------------------------

Vehicle Routing Problems `VRP` are **NP-hard** optimization problem, it generalises the travelling salesman problem (TSP).

- The objective of the VRP is to minimize the total route cost.
- There are several variants of the VRP problem,

**vrpRouting does not try to implement all variants.**


Characteristics
-------------------------------------------------------------------------------

- Capacitated Vehicle Routing Problem `CVRP` where The vehicles have limited carrying capacity of the goods.
- Vehicle Routing Problem with Time Windows `VRPTW` where the locations have time windows within which the vehicle's visits must be made.
- Vehicle Routing Problem with Pickup and Delivery `VRPPD` where a number of goods need to be moved from certain pickup locations to other delivery locations.


.. Rubric:: Limitations

- No multiple time windows for a location.
- Less vehicle used is considered better.
- Less total duration is better.
- Less wait time is better.


Pick & Delivery
...............................................................................

Problem: `CVRPPDTW` Capacitated Pick and Delivery Vehicle Routing problem with Time Windows

- Times are relative to `0`
- The vehicles

  - have start and ending service duration times.
  - have opening and closing times for the start and ending locations.
  - have a capacity.

- The orders

  - Have pick up and delivery locations.
  - Have opening and closing times for the pickup and delivery locations.
  - Have pickup and delivery duration service times.
  - have a demand request for moving goods from the pickup location to the delivery location.

- Time based calculations:

  - Travel time between customers is :math:`distance / speed`
  - Pickup and delivery order pair is done by the same vehicle.
  - A pickup is done before the delivery.

.. note To be defined


Inner queries
-------------------------------------------------------------------------------

.. pgr_matrix_start

A ``SELECT`` statement that returns the following columns:

::

    start_vid, end_vid, agg_cost

============= =============== ================================================
Column        Type            Description
============= =============== ================================================
**start_vid** |ANY-INTEGER|   Identifier of a node.
**end_vid**   |ANY-NUMERICAL| Identifier of a node
**agg_cost**  |ANY-NUMERICAL| Time to travel from ``start_vid`` to ``end_vid``
============= =============== ================================================

.. pgr_matrix_end


.. vroom_matrix_start

A ``SELECT`` statement that returns the following columns:

::

    start_id, end_id, duration [, cost]

=============== ================= ============== ==================================================
Column          Type              Default        Description
=============== ================= ============== ==================================================
**start_id**    ``ANY-INTEGER``                  Identifier of the start node.

**end_id**      ``ANY-INTEGER``                  Identifier of the end node.

**duration**    |interval|                       Time to travel from ``start_id`` to ``end_id``

**cost**        ``INTEGER``       *duration*     Cost to travel from ``start_id`` to ``end_id``
=============== ================= ============== ==================================================

.. vroom_matrix_end

.. pgr_orders_start

A ``SELECT`` statement that returns the following columns:

::

    id, amount
    p_id, p_tw_open, p_tw_close, [p_service,]
    d_id, d_tw_open, d_tw_close, [d_service]


================  ===================  =========== ================================================
Column            Type                 Default     Description
================  ===================  =========== ================================================
**id**            |ANY-INTEGER|                     Identifier of the pick-delivery order pair.
**amount**        |ANY-NUMERICAL|                   Number of units in the order
**p_id**          |ANY-INTEGER|                     The identifier of the pickup node.
**p_tw_open**     |ANY-NUMERICAL|                   The time, relative to 0, when the pickup location opens.
**p_tw_close**    |ANY-NUMERICAL|                   The time, relative to 0, when the pickup location closes.
**p_service**     |ANY-NUMERICAL|       0           The duration of the loading at the pickup location.
**d_id**          |ANY-INTEGER|                     The identifier of the delivery node.
**d_tw_open**     |ANY-NUMERICAL|                   The time, relative to 0, when the delivery location opens.
**d_tw_close**    |ANY-NUMERICAL|                   The time, relative to 0, when the delivery location closes.
**d_service**     |ANY-NUMERICAL|       0           The duration of the unloading at the delivery location.
================  ===================  =========== ================================================

.. pgr_orders_end

.. pgr_orders_e_start

A ``SELECT`` statement that returns the following columns:

::

    id, amount
    p_x, p_y, p_tw_open, p_tw_close, [p_service,]
    d_x, d_y, d_tw_open, d_tw_close, [d_service]


================  ===================  =========== ================================================
Column            Type                 Default     Description
================  ===================  =========== ================================================
**id**            |ANY-INTEGER|                     Identifier of the pick-delivery order pair.
**amount**        |ANY-INTEGER|                     Number of units in the order
**p_x**           |ANY-NUMERICAL|                   :math:`x` value of the pick up location
**p_y**           |ANY-NUMERICAL|                   :math:`y` value of the pick up location
**p_tw_open**     |ANY-INTEGER|                     The time, relative to 0, when the pickup location opens.
**p_tw_close**    |ANY-INTEGER|                     The time, relative to 0, when the pickup location closes.
**p_service**     |ANY-INTEGER|         0           The duration of the loading at the pickup location.
**d_x**           |ANY-NUMERICAL|                   :math:`x` value of the delivery location
**d_y**           |ANY-NUMERICAL|                   :math:`y` value of the delivery location
**d_tw_open**     |ANY-INTEGER|                     The time, relative to 0, when the delivery location opens.
**d_tw_close**    |ANY-INTEGER|                     The time, relative to 0, when the delivery location closes.
**d_service**     |ANY-INTEGER|         0           The duration of the loading at the delivery location.
================  ===================  =========== ================================================

.. pgr_orders_e_end

.. vrp_pickDeliver start

A ``SELECT`` statement that returns the following columns:

::

    id, amount, requested_t
    p_id, p_tw_open, p_tw_close, [p_service,]
    d_id, d_tw_open, d_tw_close, [d_service]


================  ===================  =========== ================================================
Column            Type                 Default     Description
================  ===================  =========== ================================================
**id**            |ANY-INTEGER|                     Identifier of the pick-delivery order pair.
**amount**        |ANY-NUMERICAL|                   Number of units in the order
**requested_t**   |ANY-NUMERICAL|                   TODO define if it is going to be used
**p_id**          |ANY-INTEGER|                     The identifier of the pickup node.
**p_tw_open**     |ANY-NUMERICAL|                   The time, relative to 0, when the pickup location opens.
**p_tw_close**    |ANY-NUMERICAL|                   The time, relative to 0, when the pickup location closes.
**p_service**     |ANY-NUMERICAL|       0           The duration of the loading at the pickup location.
**d_id**          |ANY-INTEGER|                     The identifier of the delivery node.
**d_tw_open**     |ANY-NUMERICAL|                   The time, relative to 0, when the delivery location opens.
**d_tw_close**    |ANY-NUMERICAL|                   The time, relative to 0, when the delivery location closes.
**d_service**     |ANY-NUMERICAL|       0           The duration of the unloading at the delivery location.
================  ===================  =========== ================================================

.. vrp_pickDeliver end


.. shipments_start

A ``SELECT`` statement that returns the following columns:

::

    id, p_location_id [, p_setup, p_service], d_location_id [, d_setup, d_service]
    [, amount, skills, priority, p_data, d_data]


======================  =========================  =========== ================================================
Column                  Type                       Default     Description
======================  =========================  =========== ================================================
**id**                  ``ANY-INTEGER``                         Positive unique identifier of the shipment.

**p_location_id**       ``ANY-INTEGER``                         Positive identifier of the pickup location.

**p_setup**             |interval|                 |interval0|  Pickup setup duration.

**p_service**           |interval|                 |interval0|  Pickup service duration.

**d_location_id**       ``ANY-INTEGER``                         Positive identifier of the delivery location.

**d_setup**             |interval|                 |interval0|  Delivery setup duration.

**d_service**           |interval|                 |interval0|  Delivery service duration.

**amount**              ``ARRAY[ANY-INTEGER]``     Empty Array  Array of non-negative integers describing
                                                                multidimensional quantities such as number
                                                                of items, weight, volume etc.

                                                                - All shipments must have the same value of
                                                                  :code:`array_length(amount, 1)`

**skills**              ``ARRAY[INTEGER]``         Empty Array  Array of non-negative integers defining
                                                                mandatory skills.

**priority**            ``INTEGER``                0            Priority level of the shipment.

                                                                - Ranges from ``[0, 100]``

**p_data**              ``JSONB``                  '{}'::JSONB Any metadata information of the pickup shipment.

**d_data**              ``JSONB``                  '{}'::JSONB Any metadata information of the delivery shipment.
======================  =========================  =========== ================================================

Where:

:ANY-INTEGER: SMALLINT, INTEGER, BIGINT

.. shipments_end

.. pgr_vehicles_start

A ``SELECT`` statement that returns the following columns:

::

    id, capacity, [speed,]
    s_id, s_tw_open, s_tw_close, [s_service,]
    [e_id, e_tw_open, e_tw_close, e_service]



==================  =================== ================ ================================================
Column              Type                  Default           Description
==================  =================== ================ ================================================
**id**              |ANY-INTEGER|                         Identifier of the vehicle
**capacity**        |ANY-NUMERICAL|                       Capacity of the vehicle
**speed**           |ANY-NUMERICAL|      `1`              Average speed of the vehicle.
**s_id**            |ANY-INTEGER|                         The node identifier of the starting location, must match a node identifier in the matrix table.
**s_tw_open**       |ANY-NUMERICAL|      `0`              The time, relative to 0, when the starting location opens.
                                                          When `s_tw_open` column exist  then `s_tw_close` column is expected
                                                          Default value when (`s_tw_open`, s_tw_close) columns do not exist
**s_tw_close**      |ANY-NUMERICAL|      `INFINITY`       The time, relative to 0, when the starting location closes.
                                                          When `s_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(s_tw_open, s_tw_close)` columns do not exist
**s_service**       |ANY-NUMERICAL|      `0`              The duration of the loading at the starting location.
**e_id**            |ANY-INTEGER|        `s_id`           The node identifier of the ending location, must match a node identifier in the matrix table.
**e_tw_open**       |ANY-NUMERICAL|      `s_tw_open`      The time, relative to 0, when the ending location opens.
                                                          When `e_tw_open` column exist  then `e__tw_close` column is expected
                                                          Default value when (`e_tw_open`, e_tw_close) columns do not exist
**e_tw_close**      |ANY-NUMERICAL|      `s_tw_close`     The time, relative to 0, when the ending location closes.
                                                          When `e_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(e_tw_open`, e_tw_close)` columns do not exist
**e_service**       |ANY-NUMERICAL|      `s_service`      The duration of the unloading at the ending location.
==================  =================== ================ ================================================

Throws:
* When column `id` is missing
* When column `capacity` is missing
* When column `s_id` is missing
* When column `s_tw_open` exists but not `s_tw_close`
* When column `s_tw_close`exists but not `s_tw_open`
* When column `e_tw_open` exists but not `e_tw_close`
* When column `e_tw_close`exists but not `e_tw_open`

.. pgr_vehicles_end

.. pgr_vehicles_e_start

A ``SELECT`` statement that returns the following columns:

::

    id, capacity, [speed,]
    s_x, s_y, s_tw_open, s_tw_close, [s_service,]
    [e_x, e_y, e_tw_open, e_tw_close, e_service]

==================  =================== ================ ================================================
Column              Type                  Default           Description
==================  =================== ================ ================================================
**id**              |ANY-INTEGER|                         Identifier of the vehicle
**capacity**        |ANY-NUMERICAL|                       Capacity of the vehicle
**speed**           |ANY-NUMERICAL|      `1`              Average speed of the vehicle.
**s_x**             |ANY-NUMERICAL|                       :math:`x` value of the coordinate of the starting location.
**s_y**             |ANY-NUMERICAL|                       :math:`y` value of the coordinate of the starting location.
**s_tw_open**       |ANY-NUMERICAL|      `0`              The time, relative to 0, when the starting location opens.
                                                          When `s_tw_open` column exist  then `s_tw_close` column is expected
                                                          Default value when (`s_tw_open`, s_tw_close) columns do not exist
**s_tw_close**      |ANY-NUMERICAL|      `INFINITY`       The time, relative to 0, when the starting location closes.
                                                          When `s_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(s_tw_open, s_tw_close)` columns do not exist
**s_service**       |ANY-NUMERICAL|      `0`              The duration of the loading at the starting location.
**e_tw_open**       |ANY-NUMERICAL|      `s_tw_open`      The time, relative to 0, when the ending location opens.
                                                          When `e_tw_open` column exist  then `e__tw_close` column is expected
                                                          Default value when (`e_tw_open`, e_tw_close) columns do not exist
**e_tw_close**      |ANY-NUMERICAL|      `s_tw_close`     The time, relative to 0, when the ending location closes.
                                                          When `e_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(e_tw_open`, e_tw_close)` columns do not exist
**e_service**       |ANY-NUMERICAL|      `s_service`      The duration of the loading at the ending location.
**e_x**             |ANY-NUMERICAL|      `s_x`            :math:`x` value of the coordinate of the ending location.
                                                          Default value when `(e_x, e_y)` columns do not exist
                                                          Default value when `e_y` column does not exist
**e_y**             |ANY-NUMERICAL|      `s_y`            :math:`y` value of the coordinate of the ending location.
                                                          When `e_y` column exist  then `e_x` column is expected
                                                          Default value when `(e_x, e_y)` columns do not exist
==================  =================== ================ ================================================

Throws:
* When column `id` is missing
* When column `capacity` is missing
* When column `s_x` is missing
* When column `s_y` is missing
* When column `s_tw_open` exists but not `s_tw_close`
* When column `s_tw_close`exists but not `s_tw_open`
* When column `e_y` exists but not `e_x`
* When column `e_x` exists but not `e_y`
* When column `e_tw_open` exists but not `e_tw_close`
* When column `e_tw_close`exists but not `e_tw_open`

.. pgr_vehicles_e_end

.. vroom_vehicles_start

A ``SELECT`` statement that returns the following columns:

::

    id, start_id, end_id
    [, capacity, skills, tw_open, tw_close, speed_factor, max_tasks, data]


======================  ======================== =================== ================================================
Column                  Type                     Default             Description
======================  ======================== =================== ================================================
**id**                  ``ANY-INTEGER``                              Positive unique identifier of the vehicle.

**start_id**            ``ANY-INTEGER``                              Positive identifier of the vehicle start location.

**end_id**              ``ANY-INTEGER``                              Positive identifier of the vehicle end location.

**capacity**            ``ARRAY[ANY-INTEGER]``   Empty Array         Array of non-negative integers describing
                                                                     multidimensional quantities such as
                                                                     number of items, weight, volume etc.

                                                                     - All vehicles must have the same value of
                                                                       :code:`array_length(capacity, 1)`

**skills**              ``ARRAY[INTEGER]``       Empty Array         Array of non-negative integers defining
                                                                     mandatory skills.

**tw_open**             |timestamp|              |tw_open_default|   Time window opening time.

**tw_close**            |timestamp|              |tw_close_default|  Time window closing time.

**speed_factor**        ``ANY-NUMERICAL``        :math:`1.0`         Vehicle travel time multiplier.

                                                                     - Max value of speed factor for a vehicle shall not be
                                                                       greater than 5 times the speed factor of any other vehicle.

**max_tasks**           ``INTEGER``              :math:`2147483647`  Maximum number of tasks in a route for the vehicle.

                                                                     - A job, pickup, or delivery is counted as a single task.

**data**                ``JSONB``                '{}'::JSONB         Any metadata information of the vehicle.
======================  ======================== =================== ================================================

**Note**:

- At least one of the ``start_id`` or ``end_id`` shall be present.
- If ``end_id`` is omitted, the resulting route will stop at the last visited task, whose choice is determined by the optimization process.
- If ``start_id`` is omitted, the resulting route will start at the first visited task, whose choice is determined by the optimization process.
- To request a round trip, specify both ``start_id`` and ``end_id`` as the same index.
- A vehicle is only allowed to serve a set of tasks if the resulting load at each route step is lower than the matching value in capacity for each metric. When using multiple components for amounts, it is recommended to put the most important/limiting metrics first.
- It is assumed that all delivery-related amounts for jobs are loaded at vehicle start, while all pickup-related amounts for jobs are brought back at vehicle end.
- :code:`tw_open ≤ tw_close`

.. vroom_vehicles_end

.. vrp_pickDeliver start

A ``SELECT`` statement that returns the following columns:

::

    id, capacity, [speed,]
    s_id, s_tw_open, s_tw_close, [s_service,]
    [e_id, e_tw_open, e_tw_close, e_service]



==================  =================== ================ ================================================
Column              Type                  Default           Description
==================  =================== ================ ================================================
**id**              |ANY-INTEGER|                         Identifier of the vehicle
**capacity**        |ANY-NUMERICAL|                       Capacity of the vehicle
**stops**           |ANY-NUMERICAL|      `[]`             Array of shipments identifiers indicating an initial solution
**s_id**            |ANY-INTEGER|                         The node identifier of the starting location, must match a node identifier in the matrix table.
**s_tw_open**       |ANY-NUMERICAL|      `0`              The time, relative to 0, when the starting location opens.
                                                          When `s_tw_open` column exist  then `s_tw_close` column is expected
                                                          Default value when (`s_tw_open`, s_tw_close) columns do not exist
**s_tw_close**      |ANY-NUMERICAL|      `INFINITY`       The time, relative to 0, when the starting location closes.
                                                          When `s_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(s_tw_open, s_tw_close)` columns do not exist
**s_service**       |ANY-NUMERICAL|      `0`              The duration of the loading at the starting location.
**e_id**            |ANY-INTEGER|        `s_id`           The node identifier of the ending location, must match a node identifier in the matrix table.
**e_tw_open**       |ANY-NUMERICAL|      `s_tw_open`      The time, relative to 0, when the ending location opens.
                                                          When `e_tw_open` column exist  then `e__tw_close` column is expected
                                                          Default value when (`e_tw_open`, e_tw_close) columns do not exist
**e_tw_close**      |ANY-NUMERICAL|      `s_tw_close`     The time, relative to 0, when the ending location closes.
                                                          When `e_tw_close` column exist  then `e_tw_open` column is expected
                                                          Default value when `(e_tw_open`, e_tw_close)` columns do not exist
**e_service**       |ANY-NUMERICAL|      `s_service`      The duration of the unloading at the ending location.
==================  =================== ================ ================================================

Throws:
* When column `id` is missing
* When column `capacity` is missing
* When column `s_id` is missing
* When column `s_tw_open` exists but not `s_tw_close`
* When column `s_tw_close`exists but not `s_tw_open`
* When column `e_tw_open` exists but not `e_tw_close`
* When column `e_tw_close`exists but not `e_tw_open`

.. pgr_pickDeliver end

.. breaks_start

A ``SELECT`` statement that returns the following columns:

::

    id, vehicle_id [, service, data]

====================  =========================  =========== ================================================
Column                Type                       Default      Description
====================  =========================  =========== ================================================
**id**                ``ANY-INTEGER``                         Positive unique identifier of the break.
                                                              (unique for the same vehicle).

**vehicle_id**        ``ANY-INTEGER``                         Positive unique identifier of the vehicle.

**service**           |interval|                 |interval0|  The break duration.

**data**              ``JSONB``                  '{}'::JSONB  Any metadata information of the break.
====================  =========================  =========== ================================================

.. breaks_end


.. jobs_start

A ``SELECT`` statement that returns the following columns:

::

    id, location_id
    [, setup, service, delivery, pickup, skills, priority, data]


====================  =========================  =========== ================================================
Column                Type                       Default     Description
====================  =========================  =========== ================================================
**id**                ``ANY-INTEGER``                        Positive unique identifier of the job.

**location_id**       ``ANY-INTEGER``                        Positive identifier of the job location.

**setup**             |interval|                 |interval0| Job setup duration.

**service**           |interval|                 |interval0| Job service duration.

**delivery**          ``ARRAY[ANY-INTEGER]``     Empty Array Array of non-negative integers describing
                                                             multidimensional quantities for delivery such
                                                             as number of items, weight, volume etc.

                                                             - All jobs must have the same value of
                                                               :code:`array_length(delivery, 1)`

**pickup**            ``ARRAY[ANY-INTEGER]``     Empty Array Array of non-negative integers describing
                                                             multidimensional quantities for pickup such as
                                                             number of items, weight, volume etc.

                                                             - All jobs must have the same value of
                                                               :code:`array_length(pickup, 1)`

**skills**            ``ARRAY[INTEGER]``         Empty Array Array of non-negative integers defining
                                                             mandatory skills.

**priority**          ``INTEGER``                0           Priority level of the job

                                                             - Ranges from ``[0, 100]``

**data**              ``JSONB``                  '{}'::JSONB Any metadata information of the job.
====================  =========================  =========== ================================================

Where:

:ANY-INTEGER: SMALLINT, INTEGER, BIGINT

.. jobs_end


.. time_windows_start

A ``SELECT`` statement that returns the following columns:

::

    id [, kind], tw_open, tw_close

====================  ====================================== =====================================================
Column                Type                                   Description
====================  ====================================== =====================================================
**id**                ``ANY-INTEGER``                         Positive unique identifier of the job,
                                                              pickup/delivery shipment, or break.

**kind**              ``CHAR``                                **Only required for shipments time windows**.
                                                              Value in ['p', 'd'] indicating whether
                                                              the time window is for:

                                                              - Pickup shipment, or
                                                              - Delivery shipment.

**tw_open**           |timestamp|                             Time window opening time.

**tw_close**          |timestamp|                             Time window closing time.
====================  ====================================== =====================================================

**Note**:

- All timings are in **seconds** when represented as an ``INTEGER``.
- Every row must satisfy the condition: :code:`tw_open ≤ tw_close`.
- It is up to users to decide how to describe time windows:

  - **Relative values**, e.g. [0, 14400] for a 4 hours time window starting at the beginning of the planning horizon. In that case all times reported in output with the arrival column are relative to the start of the planning horizon.
  - **Absolute values**, "real" timestamps. In that case all times reported in output with the arrival column can be interpreted as timestamps.

.. time_windows_end


.. _Getting_started:

Getting Started
-------------------------------------------------------------------------------

This is a simple guide to walk you through the steps of getting started
with vrpRouting. In this guide we will cover:

.. contents::
    :local:


.. _create_database:

Create a routing Database
...............................................................................

The first thing we need to do is create a database and load pgrouting in
the database. Typically you will create a database for each project. Once
you have a database to work in, your can load your data and build your
application in that database. This makes it easy to move your project
later if you want to to say a production server.

For Postgresql 9.2 and later versions

.. code-block:: bash

	createdb mydatabase
	psql mydatabase -c "create extension postgis"
	psql mydatabase -c "create extension pgrouting"


Handling Parameters
*******************************************************************************

To define a problem, several considerations have to be done, to get consistent results.
This section gives an insight of how parameters are to be considered.

- `Capacity and Demand Units Handling`_
- `Locations`_
- `Time Handling`_
- `Factor Handling`_


Capacity and Demand Units Handling
...............................................................................

The `capacity` of a vehicle, can be measured in:

- Volume units like :math:`m^3`.
- Area units like :math:`m^2` (when no stacking is allowed).
- Weight units like :math:`kg`.
- Number of boxes that fit in the vehicle.
- Number of seats in the vehicle

The `demand` request of the pickup-deliver orders must use the same units as the units used in the vehicle's `capacity`.

To handle problems like:  10 (equal dimension) boxes of apples and 5 kg of feathers that are to be transported (not packed in boxes).

If the vehicle's `capacity` is measured by `boxes`, a conversion of `kg of feathers` to `equivalent number of boxes` is needed.
If the vehicle's `capacity` is measured by `kg`, a conversion of `box of apples` to `equivalent number of kg` is needed.

Showing how the 2 possible conversions can be done

Let:
- :math:`f_boxes`: number of boxes that would be used for `1` kg of feathers.
- :math:`a_weight`: weight of `1` box of apples.

=============== ====================== ==================
Capacity Units  apples                  feathers
=============== ====================== ==================
boxes            10                     :math:`5 * f\_boxes`
kg              :math:`10 * a\_weight`       5
=============== ====================== ==================



Locations
*******************************************************************************

- When using the Euclidean signatures:

  - The vehicles have :math:`(x, y)` pairs for start and ending locations.
  - The orders Have :math:`(x, y)` pairs for pickup and delivery locations.

- When using a matrix:

  - The vehicles have identifiers for the start and ending locations.
  - The orders have identifiers for the pickup and delivery locations.
  - All the identifiers are indices to the given matrix.


Time Handling
...............................................................................

The times are relative to 0

Suppose that a vehicle's driver starts the shift at 9:00 am and ends the shift at 4:30 pm
and the service time duration is 10 minutes with 30 seconds.

All time units have to be converted

============ ================= ==================== ===================== =========
Meaning of 0   time units       9:00 am              4:30 pm               10 min 30 secs
============ ================= ==================== ===================== =========
0:00 am         hours            9                  16.5                   :math:`10.5 / 60  = 0.175`
9:00 am         hours            0                  7.5                    :math:`10.5 / 60  = 0.175`
0:00 am         minutes          :math:`9*60 = 54`  :math:`16.5*60 = 990`  10.5
9:00 am         minutes          0                  :math:`7.5*60 = 540`   10.5
============ ================= ==================== ===================== =========


.. _pd_factor:

Factor Handling
*******************************************************************************

.. TODO

TBD


Group of Functions
-------------------------------------------------------------------------------

TBD



Inner Queries
-------------------------------------------------------------------------------

TBD


Return columns & values
--------------------------------------------------------------------------------

TBD


Performance
-------------------------------------------------------------------------------

TBD

How to contribute
-------------------------------------------------------------------------------

.. rubric:: Wiki

* Edit an existing  `vrpRouting Wiki <https://github.com/pgRouting/vrprouting/wiki>`_ page.


.. rubric:: Adding Functionaity to vrpRouting

Consult the `developer's documentation <https://vrp.pgrouting.org/doxy/main/index.html>`_


.. rubric:: Indices and tables

* :ref:`genindex`
* :ref:`search`

.. |interval| replace:: ``INTERVAL`` or ``INTEGER``
.. |interval0| replace:: '00:00:00'::INTERVAL or :math:`0`
.. |timestamp| replace:: ``TIMESTAMP`` or ``INTEGER``
.. |tw_open_default| replace:: '1970-01-01 00:00:00'::TIMESTAMP or :math:`0`
.. |tw_close_default| replace:: '2106-02-07 06:28:15'::TIMESTAMP or :math:`4294967295`
