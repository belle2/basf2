.. _general_modpath:

Modules and Paths
-----------------

A typical data processing chain consists of a linear arrangement of smaller
processing blocks, called :py:class:`Modules <basf2.Module>`. Their tasks vary
from simple ones like reading data from a file to complex tasks like the full
detector simulation or the tracking.

In basf2 all work is done in modules, which means that even the reading of data
from disk and writing it back is done in modules. They live in a
:py:class:`Path <basf2.Path>`, which corresponds to a container where the
modules are arranged in a strict linear order. The specific selection and
arrangement of modules depend on the user's current task. When processing data,
the framework executes the modules of a path, starting with the first one and
proceeding with the module next to it. The modules are executed one at a time,
exactly in the order in which they were placed into the path.

Modules can have conditions attached to them to steer the processing flow
depending of the outcome of the calculation in each module.

The data, to be processed by the modules, is stored in a common storage, the
DataStore. Each module has read and write access to the storage. In addition
there's also non-event data, the so called conditions, which will be loaded from
a central conditions database and are available in the DBStore.

.. _framework_modpath_diagram:

.. figure:: modules_paths.png
  :width: 40em

  Schematic view of the processing flow in the Belle II Software


.. Functions concerning modules and Paths
.. ++++++++++++++++++++++++++++++++++++++

Usually each script needs to create a new path using `Path() <basf2.Path>`, add
all required modules in the correct order and finally call `process <basf2.process>` on
the fully configured path.

.. warning:: Preparing a `Path <basf2.Path>` and adding `Modules <basf2.Module>` to it **does not
   execute anything**, it only prepares the computation which is only done when
   `process <basf2.process>` is called.

The following functions are all related to the handling of modules and paths:

.. autofunction:: basf2.create_path
.. autofunction:: basf2.register_module
.. autofunction:: basf2.set_module_parameters
.. autofunction:: basf2.print_params
.. autofunction:: basf2.print_path
.. autofunction:: basf2.process


The Module Object
+++++++++++++++++

Unless you develop your own module in Python you should always instantiate new
modules by calling `register_module` or `Path.add_module`.

.. autoclass:: basf2.Module
   :members:

The Path Object
+++++++++++++++

.. autoclass:: basf2.Path
   :members:
   :special-members:
   :exclude-members: __init__, __str__

Other Related Classes
+++++++++++++++++++++

.. autoclass:: basf2.ModuleParamInfo
   :members:
.. autoclass:: basf2.ModulePropFlags
.. autoclass:: basf2.AfterConditionPath
