List of Core Modules
--------------------

This page contains a list of all modules in the framework package. Of special
importance to the users are the following modules:

* `EventInfoSetter` is used to create empty events and should usually be the
  first module in the path if new events are to be generated.
* `RootOutput` is used to write events to disk. This module can be added at any
  point in time and will usually write out the complete contents of the
  DataStore. For convenince there are special functions like `add_mdst_output`
  which will limit the output to certain collections.
* `RootInput` can be used to read events created with `RootOutput`


.. b2-modules::
   :package: framework
   :io-plots:
