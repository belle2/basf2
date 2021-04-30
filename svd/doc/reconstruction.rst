.. _svdreconstruction:

SVD Reconstruction
==================

The SVD reconstruction starts with ``SVDShaperDigits`` and ``SVDEventInfo`` and ends with ``SVDSpacePoints``.

Please use the following python function if you want to add the SVD reconstruction to your steering file:

.. autofunction:: svd.__init__.add_svd_reconstruction

Reconstruction Modules
----------------------

This is a list of the ``svd`` modules used for reconstruction. 

.. b2-modules::
   :package: svd
   :modules: SVDClusterizer, SVDMissingAPVsClusterCreator, SVDSpacePointCreator, SVDRecoDigitCreator
   :io-plots:

