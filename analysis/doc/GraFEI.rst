.. _GraFEI:


Graph-based Full Event Interpretation
=====================================

**Author: J. Cerasoli**

The **Graph-based Full Event Interpretation** (graFEI) is a machine learning tool to inclusively reconstruct events in Belle II 
using the information on the final state particles only, without any prior assumption about the nature of the underlying event.
This task is achieved with the use of deep Graph Neural Networks (GNNs), a particular class of neural networks acting on *graphs*. 
Graphs are entities composed of a set of *nodes* :math:`V=\{v_{i}\}_{i=1}^N`` 
connected by *edges* :math:`E = \{e_{v_{i} v_{j}} \equiv e_{ij}\}_{i \neq j}`.

.. seealso::
   The model is described in `these proceedings <https://indico.cern.ch/event/1106990/papers/4996235/files/12252-ACAT_2022_proceedings.pdf>`_.
   This work is based on 
   `'Learning tree structures from leaves for particle decay reconstruction' <https://iopscience.iop.org/article/10.1088/2632-2153/ac8de0>`_ 
   by Kahn et al. Please consider citing both papers. 
   The code is adapted from the work of Kahn et al and is available `here <https://github.com/Helmholtz-AI-Energy/BaumBauen>`_.
   A detailed description of the model is also available in this `Belle II internal note <https://docs.belle2.org/record/3649>`_.

The network is trained to predict the mass hypotheses of final state particles and the **Lowest Common Ancestor** (LCA) matrix of the event.
Each element of this matrix corresponds to a pair of final state particles, and contains the lowest ancestor common to both particles. 
To avoid the use of a unique identifier for each ancestor, a system of classes is used: 
6 for :math:`\Upsilon (4S)` resonances, 5 for :math:`B^{\pm,0}` mesons, 4 for :math:`{D^{*}_{(s)}}^{\pm, 0}`, 3 for :math:`D^{\pm,0}`, 
2 for :math:`K_{s}^{0}`, 1 for :math:`\pi^{0}`` or :math:`J/\psi` and 0 for particles not belonging to the decay tree. 
This new representation of the LCA is called LCAS matrix, where the S stands for "stage".

The model can be trained and evaluated in two different modes:

* :math:`\Upsilon (4S)` reconstruction mode: the model is trained to reconstruct the LCAS matrix of the whole event, i.e. the maximum depth of
  the LCAS matrix is 6;
* :math:`B` reconstruction mode: the model is trained to reconstruct single :math:`B` decays, i.e. the maximum depth of the LCAS matrix is 5. 
  In this case, when applying the model to some data, a signal-side must be reconstructed first, and the graFEI is used to reconstruct the rest-of-event.


Model training
##############

The model is trained with ``ROOT`` ntuples produced with the steering file ``analysis/scripts/grafei/scripts/CreateTrainingFiles.py``.
The file requires the argument ``-t`` to be set to either ``B+``, ``B0`` or ``Ups`` for :math:`B^{+}`, :math:`B^{0}` or :math:`\Upsilon (4S)` reconstruction respectively.
The output files used for training and evaluation must be placed in the folders ``root/train`` and ``root/val`` respectively, where ``root`` is a folder of your choice.

The training is performed with the python script ``analysis/scripts/grafei/scripts/train_model.py``. It requires a ``yaml`` config file to be passed to the script with the ``-c`` argument.
You can find a default config file at ``analysis/scripts/grafei/config.yaml``, where all the options are documented.
The training will output a copy of the config file used and a weight file in the format ``.pt`` that can be used to apply the model to some other data.
The output folder is defined in the config file.


Code documentation
##################

This section describes the grafei code, contained in ``analysis/scripts/grafei``.

Basf2 modules
*************

Here the modules needed to run the graFEI in a basf2 are described. 
You can import these modules in a steering file with ``from grafei import MODULENAME``.

.. automodule:: grafei
   :members:


Other modules and functions
***************************

Here the core code of the graFEI is described. This section is intended for developers, 
users usually do not need to manipulate these classes and functions.

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

