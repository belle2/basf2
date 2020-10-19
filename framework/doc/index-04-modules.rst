List of Core Modules
--------------------

This page contains a list of all modules in the framework package.

.. note::

	If you are just getting started with your first analysis in basf2:
	While these modules are at the core of basf2, you will probably not need
	to use them directly for common tasks.
	Instead, head over to the documentation of the :ref:`analysis`.

Of special importance to the users are the following modules:

* `EventInfoSetter` is used to create empty events and should usually be the
  first module in the path if new events are to be generated.
* `RootOutput` is used to write events to disk. This module can be added at any
  point in time and will usually write out the complete contents of the
  DataStore. For convenience there are special functions like 
  :py:func:`mdst.add_mdst_output`
  which will limit the output to certain collections.
* `RootInput` can be used to read events created with `RootOutput`


.. b2-modules::
   :package: framework
   :io-plots:
