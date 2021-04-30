.. _svdsimulation:

SVD Simulation
==================

The SVD simulation starts with ``SVDSimHits`` and ends with ``SVDShaperDigits`` and ``SVDEventInfo``.

Please use the following python function if you want to add the SVD simulation to your steering file:

.. autofunction:: svd.__init__.add_svd_simulation


Simulation Modules
------------------

This is a list of the ``svd`` modules used for simulation. 

.. b2-modules::
   :package: svd
   :modules: SVDEventInfoSetter, SVDDigitizer
   :io-plots:
