Module Statistics
-----------------

The basf2 software takes extensive statistics during event processing about the
memory consumption and execution time of all modules. For most users a simple
print of the `statistics` object will be enough and creates a text table of the
execution times and memory conumption::

    import basf2
    print(basf2.statistics)

However the statistics object provides full access to all the separate values
directly in python if needed. See
:download:`module_statistics.py </framework/examples/module_statistics.py>` for a full example.

.. note::

    The memory consumption is measured by looking into :file:`/proc/{PID}/statm`
    between execution calls so for short running modules this might not be
    `accurate <https://stackoverflow.com/a/30799817/3596741>`_
    but it should give a general idea.

.. attribute:: basf2.statistics

   Global instance of a `ProcessStatistics <basf2.ProcessStatistics>` object containing all the statistics

.. autoclass:: basf2.ProcessStatistics
   :members:
   :special-members:
   :exclude-members: __init__
