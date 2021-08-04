.. _svdreconstruction:

SVD Reconstruction
==================

The SVD reconstruction starts with :ref:`SVDShaperDigits<svdshapers>` and :ref:`SVDEventInfo<svdeventinfo>`, :ref:`SVDClusters<svdclusters>` are created on both sides of the sensors and then combined into :ref:`SVDSpacePoints<svdsps>`.

Several algorithms are available for cluster charge, time and position reconstruction, and they are specified as parameters of the :b2:mod:`SVDClusterizer` module (``chargeAlgorithm{3/6}Samples``, ``timeAlgorithm{3/6}Samples``, ``positionAlgorithm{3/6}Samples``).

Use the following python function if you want to add the SVD reconstruction to your steering file:

.. autofunction:: svd.__init__.add_svd_reconstruction


Clustering
----------
The first step of reconstruction after unpacking (or simulation) is the clustering, i.e. grouping adjacent strips into ``RawCluster``. All acquired strips are good for clustering since the minimum value of SNR corresponds to the online zero-suppression cut: SNR > 3.
A ``RawCluster`` is promoted to ``SVDCluster`` if there is at least one strip in the cluster with SNR > 5.
The parameters for clustering are stored in the :ref:`SVDClusterCuts<svdclustercuts>` DBObject.

Cluster Charge Reconstruction
-----------------------------
The cluster charge is a measurement of the charge released in the sensor by the charged particle, and collected on one side of the sensor.

The **default** algorithm used to compute cluster charge is the ``MaxSample``: the highest strip sample for each strip in the cluster represents the strip charge (in ADC). This value is converted in :math:`e^{-}` using the :ref:`SVDPulseShapeCalibrations<svdpulsecal>` DBObject. The cluster charge is computed as the sum of the strips charges in :math:`e^{-}`.


We have two alternative algorithms to compute the cluster charge that can be selected by setting the :b2:mod:`SVDClusterizer` parameter ``chargeAlgorithm{3/6}Samples``:

#. ``SumSamples``: the strip charge is evaluated as the sum of the 6 samples in ADC, converted in :math:`e^{-}` with the help of the :ref:`SVDPulseShapeCalibrations<svdpulsecal>` DBObject and then the cluster charge is the computed as the sum of the strip charges.

#. ``ELS3``: first all strips in the cluster are summed sample by sample. Then the 3 best consecutive summed-samples are found with the `MaxSum`_ algorithm and the maximum :math:`A` of the theoretical CR-RC waveform (with :math:`\tau = 55\ \rm ns`):

   .. _svdcrrc:

   .. math::

      a(t) = A \frac{t-t_{\rm raw}}{\tau}\exp{\left(1 - \frac{t-t_{\rm raw}}{\tau}\right)} 

   is computed with a simple system of equations:

   .. math::

      A = \frac{(e_{\tau}^{-1} - e_{\tau}^{3}) a_{1} + (2+e_{\tau}^{2})a_{2} - (1+2e_{\tau}^{2})a_{0}}{\Delta t/\tau \exp{(1+t_{\rm raw}/\tau)}(1+4e_{\tau}^{2}+e_{\tau}^{4})}

   where :math:`e_{\tau} = e^{- \frac{\Delta t}{\tau}}`, :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip,  :math:`t_{\rm raw}` is the :ref:`raw ELS3 time<svdels3time>`,  and  :math:`a_{j}` is the amplitude of j-th summed-sample.

   Finally the charge is converted in :math:`e^{-}` with the the :ref:`SVDPulseShapeCalibrations<svdpulsecal>` DBObject.
 
.. seealso::

   All three algorithms are implemented in the ``svd/reconstruction/SVDClusterCharge`` class.

.. _MaxSum: 

**The MaxSum Algorithm**:

#. Find the two consecutive samples for which :math:`(a_{i} + a_{i+1})` represents the maximum, where :math:`a_{j}` is the amplitude of j-th sample.

.. _svdff:

#. The three chosen samples are: :math:`a_{i-1}, a_{i},  a_{i+1}`, with :math:`i-1 = FF`, the ``FirstFrame``.
#. There will be cases in which :math:`i = 0`, in this case the three chosen samples are :math:`a_0 , a_1 , a_2` with :math:`FF=0`.


Cluster Time Reconstruction
---------------------------
The cluster time is a measurement of the time of the hit (with respect to the trigger signal).

The **default** algorithm used to compute cluster time is the ``CoG3``: first all strips in the cluster are summed sample by sample, and the 3 best consecutive summed-samples are determined using the `MaxSum`_ algorithm. Then, the raw cluster time is the average of the 3 best summed-samples time with the sample charge:

.. math::

   t_{\rm cluster}^{\rm raw} = \Delta t \cdot \frac{\sum_{i=0}^{i<3}i\ a_i}{\sum_{i=0}^{i<3} a_i}.

where the :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip and  :math:`a_{j}` is the amplitude of j-th summed-sample.


The raw time is finally calibrated with a third order polynomial stored in the :ref:`SVDCoG3SampleTimeCalibration<svdcog3timecal>` DBObject.

We have two alternative algorithms to compute the cluster time that can be selected by setting the :b2:mod:`SVDClusterizer` parameter ``timeAlgorithm{3/6}Samples``. 

#. ``CoG6``: the cluster time is the average of the strips time weighted with the strip charge. The raw strip time is computed as the average of the sample time weighted with the sample amplitude:

   .. math::

      t_{\rm strip}^{\rm raw} = \Delta t \cdot \frac{\sum_{i=0}^{i<6}i\ a_i}{\sum_{i=0}^{i<6} a_i}.

   where the :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip and  :math:`a_{j}` is the amplitude of j-th sample.
   The raw strip time is calibrated with a third order polynomial stored in the :ref:`SVDCoGTimeCalibration<svdcog6timecal>` DBObject.


#. ``ELS3``: as for ``CoG3``, all strips in the cluster are summed sample by sample and the `MaxSum`_ algorithm is applied, and then the raw cluster time is computed with a simple system of equations as :math:`t_{\rm raw}` of the theoretical :ref:`CR-RC waveform<svdcrrc>`:
   
   .. _svdels3time:

   .. math::

      t_{\rm cluster}^{\rm raw} = - \Delta t \cdot \frac{2e_{\tau}^{4} + w e_{\tau}^{2}}{1 - e_{\tau}^{4}-w(2+e_{\tau}^{2})}

   where :math:`e_{\tau} = e^{- \frac{\Delta t}{\tau}}`, :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip,  :math:`w = \frac{a_{0}-2 e_{\tau}^{-2} a_{2}}{2a_{0} + e_{\tau}^{-1} a_{1}}` and  :math:`a_{j}` is the amplitude of j-th summed-sample.

   The raw time is finally calibrated with a third order polynomial stored in the  :ref:`SVDELS3SampleTimeCalibration<svdels3timecal>` DBObject.
 
.. seealso::

   All three algorithms are implemented in the ``svd/reconstruction/SVDClusterTime`` class.

.. note::

   The hit time expressed in the trigger reference :math:`t_{\rm hit}` is:

   .. math::

      t_{\rm hit} = f(t_{\rm raw}) + r_{\rm shift} + \frac{\Delta t}{4} \cdot (3 - TB + 4\ FF)
   
   where :math:`f(t_{\rm raw})` is the calibrated time, :math:`r_{\rm shift}` is the relative shift among 3-sample and 6-sample event (= 0 in 6-sample events), :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip, :math:`TB` is the :ref:`TriggerBin<svdtb>` and :math:`FF` is the :ref:`FirstFrame<svdff>`.
   

Cluster Position Reconstruction
-------------------------------
The algorithm to determine the cluster position depends on the cluster size, i.e. the number of strips forming the cluster.

For one-strip clusters the position is the position of the strip, i.e. the position of the center of the readout implant.

For more-than-one strip clusters we have two algorithms:

* center-of-gravity ``CoG``, the cluster position is computed averaging the strip position weighted with the strip charge:

  .. math::

     x_{\rm cluster} =  \frac{\sum_{\rm strips} x_i S_i}{\sum_{\rm strips} S_i}.

  where :math:`x_i` is the strip position and :math:`S_i` is the strip charge.

* analog-head-tail ``AHT``:

  .. math::

     x_{\rm cluster} = \frac{1}{2} \left[x_{\rm head} + x_{\rm tail} + p  \frac{S_{\rm head} - S_{\rm tail}}{S_{\rm center}} \right].

  where :math:`p` is the readout pitch,  :math:`x_{\rm head/tail}` (:math:`S_{\rm head/tail}`) are the position (charge) of the two strips at the edge of the cluster and :math:`S_{\rm center}` is the average strip charge after removing the strips at the edge, :math:`S_{\rm center} = \frac{S_{\rm tot} - S_{\rm head} - S_{\rm tail}}{\rm size - 2}` with :math:`S_{\rm tot}` is the total cluster charge.

The available algorithms to determine the strip charge for the position computation are the same available for clusters, strips are considered as one-strip clusters. To choose the strip charge reconstruction algorithm for cluster position computation use the :b2:mod:`SVDClusterizer` parameter ``stripChargeAlgorithm{3/6}Samples``. The **default** algorithm is the ``MaxSample``.


In the current **default** reconstruction the ``CoG`` is used for cluster size > 1 (``AHT`` is not used).

.. note::

   Position errors are stored  :ref:`SVDCoGOnlyPositionError<svdcogonlyerr>`, while scaling factors are stored in :ref:`SVDCoGOnlyErrorScaleFactors<svdcogonlycal>`.

The ``SVDClusterizer`` supports the following position reconstruction algorithms (that can be passed as string, see the :b2:mod:`SVDClusterizer` parameter ``positionAlgorithm{3/6}Samples`` parameter)

* ``OldDefault``: ``CoG`` for cluster size = 2 and ``AHT`` for cluster sizes > 2 (scale factors stores in :ref:`SVDOldDefaultErrorScaleFactors<svdolddefaultcal>`
* ``CoGOnly`` (**current default**): the ``CoG`` is used for all cluster sizes :math:`\ge2` (``AHT`` is not used), scale factors stored in :ref:`SVDCoGOnlyErrorScaleFactors<svdcogonlycal>`.

.. seealso::

   All algorithms are implemented in the ``svd/reconstruction/SVDClusterPosition`` class.

Creation of Clusters in disabled-APV regions
--------------------------------------------

In case one or more APV readout chips are disabled during data taking, a *fake* cluster is created in the middle of the region in order not to loose the information of the hit on the other side of the sensor. For more details see :b2:mod:`SVDMissingAPVsClusterCreator`.

:ref:`SpacePoint<svdsps>` Creation
----------------------------------

All clusters on one side of each sensor are combined with all clusters on the other side. Certain combinations of clusters can be excluded based on the hit time, the two available cuts are:

#. exclude ``SpacePoints`` in which at least one cluster has hit time below a certain threshold
#. exclude ``SpacePoints`` in which the time difference of the two clusters exceeds a certain threshold

The choice of the cut and of the threshold is stored in the :ref:`SVDHitTimeSelection<svdhittimeselection>`.

SpacePoints are not created if they exceed a certain threshold defined in the ``numMaxSpacePoints`` parameter of the :b2:mod:`SVDSpacePointCreator`

Strip Reconstruction (Optional)
-------------------------------

The :b2:mod:`SVDRecoDigitCreator` reconstructs raw strips, creating one :ref:`SVDRecoDigit<svdrecos>` for each :ref:`SVDShaperDigit<svdshapers>`.

Strip reconstruction is not called in the default SVD reconstruction. It is anyway necessary for DQM or other performance studies.
Use the following python function if you want to add the SVD strip reconstruction to your steering file:

.. autofunction:: svd.__init__.add_svd_create_recodigits

The charge and time algorithm available for clusters are also available for strips (considered as one-strip clusters). To choose the algorithm use the :b2:mod:`SVDRecoDigitCreator` parameter ``chargeAlgorithm{3/6}Samples``, ``timeAlgorithm{3/6}Samples``.



Reconstruction Modules
----------------------

This is a list of the ``svd`` modules used for reconstruction. 

.. b2-modules::
   :package: svd
   :modules: SVDUnpacker, SVDClusterizer, SVDMissingAPVsClusterCreator, SVDSpacePointCreator, SVDRecoDigitCreator
   :io-plots:

