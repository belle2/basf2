.. _svdsimulation:

SVD Simulation
==================

The SVD simulation starts with ``SVDSimHits`` and ends with :ref:`SVDShaperDigits<svdshapers>` and :ref:`SVDEventInfo<svdeventinfo>`.

First of all the SVD configuration is written in the :ref:`SVDEventInfo<svdeventinfo>` (DAQ mode, number of APV samples in the event, ...).
Each chargelet (small group of ionized :math:`e^{-}` or :math:`h^{+}`) is drifted towards the sensor sides (:math:`e^{-}` are collected on the v/N side, :math:`h^{+}` are collected on the u/P side) in a simplified electromagnetic field (including diffusion) and then each charge is assigned to the nearest readout or floating implant. Charge sharing among implants is simulated using coupling constants measured on data and stored in :ref:`SVDChargeSimulationCalibrations<svdchargesimulation>`. Each charge :math:`Q` on a readout strip is assigned an approximate time :math:`\delta` equals to the time it reaches half-way to the sensor side. The APV25 waveform is simulated in :math:`e^{-}` for each chargelet getting to the sensor side with the following function:

.. math::

   w(t) = Q \cdot \frac{149.012 \cdot \left(\frac{t-\delta}{\tau}\right)^2}{\left[1 + \left(\frac{t-\delta}{\tau}\right)\right]^{10}}

..   w(t|\delta,\tau) = Q \cdot \frac{t-\delta}{\tau} * \exp{(1 - \frac{t-\delta}{\tau})} \quad {\rm for }\quad t \ge \delta


where :math:`Q` is the charge, :math:`\delta` is the start time of the waveform and :math:`\tau = 250` ns.

The total analog APV waveform is the sum of contributions from each charge.

When all ionized charges are collected on the implants, the final APV waveform is digitized in ADC taking into account that an :math:`e^{-}/h^{+}` in Geant4 is not exactly equivalent to a real :math:`e^{-}/h^{+}` and therefore is weighted with the ``electronWeight`` stored in :ref:`SVDChargeSimulationCalibrations<svdchargesimulation>`, and then transformed in ADC using the gain stored in :ref:`SVDPulseShapeCalibrations<svdpulsecal>`. Noise from :ref:`SVDNoiseCalibrations<svdnoisecal>` is superimposed to the samples. We then check that at least one of the 6 samples is above the zero-suppression threshold and we mask strips according to :ref:`SVDFADCMaskedStrips<svdmaskedcal>` and disable APVs according to the channel mapping. Finally :ref:`SVDShaperDigits<svdshapers>` are created.

In case the event is a 3-sample event, the 3 out of the 6 samples are identified and :ref:`SVDShaperDigits<svdshapers>` are filled with the 3 selected samples in the first 3 positions, and 0 in the rest, and the zero-suppression is checked again.

.. note::
   SVD sampling clock period is :math:`t_{\rm APV} = 31.44` ns, 4 times larger than the system (trigger) clock. In order to include this in the simulation, we move the acquisition window (start of APV sampling) by units of 1/4 of APV clock (0,1,2,3, the ``TriggerBin``), synchronized with the other detectors (i.e. detector clocks keep the same phase across events).

Use the following python function if you want to add the SVD simulation to your steering file:

.. autofunction:: svd.__init__.add_svd_simulation


Simulation Modules
------------------

This is a list of the ``svd`` modules used for simulation. 

.. b2-modules::
   :package: svd
   :modules: SVDEventInfoSetter, SVDDigitizer
   :io-plots:
