..
   ****************************************************************************
    pgOrtools Manual
    Copyright(c) pgOrtools Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************

.. _support:

Support
===============================================================================

pgOrtools community support is available through the
`pgRouting website <https://pgrouting.org/support.html>`_,
`documentation <https://vrp.pgrouting.org>`_, tutorials, mailing lists and others.

If you’re looking for :ref:`commercial support <support_commercial>`, find below
a list of companies providing pgOrtools development and consulting services.


Reporting Problems
-------------------------------------------------------------------------------

Bugs are reported and managed in an i
`issue tracker <https://github.com/pgRouting/pgortools/issues>`_. Please follow
these steps:

1. Search the tickets to see if your problem has already been reported.
   If so, add any extra context you might have found, or at least indicate that
   you too are having the problem. This will help us prioritize common issues.
2. If your problem is unreported, create a
   `new issue <https://github.com/pgRouting/pgortools/issues/new>`__ for it.
3. In your report include explicit instructions to replicate your issue.
   The best tickets include the exact SQL necessary to replicate a problem.
4. If you can test older versions of PostGIS for your problem, please do.
   On your ticket, note the earliest version the problem appears.
5. For the versions where you can replicate the problem, note the operating system
   and version of pgOrtools, pgRouting, PostGIS and PostgreSQL.
6. It is recommended to use the following wrapper on the problem to pin point the
   step that is causing the problem.

.. code-block:: sql

    SET client_min_messages TO debug;
      <your code>
    SET client_min_messages TO notice;



Mailing List and GIS StackExchange
-------------------------------------------------------------------------------

There are two mailing lists for pgOrtools hosted on OSGeo mailing list server
which are of pgRouting:

* User mailing list: https://lists.osgeo.org/mailman/listinfo/pgrouting-users
* Developer mailing list: https://discourse.osgeo.org/c/pgrouting/pgrouting-dev

For general questions and topics about how to use pgOrtools, please write to the user mailing list.

You can also ask at `GIS StackExchange <https://gis.stackexchange.com/>`_ and tag
the question with ``pgortools``. Find all questions tagged with ``pgortools``
under https://gis.stackexchange.com/questions/tagged/pgortools or subscribe to the
`pgOrtools questions feed <https://gis.stackexchange.com/feeds/tag?tagnames=pgortools&sort=newest>`_.


.. _support_commercial:

Commercial Support
-------------------------------------------------------------------------------

For users who require professional support, development and consulting services,
consider contacting any of the following organizations, which have significantly
contributed to the development of pgOrtools:

.. list-table::
   :widths: 100 160 200

   * - **Company**
     - **Offices in**
     - **Website**
   * - Georepublic
     - Germany, Japan
     - https://georepublic.info
   * - Paragon Corporation
     - United States
     - https://www.paragoncorporation.com
