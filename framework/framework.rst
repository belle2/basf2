=========
Framework
=========

basf2
=====
.. automodule:: basf2
   :members:

   .. autofunction:: basf2.get_random_seed
   .. autofunction:: basf2.set_random_seed

..
  .. autofunction:: basf2.update_file_metadata



Modules and Paths
-----------------
.. autoclass:: basf2.Path
   :members:
   :special-members:

.. autoclass:: basf2.Module
   :members:

.. autoclass:: basf2.ModulePropFlags
   :members:

.. autoclass:: basf2.AfterConditionPath
   :members:


Logging
-------

Logging in basf2 from within Python is quite similar to C++, and mainly consists of the functions:

.. autofunction:: basf2.B2FATAL
.. autofunction:: basf2.B2ERROR
.. autofunction:: basf2.B2WARNING
.. autofunction:: basf2.B2INFO
.. autofunction:: basf2.B2RESULT
.. autofunction:: basf2.B2DEBUG

.. autoclass:: basf2.LogPythonInterface
   :members:
   :undoc-members:

.. autoclass:: basf2.LogConfig
   :members:
   :undoc-members:

.. autoclass:: basf2.LogInfo
   :members:
   :undoc-members:

.. autoclass:: basf2.LogLevel
   :members:
   :undoc-members:


Module Statistics
-----------------

.. currentmodule:: basf2.ProcessStatisticsPython

.. autoclass:: basf2.ProcessStatisticsPython
   :members:
   :undoc-members:


Database
--------
.. autofunction:: basf2.use_central_database
.. autofunction:: basf2.use_database_chain
.. autofunction:: basf2.use_local_database
.. autofunction:: basf2.reset_database


Utilities
=========

.. automodule:: root_pandas
   :members:

.. automodule:: pdg
   :members:

.. automodule:: interactive
   :members:

.. automodule:: conditions_db
   :members:

.. automodule:: hep_ipython_tools
   :members:
   :imported-members:

.. automodule:: pager
   :members:


B2Tools
-------
.. automodule:: B2Tools.b2latex
   :members:
   :show-inheritance:

.. automodule:: B2Tools.format
   :members:
   :show-inheritance:

..
   Test
   ----
   
   ..
       .. automodule:: Belle2
      :members:
      :imported-members:
      :undoc-members:
   
   .. currentmodule:: Belle2
   
   .. autoclass:: PyStoreArray
      :members:
      :private-members:
      :inherited-members:
      :special-members:
      :undoc-members:
   ..
       .. autoclass:: PyStoreArray
      :members:
      :private-members:
      :inherited-members:
      :special-members:
      :undoc-members:
