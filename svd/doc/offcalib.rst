.. _svdoffcalib:

SVD Offline Calibration
=======================

Here we briefly describe the so-called offline calibrations.

.. _svdtimecalib:

Hit Time Calibration
---------------------

The time calibration is implemented in the :ref:`calibration framework<cal_caf>` and is run on **6-sample data** with AirFlow.

The hit time calibration exploits the correlation between the time of the event ``EventT0`` and the hit time. We select clusters associated to tracks and neglect the flight time of the particle.

For all three time estimators ``CoG6``, ``CoG3`` and ``ELS3`` we first compute the time of the event in the SVD reference frame, :math:`t_0^{\rm SVD}`:

.. math::

   t_0^{\rm SVD} = t_0 - \frac{\Delta t}{4} \cdot (3 - TB + 4\ FF)

where :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip, :math:`TB = 0,1,2,3` is the :ref:`TriggerBin<svdtb>` and provides the correct time shift to move into the SVD reference frame, and :math:`FF=0,1,2,3` (:ref:`FirstFrame<svdff>`) is applied only for ``CoG3`` and ``ELS3`` due to the :ref:`MaxSum<MaxSum>` algorithm.

We build the 2D plot of the raw hit time :math:`t_{\rm raw}` vs :math:`t_0^{\rm SVD}`. Note that the :math:`t_{\rm raw}` is different for the three estimator.
We then take the ``ProfileX`` of the 2D plot and fit the correlation with a function that depends on the estimator. An example of correlation plot for CoG6, with calibration function superimposed is shown in the following figure:

.. figure:: figures/time_calibration.png
   :align: center

**CoG6 Calibration Function**

For ``CoG6`` we use a third order polynomial:

.. math::

   t_{\rm CoG6} = a + b t_{\rm raw} + c t_{\rm raw}^2 + d t_{\rm raw}^3

**CoG3 Calibration Function**

For ``CoG3`` we use a third order polynomial, but with a different parameterization:

.. math::

   t_{\rm CoG3} = a +( b + cd^2) t_{\rm raw} - cd t_{\rm raw}^2 + c t_{\rm raw}^3/3

**ELS3 Calibration Function**

For ``ELS3`` we use the following calibration function:

.. math::

   t_{\rm ELS3} = a + b t_{\rm raw} + \frac{c}{t_{\rm raw} - d}\quad {\rm for} \quad x < d - \frac{\sqrt{-c}}{4}


.. _svdclustertimeshifting:

SVD Cluster-Time Shift w.r.t. Cluster-Size
------------------------------------------
The mean of the cluster-time distribution shifts with the cluster size, as the strips of the clusters with lower amplitude, which are at the edge, delays in time. This effect is not simulated and it appears only in data. To compensate this, improving cluster time resolution in data and data-simulation agreement, a shift is added to the calibrated time.

The calculation of shift values is implemented in the :ref:`calibration_caf` and is run on **6-sample data** with AirFlow and depends on :ref:`svdtimecalib`.

For all three time estimators ``CoG6``, ``CoG3`` and ``ELS3``, we build the 1D plot of the cluster-time :math:`\left[t^{\rm Calibrated}_{\rm Cluster} - t_0^{\rm Event}\right]`; where :math:`t_{\rm Cluster}` is calibrated time of clusters in an event and :math:`t_0^{\rm Event}` is the EventT0 (mainly from CDC). Then the 1D plots are processed to find the peak.

The steps are in the following:

#. The 1D plot, mentioned above, is fitted with a single Gaussian function;
#. The same plot is fitted with a double Gaussian, using the fit result of the single Gaussian as initial parameters;
#. The value of shift is taken as follows:

   #. if the double Gaussian fit is successful, the shift considered is the mean of the Gaussian with :math:`f>0.5`;
   #. else if single Gaussian fit is successful, the shift is the mean of the single Gaussian;
   #. else shift is the mean of the histogram.

The shift-values are stored in :ref:`SVDClusterTimeShifter<svdclustertimeshifter>` DBObject.
