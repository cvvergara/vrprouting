..
   ****************************************************************************
    pgORpy Manual
    Copyright(c) pgORpy Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************

|

*******************************************************************************
Table of Contents
*******************************************************************************

pgORpy brings to the database Operations Research tools from `OR-tools
<https://developers.google.com/optimization>`__

This is the manual for pgORpy |release|.

.. image:: _static/images/ccbysa.png
    :align: left
    :alt: Creative Commons Attribution-Share Alike 3.0 License

The pgORpy Manual is licensed under a `Creative Commons Attribution-Share Alike 3.0 License
<https://creativecommons.org/licenses/by-sa/3.0/>`__.

Feel free to use this documentation material any way you like, but we ask that
you attribute credit to the pgORpy project and wherever possible, a link back
to https://pgrouting.org.

General
===============================================================================

.. toctree::
    :maxdepth: 2

    introduction
    installation
    support
    concepts

- :doc:`sampledata` that is used in the examples of this manual.

.. toctree::
   :hidden:

   sampledata

Functions
===============================================================================

Utilities
-------------------------------------------------------------------------------

* :doc:`por_version` - Get pgORpy's version information.
* :doc:`por_full_version` - Get pgORpy's details of version.

.. toctree::
   :hidden:

   por_version
   por_full_version

OR-tools
-------------------------------------------------------------------------------

- :doc:`por_bin_packing`
- :doc:`por_knapsack`
- :doc:`por_multiple_knapsack`

.. toctree::
   :hidden:

   por_bin_packing
   por_knapsack
   por_multiple_knapsack

Release Notes
===============================================================================

.. toctree::

   release_notes

.. rubric:: Indices and tables

* :ref:`genindex`
* :ref:`search`
