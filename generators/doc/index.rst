B65;6800;1c.. _event_generators:

================
Event Generators
================

This package contains all code related to physics generators. Please have a
look at `Overview of the Belle II Physics Generators`_. As a general rule users
are strongly encouraged to use the functions provided by the `generators`
python module and not manually add generator modules directly to the path.

.. _Overview of the Belle II Physics Generators: https://docs.belle2.org/record/282?ln=en


The generators Python Module
----------------------------

.. automodule:: generators
   :members:


Modules in the Generators Package
---------------------------------

Apart from the default settings above the generators can also be setup directly
and there are special generators not covered above.

.. warning:: Please use the function from the generators module to setup event generation if possible.

.. b2-modules::
   :package: generators


Selective background simulation using graph neural networks
-----------------------------------------------------------

Using graph neural network with attention mechanism to predict whether a generated event will
pass the skim after detector simulation and reconstruction. Selection and weighting methods are
then invested to choose proper events according to their scores while trying to avoid bias.
The computational resource used for steps between generation and skim will be saved in this way.

Currently only the practical part is available e.g. the well trained neural network
(parameters saved in global tag ``SmartBKG_GATGAP`` with payload ``GATGAPgen.pth``)
to filter out events that can pass FEI hadronic B0 skim. The corresponding neural network
built with PyTorch is stored in ``generators/smartBKG/model/gatgap.py`` while
``generators/smartBKG/NN_filter_module.py`` is a wrapper (``basf2.module``) suited in basf2
framework.

.. autoclass:: smartBKG.NN_filter_module.NNFilterModule


Validation
----------

.. toctree::
   :glob:

   validation
