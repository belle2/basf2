.. _ksselectorwrappers:

ksSelector
==========

The ksSelector is a module for MVA-based :math:`K^0_S` selection.
This KsSelector consists of two selection through MVA, V0Selector and LambdaVeto. The first is a MVA to discriminate :math:`K^0_S`
from mis-reconstructed :math:`K^0_S` coming from IP. The second is a MVA to discriminate :math:`K^0_S` and :math:`\Lambda`.
Parameters for MVA training are selected not to raise bias in mass of :math:`K^0_S` for true :math:`K^0_S`.

V0Selector
----------

For V0Selector, mainly kinematics of :math:`K^0_S` and its daughter pions, flight length of :math:`K^0_S` and number of hits in VXD
are used for MVA training. The most dominant parameter for V0 selection is cosVertexMomentum, which represents the angle between
vertex position and momentum of :math:`K^0_S`.

LambdaVeto
----------

For LambdaVeto, daughters' protonID, Lambda_mass which is calculated assuming proton mass hypothesis for one of daughter pions,
and kinematics of pions are used for MVA training. Two Lambda_mass parameters are dominant for LambdaVeto selection.

Tutorials
---------

An example tutorial for applying ksSelector to the existing :math:`K^0_S` particleList is :

::

  analysis/examples/tutorials/B2A910-ApplyKsSelector.py

An example tutorial for creating dataset for MVA training, which include true :math:`K^0_S`, :math:`\Lambda`, and mis-reconstructed :math:`K^0_S` is :

::

  analysis/examples/tutorials/B2A911-CreateTrainData.py

An example tutorial for training dataset is :

::

  analysis/examples/tutorials/B2A912-TrainKsSelector.py

ksSelector Functions
--------------------

.. automodule:: ksSelector
   :members:
   :undoc-members:
