.. _GraFEI:


Graph-based Full Event Interpretation
=====================================

**Author: J. Cerasoli**

The Graph-based Full Event Interpretation is a machine learning tool based on deep grah neural networks to inclusively reconstruct events in Belle II.

Code documentation
##################

This section describes the code used to train the graFEI and apply it to Belle II data and simulation.

Basf2 modules
*************

Here the modules needed to run the graFEI in a basf2 are described. 
You can import these modules in a steering file with ``from grafei import MODULENAME``.

.. automodule:: grafei
   :members:


Other modules and functions
***************************

Here the core code of the graFEI is described.

.. autofunction:: grafei.modules.FlagBDecayModule.getObjectList

.. automodule:: grafei.model.config
   :members:

.. automodule:: grafei.model.create_trainer
   :members:

.. automodule:: grafei.model.data_utils
   :members:

.. automodule:: grafei.model.dataset_split
   :members:

.. automodule:: grafei.model.dataset_utils
   :members:

.. automodule:: grafei.model.edge_features
   :members:

.. automodule:: grafei.model.geometric_datasets
   :members:

.. automodule:: grafei.model.geometric_layers
   :members:

.. automodule:: grafei.model.geometric_network
   :members:

.. automodule:: grafei.model.lca2adjacency
   :members:

.. automodule:: grafei.model.metrics
   :members:

.. automodule:: grafei.model.multiTrain
   :members:

.. automodule:: grafei.model.normalize_features
   :members:

.. automodule:: grafei.model.tree_utils
   :members:

.. automodule:: grafei.modules.RootSaverModule
   :members:

.. automodule:: grafei.modules.isMostLikelyTempVarsModule
   :members:

