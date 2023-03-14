:tocdepth: 3

=========================
Belle II Python Interface
=========================

.. module:: basf2

The Belle II Software has an extensive interface to the Python 3 scripting
language: All configuration and steering is done via python and in principle
also simple algorithms can be implemented directly in python. All main
functions are implemented in a module called `basf2` and most people will just
start their steering file or script with ::

    import basf2
    main = basf2.Path()

.. toctree::
   :glob:
   :maxdepth: 2

   modules_paths
   logging
   module_statistics
   conditions-database
   additional_functions
   other_modules