.. _ksselectorwrappers:

ksSelector
======================

The ksSelector is a module for MVA-based :math:`K^0_S` selection.
This KsSelector consists of two selection through MVA. The first is a MVA to discriminate :math:`K^0_S`
and mis-reconstructed :math:`K^0_S` coming from IP. The second is a MVA to discriminate :math:`K^0_S` and :math:`\Lambda`.

Tutorials
----------

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
