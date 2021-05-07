
SVD
===

The Silicon Strip Detector (SVD) contributes to tracking and particle identification (PID).
In particular, SVD reconstruction provides the position, charge and time of the 1-D hit (``cluster``) on each sensor side. The charge is used for :math:`\frac{dE}{dx}` PID, while the combination of ``clusters`` on the two sides of the sensor provides the 3-D position of the hits that are used for the tracking pattern recognition and track fit.

SVD raw data consists of:

* list of strips above threshold SNR > 3: for each strip the amplitude (in ``ADC``) of each of the 6 (or 3) acquired samples from the digitization of the APV (readout chip) waveform
* information of the arrival of the trigger signal with respect to the SVD time reference frame. This information is used to express the hit time in the trigger time-reference frame.
* additional information on the acquisition mode, data format, ...

In addition, calibration constants from local runs (strip gain, noise, ...) and from offline calibrations (hit time, :math:`\frac{dE}{dx}`) are used in reconstruction.

In the following we describe the ``svd`` package .

.. warning::
   documentation under development

.. toctree:: reconstruction

.. toctree:: offcalib

.. toctree:: simulation

.. toctree:: utilities 

.. toctree:: dqm

.. toctree:: datastore

.. toctree:: dbstore

.. toctree:: todo
