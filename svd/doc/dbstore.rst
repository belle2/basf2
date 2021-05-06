.. _svddbstore:

SVD DBStore
===========

Here we briefly describe the SVD objects that populate the Conditions Database.

.. _granularity:

Wrappers have one of the following the following granularity:

#. strip granularity: one value/payload per strip
#. sensor-side granularity: one value/payload per sensor side
#. detector granularity: one value/payload for all sensors

.. warning:
   this is not a detailed description, please check svd/dbobjects and svd/calibration if you want to know more

Simulation Calibrations
-----------------------

In simulation, sensors are grouped by type (L3, origami, backward, forward) and by side (u/P, v/N). Sensors in the same group share the same calibration constats.

.. _svdchargesimulation:

.. cpp:class:: SVDChargeSimulationCalibrations

	       Parameters for strip charge simulation, with sensor-side granularity_:

	       #. electron weight: describing the difference between Geant4 electrons/holes and real electrons/holes
	       #. coupling constants: describe the charge sharing among implants up to readout-to-next-to-next-to-floating

The local calibrations wrappers  :ref:`SVDNoiseCalibrations<svdnoisecal>`, :ref:`SVDPulseShapeCalibrations<svdpulsecal>` and :ref:`SVDFADCMaskedStrips<svdmaskedcal>` are also used in the digitizer.

Local Calibrations
------------------

.. _svdnoisecal:

.. cpp:class:: SVDNoiseCalibrations

	       Wrapper with strip granularity_  storing the strip noise

.. _svdpulsecal:

.. cpp:class:: SVDPulseShapeCalibrations

	       Wrapper with strip granularity_  storing strip gain (ADC/e-), peak time and pulse width

.. _svdmaskedcal:

.. cpp:class:: SVDFADCMaskedStrips

	       Wrapper with strip granularity_  storing the strips masked on FADC

Detector Configuration
----------------------

.. cpp:class:: SVDGlobalConfigParameters

	       Stores the detector configuration for the Belle2 run, with detector granularity_

.. cpp:class:: SVDLocalConfigParameters

	       Stores the detector configuration for a local run, with detector granularity_

The ``SVDDectectorConfiguration`` wrapper contains both ``SVDGlobalConfigParameters`` and ``SVDLocalConfigParameters``.

Reconstruction Calibrations
---------------------------

.. _svdclustercuts:

.. cpp:class:: SVDClusterCuts

	       Stores the parameters for the clusterization with sensor-side granularity_

.. _svdhittimeselection:

.. cpp:class:: SVDHitTimeSelection

	       Stores the functions and their parameters for the selection of clusters based on the cluster time in the SpacePoint creation step, with sensor-side granularity_.

The ``SVDClusterCalibrations`` wrapper contains both ``SVDClusterCuts`` and ``SVDhitTimeSelection``.

.. cpp:class:: SVDOccupancyCalibrations
	       
	       wrapper with the strip occupancy averaged over a run, strip granularity_

.. cpp:class:: SVDHotStripsCalibrations

	       wrapper with the hot strips as determined by ``SVDHotStripFinder``, strip granularity_


Time Calibrations
-----------------

.. _svdcog6time:

.. cpp:class:: SVDCoGTimeCalibrations

	       CoG6 Time calibration wrapper, with sensor-side granularity_

.. _svdcog3time:

.. cpp:class:: SVD3SampleCoGTimeCalibrations

	       CoG3 Time calibration wrapper, with sensor-side granularity_

.. _svdels3time:

.. cpp:class:: SVD3SampleELSTimeCalibrations
	       
	       ELS3 Time calibration wrapper, with sensor-side granularity_

