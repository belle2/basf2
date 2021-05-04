.. _svdreconstruction:

SVD Reconstruction
==================

The SVD reconstruction starts with ``SVDShaperDigits`` and ``SVDEventInfo``, ``SVDClusters`` are created on both sides of the sensors and then combined into ``SVDSpacePoints``.

Clustering
----------
The first step of reconstruction after unpacking (or simulation) is the the clustering, i.e. grouping adjacent strips into ``RawCluster``. A strip is considered for clustering its SNR > 3, which corresponds to the online zero-suppression cut: if a strip has a SNR <=3 it's not written out. Consequently all acquired strips are good for clustering.
A RawCluster is promoted to ``SVDCluster`` if there is at least one strip in the cluster with SNR > 5.

Cluster Charge Reconstruction
-----------------------------
The cluster charge is a measurement of the charge released in the sensor by the charged particle, and collected on one side of the sensor.

The *default algorithm* used to compute cluster charge is the ``MaxSample``: the highest strip sample for each strip in the cluster represents the strip charge (in ADC). This value is calibrated using the ``SVDPulseShapeCalibrations`` DBObject and converted in :math:`e^{-}` and the cluster charge is evaluated as the sum of the strips charges in :math:`e^{-}`.


We have two alternative algorithms to compute the cluster charge:

#. ``SumSamples``: the strip charge is evaluated as the sum of the 6 samples in ADC, converted in :math:`e^{-}` and then the cluster charge is the computed as the sum of the strip charges.

#. ``ELS3``: first all strips in the cluster are summed sample by sample. Then the 3 best consecutive summed-samples are found and the maximum :math:`A` of :math:`a(t) = \frac{A(t-t_{\rm raw}}{\tau}\exp{\left(1 - \frac{t-t_{\rm raw}}{\tau}\right)}` (CR-RC waveform) is computed with a simple system of equations (with :math:`\tau = 55\ \rm ns`) and converted in :math:`e^{-}`.
 
.. note::

   All three algorithms are implemented in the ``svd/reconstruction/SVDClusterCharge`` class.

Cluster Time Reconstruction
---------------------------
The cluster time is a measurement of the time of the hit (with respect to the trigger signal).

The *default algorithm* used to compute cluster time is the ``CoG6``: the cluster time is the average of the strips time weighted with the strip charge. The raw strip time is computed as the average of the sample time weighted with the sample amplitude:

.. math::

   t_{strip}^{\rm raw} = \frac{\sum_{i=0}^{i<6}t_i A_i}{\sum_{i=0}^{i<6} A_i}.

The raw strip time is calibrated with a third order polynomial stored in the ``SVDCoGTimeCalibration`` payload.


We have two alternative algorithms to compute the cluster time. For both, first all strips in the cluster are summed sample by sample, tthen the 3 best consecutive summed-samples are determined.

#. ``CoG3``: the raw cluster time is the average of the 3 best samples time with the sample charge. The raw time is finally calibrated with a third order polynomial stored in ``SVDCoG3SamplesCalibrations``.

#. ``ELS3``: the raw cluster time is computed with a symple system of equations as :math:`t_{\rm raw}` of :math:`a(t) = \frac{A(t-t_{\rm raw}}{\tau}\exp{\left(1 - \frac{t-t_{\rm raw}}{\tau}\right)}` (CR-RC waveform). The raw time is finally calibrated with a third order polynomial stored in ``SVDELS3SamplesCalibrations``.
 
.. note::

   All three algorithms are implemented in the ``svd/reconstruction/SVDClusterTime`` class.


Cluster Position Reconstruction
-------------------------------

SpacePoint Creation
-------------------

Strip Reconstruction (Optional)
-------------------------------
Please use the following python function if you want to add the SVD reconstruction to your steering file:

.. autofunction:: svd.__init__.add_svd_reconstruction

Reconstruction Modules
----------------------

This is a list of the ``svd`` modules used for reconstruction. 

.. b2-modules::
   :package: svd
   :modules: SVDClusterizer, SVDMissingAPVsClusterCreator, SVDSpacePointCreator, SVDRecoDigitCreator
   :io-plots:

