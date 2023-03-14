.. _svd:

SVD
===

The Silicon Vertex Detector (SVD) contributes to tracking and particle identification (PID).
In particular, SVD reconstruction provides the position, charge and time of the 1-D hit (**cluster**) on each sensor side. The charge is used for :math:`\frac{dE}{dx}` PID, while the combination of clusters on the two sides of the sensor provides the 3-D position of the hits that are used for the tracking pattern recognition and track fit.

.. _svdtb:

SVD raw data consists of:

1. list of acquired strips (SNR > 3): for each strip the amplitude (in ADC) of each of the 6 (or 3) acquired samples from the digitization of the APV (readout chip) waveform

2. information of the arrival of the trigger signal with respect to the SVD time-reference frame, used to express the hit time in the trigger time-reference frame. In particular, the ``TriggerBin`` is stored in the raw data and is the quarter of the APV clock in which the L1 trigger was received.

3. additional information on the acquisition mode, data format, ...

In addition, calibration constants from local runs (strip gain, noise, ...) and from offline calibrations (hit time, :math:`\frac{dE}{dx}`) are used in reconstruction.

In the following we describe the ``svd`` package .

.. warning::
   if you spot unclear or incorrect information, let us know at svd-software@belle2.org

.. toctree:: reconstruction

.. toctree:: offcalib

.. toctree:: simulation

.. toctree:: tools

.. toctree:: utilities 

.. toctree:: dqm

.. toctree:: datastore

.. toctree:: dbstore

.. toctree:: todo
