.. _svdreconstruction:

SVD Reconstruction
==================

The SVD reconstruction starts with :ref:`SVDShaperDigits<svdshapers>` and :ref:`SVDEventInfo<svdeventinfo>`, :ref:`SVDClusters<svdclusters>` are created on both sides of the sensors and then combined into :ref:`SVDSpacePoints<svdsps>`.

Several algorithms are available for cluster charge, time and position reconstruction, and they are specified as parameters of the :b2:mod:`SVDClusterizer` module (``chargeAlgorithm{3/6}Samples``, ``timeAlgorithm{3/6}Samples``, ``positionAlgorithm{3/6}Samples``).

When we reconstruct simulated events, fudge factors are applied to the cluster time and position in order to
reproduce the resolution that we measure in data, see :ref:`SVDMCClusterPositionFudge<svdfudgeposition>` and :ref:`SVDMCClusterTimeFudge<svdfudgetime>`.

Use the following python function if you want to add the SVD reconstruction to your steering file:

.. autofunction:: svd.__init__.add_svd_reconstruction


Clustering
----------
The first step of reconstruction after unpacking (or simulation) is the clustering, i.e. grouping adjacent strips into ``RawCluster``. All acquired strips are good for clustering since the minimum value of SNR corresponds to the online zero-suppression cut: SNR > 3.
A ``RawCluster`` is promoted to ``SVDCluster`` if there is at least one strip in the cluster with SNR > 5.
The parameters for clustering are stored in the :ref:`SVDClustering<svdclustercuts>` DBObject.

Cluster Charge Reconstruction
-----------------------------
The cluster charge is a measurement of the charge released in the sensor by the charged particle, and collected on one side of the sensor.

The **default** algorithm used to compute cluster charge is the ``MaxSample``: the highest strip sample for each strip in the cluster represents the strip charge (in ADC). This value is converted in :math:`e^{-}` using the :ref:`SVDPulseShapeCalibrations<svdpulsecal>` DBObject. The cluster charge is computed as the sum of the strips charges in :math:`e^{-}`.


We have two alternative algorithms to compute the cluster charge that can be selected by setting the :b2:mod:`SVDClusterizer` parameter ``chargeAlgorithm{3/6}Samples``:

#. ``SumSamples``: the strip charge is evaluated as the sum of the 6 samples in ADC, converted in :math:`e^{-}` with the help of the :ref:`SVDPulseShapeCalibrations<svdpulsecal>` DBObject and then the cluster charge is the computed as the sum of the strip charges.

#. ``ELS3``: first, all strips in the cluster are summed sample by sample. Then, the 3 best consecutive summed-samples are found with the `MaxSum`_ algorithm and the maximum :math:`A` of the theoretical CR-RC waveform (with :math:`\tau = 55\ \rm ns`):

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

.. _svdff:

#. find the two consecutive samples for which :math:`(a_{i} + a_{i+1})` represents the maximum, where :math:`a_{j}` is the amplitude of j-th sample.
#. the three chosen samples are: :math:`a_{i-1}, a_{i},  a_{i+1}`, with :math:`i-1 = FF`, the ``FirstFrame``.
#. in case in which :math:`i = 0`, the three chosen samples are :math:`a_0 , a_1 , a_2` with :math:`FF=0`.


Cluster Time Reconstruction
---------------------------
The cluster time is a measurement of the time of the hit (with respect to the trigger signal).

The **default** algorithm used to compute cluster time is the ``CoG3``: first, all strips in the cluster are summed sample by sample, and the 3 best consecutive summed-samples are determined using the `MaxSum`_ algorithm. Then, the raw cluster time is the average of the 3 best summed-samples time with the sample charge:

.. math::

   t_{\rm cluster}^{\rm raw} = \Delta t \cdot \frac{\sum_{i=0}^{i<3}i\ a_i}{\sum_{i=0}^{i<3} a_i}.

where the :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip and  :math:`a_{j}` is the amplitude of j-th summed-sample.


The raw time is finally calibrated with a third order polynomial stored in the :ref:`SVDCoG3SampleTimeCalibration<svdcog3timecal>` DBObject, see :ref:`svdtimecalib` for more details on the calibration.

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

Shifting of SVD-cluster-time based on Cluster-size:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Mean of cluster-times shifts with size of the clusters as strips with lower amplitude delays the time. This is not simulated and only should be seen in Data. To compensate this, a shift is added to the calibrated time. The values are stored in :ref:`SVDClusterTimeShifter<svdclustertimeshifter>` DBObject.


Cluster Position Reconstruction
-------------------------------
The algorithm to determine the cluster position depends on the cluster size, i.e. the number of strips forming the cluster.

For one-strip clusters the position is the position of the strip, i.e. the position of the center of the readout implant.

For more-than-one strip clusters we have two algorithms:

1) center-of-gravity ``CoG``, the cluster position is computed averaging the strip position weighted with the strip charge:

  .. math::

     x_{\rm cluster} =  \frac{\sum_{\rm strips} x_i S_i}{\sum_{\rm strips} S_i}.

  where :math:`x_i` is the strip position and :math:`S_i` is the strip charge.

2) analog-head-tail ``AHT``:

  .. math::

     x_{\rm cluster} = \frac{1}{2} \left[x_{\rm head} + x_{\rm tail} + p  \frac{S_{\rm head} - S_{\rm tail}}{S_{\rm center}} \right].

  where :math:`p` is the readout pitch,  :math:`x_{\rm head/tail}` (:math:`S_{\rm head/tail}`) are the position (charge) of the two strips at the edge of the cluster and :math:`S_{\rm center}` is the average strip charge after removing the strips at the edge, :math:`S_{\rm center} = \frac{S_{\rm tot} - S_{\rm head} - S_{\rm tail}}{\rm size - 2}` with :math:`S_{\rm tot}` is the total cluster charge.

The available algorithms to determine the strip charge for the position computation are the same available for clusters, strips are considered as one-strip clusters. To choose the strip charge reconstruction algorithm for cluster position computation use the :b2:mod:`SVDClusterizer` parameter ``stripChargeAlgorithm{3/6}Samples``. The **default** algorithm is the ``MaxSample``.


In the current **default** reconstruction the ``CoG`` is used for cluster size > 1 (``AHT`` is not used).
Indeed, the ``SVDClusterizer`` supports the following position reconstruction algorithms (that can be passed as string, see the :b2:mod:`SVDClusterizer` parameter ``positionAlgorithm{3/6}Samples`` parameter)

1) ``CoGOnly`` (**current default**): the ``CoG`` is used for all cluster sizes :math:`\ge2` (``AHT`` is not used), error scale factors (= 1 for data and MC) are stored in :ref:`SVDCoGOnlyErrorScaleFactors<svdcogonlycal>`.

2) ``OldDefault``: ``CoG`` for cluster size = 2 and ``AHT`` for cluster sizes > 2 (error scale factors stored in :ref:`SVDOldDefaultErrorScaleFactors<svdolddefaultcal>`

.. note::

   For ``CoGOnly``, position errors are stored  :ref:`SVDCoGOnlyPositionError<svdcogonlyerr>`.


.. seealso::

   All algorithms are implemented in the ``svd/reconstruction/SVDClusterPosition`` class.

Creation of Clusters in disabled-APV regions
--------------------------------------------

In case one or more APV readout chips are disabled during data taking, a *fake* cluster is created in the middle of the region in order not to loose the information of the hit on the other side of the sensor. For more details see :b2:mod:`SVDMissingAPVsClusterCreator`.

SVD Cluster Grouping
--------------------
Clusters are grouped event-by event based on their time distribution. Clusters created by charged particles coming to the same e+e- collision are near in time, and will be grouped together.

Method in :b2:mod:`SVDTimeGrouping` module:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Steps                          | Description                                                                                                                                                                                                                                                                                                                                                                     |
+================================+=================================================================================================================================================================================================================================================================================================================================================================================+
| Preparing Histogram            | Creating a 1D histogram of given range **[-160, 160]** ns and bin-width of **1/2** ns. (default **tRangeLow=-160, tRangeHigh=160, rebinningFactor=2**)                                                                                                                                                                                                                          |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Filling Cluster Time           | For each cluster, a normalized Gaussian is filled in the above histogram, centered at the cluster-time. The width of the Gaussian is parsed from the hard-coded values in the header depending on the side of the sensor and the number of strips in the cluster. To save time, only the bins within 3 sigma of the center is calculated and filled. (default **fillSigmaN=3**) |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Finding Peaks                  | Once all the Gaussian entries are filled, the correlated-clusters form a big Gaussian (which will be called **group** hereafter). The group-finding is performed in the following method.                                                                                                                                                                                       |
|                                |                                                                                                                                                                                                                                                                                                                                                                                 |
|                                | * Maximum bin is found and a Gaussian is fitted in the range of **[-5, 5]** ns keeping the bin as center. (default **fitRangeHalfWidth=7**)                                                                                                                                                                                                                                     |
|                                | * The group info (integral, center, width) is stored.                                                                                                                                                                                                                                                                                                                           |
|                                | * The fitted Gaussian is then subtracted from the histogram to find the next significant peak/group. To save time, value of the Gaussian for the bins only within 5 sigma are calculated and subtracted from those bins. (default **removeSigmaN=7**)                                                                                                                           |
|                                | * Maximum bin is found again and checked whether this peak is above threshold ( > 0.05 x firstGroup). (default **fracThreshold=0.05**)                                                                                                                                                                                                                                          |
|                                | * If above threshold, then the process is repeated again.                                                                                                                                                                                                                                                                                                                       |
|                                | * Search is stopped if 20 groups are found. (default **maxGroups=20**)                                                                                                                                                                                                                                                                                                          |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Creating Dummy Groups          | If the number of groups found is less than **maxGroups** then, dummy groups are inserted to increase the total groups to **maxGroups**. This helps in the sorting process described in the following.                                                                                                                                                                           |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Sorting the Groups             | Naturally, the groups are sorted by their prominence. But in an event, background might be higher than signal, causing the background peak occupying the first position. So there is a need for sorting the groups. It is done in two stages:                                                                                                                                   |
|                                |                                                                                                                                                                                                                                                                                                                                                                                 |
|                                | * Background:                                                                                                                                                                                                                                                                                                                                                                   |
|                                |                                                                                                                                                                                                                                                                                                                                                                                 |
|                                |     * All the groups with centers outside the signal range **[-50,50]**, are moved towards the end of the queue. (default **expectedSignalTimeMin=-50, expectedSignalTimeMax=50**)                                                                                                                                                                                              |
|                                |     * These groups are then sorted in reverse, that means, the probability of being background is max at groupID = 19(or max) and decreases with group number decreasing.                                                                                                                                                                                                       |
|                                |                                                                                                                                                                                                                                                                                                                                                                                 |
|                                | * Signal:                                                                                                                                                                                                                                                                                                                                                                       |
|                                |                                                                                                                                                                                                                                                                                                                                                                                 |
|                                |     * Signal groups are naturally sorted based on integral at this point. But sometimes a background group, near signal may be higher than the signal group. Now the probability of finding a signal group near 0 is greater. So the signal groups are sorted with an exponential weight with width 30 ns. (default **expectedSignalTimeCenter=0, signalLifetime=30**)          |
|                                |     * So, the probability of being signal is max at groupID = 0 and decreases with group number increasing.                                                                                                                                                                                                                                                                     |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Signal Group Selection         | Depending on the type of study, we may choose the first group to be accepted for space-point-creation. (default **numberOfSignalGroups=1**)                                                                                                                                                                                                                                     |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **formSingleSignalGroup**      | If this flag is enabled, all the groups are given same groupId = 0.                                                                                                                                                                                                                                                                                                             |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Assign GroupId to SVD-Clusters | All the clusters are now compared against the surviving groups. The clusters within 5 sigma of a group center are then assigned the groupId and groupInfo. Hence, one cluster shared between two groups can have two groupId. (default **acceptSigmaN=7**)                                                                                                                      |
+--------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Payloads related to SVDTimeGrouping:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#. :ref:`SVDRecoConfiguration<svdrecoconfiguration>` : The switiching ON/OFF of SVDTimeGrouping is controlled by this DBObject.
#. :ref:`SVDTimeGroupingConfiguration<svdtimegroupingconfiguration>` : All the parameters used by this module is stored in this DBObject.


:ref:`SpacePoint<svdsps>` Creation
----------------------------------

All clusters on one side of each sensor are combined with all clusters on the other side. Certain combinations of clusters can be excluded based on the hit time, the two available cuts are:

#. exclude ``SpacePoints`` in which at least one cluster has hit time below a certain threshold:

   .. math::

      t_{u/v} > t_{\rm min}

#. exclude ``SpacePoints`` in which at least one cluster has hit time far in time w.r.t. trigger (t=0) above a certain threshold:

   .. math::

      |t_{u/v}| < t_{\rm max}

#. exclude ``SpacePoints`` in which the time difference of the two clusters exceeds a certain threshold:

   .. math::

      |t_u - t_v| < \Delta t_{\rm max}

#. exclude ``SpacePoints`` formed by clusters belonging to different SVD groups.

The choice of the cut and of the threshold is stored in the :ref:`SVDHitTimeSelection<svdhittimeselection>` for what concerns 1,2,3; while to enable the selection based on grouping, the module parameter ``useSVDGroupInfo`` should be used.

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
   :modules: SVDUnpacker, SVDClusterizer, SVDTimeGrouping, SVDMissingAPVsClusterCreator, SVDSpacePointCreator, SVDRecoDigitCreator
   :io-plots:

