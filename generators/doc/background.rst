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
