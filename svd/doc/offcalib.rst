.. _svdoffcalib:

SVD Offline Calibration
=======================

Here we briefly describe the so-called offline calibrations.

Hit Time Calibration
---------------------

The time calibration is implemented in the :ref:`calibration_caf` and is run on data with AirFlow, on data acquired in the 6-sample DAQ mode.

The hit time calibration exploits the correlation between the time of the event ``EventT0`` and the hit time. We select clusters associated to tracks and neglect the flight time of the particle.

For all three time estimators ``CoG6``, ``CoG3`` and ``ELS3`` we first compute the time of the event in the SVD reference frame, :math:`t_0^{\rm SVD}`: 

.. math:: 

   t_0^{\rm SVD} = t_0 - \frac{\Delta t}{4} \cdot (3 - TB + 4\ FF)

where :math:`\Delta t \simeq 31.44` ns is the sampling period of the APV readout chip, :math:`TB = 0,1,2,3` is the :ref:`TriggerBin<svdtb>` and provides the correct time shift to move into the SVD reference frame, and :math:`FF=0,1,2,3` (:ref:`FirstFrame<svdff>` is applied only for ``CoG3`` and ``ELS3`` due to the :ref:`MaxSum<MaxSum>` algorithm.

We build the 2D plot of the raw hit time :math:`t_{\rm raw}` vs :math:`t_0^{\rm SVD}`. Note that the :math:`t_{\rm raw}` is obviously different for the three estimator.
 We then take the ``ProfileX`` of the 2D plot and fit the correlation with a function that depends on the estimator. 

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
