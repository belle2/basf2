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

``SVDChargeSimulationCalibrations``, simulated strip charge (sensor-side granularity)

#. electron weight: describing the difference between Geant4 electrons/holes and real electrons/holes
#. coupling constants: describe the charge sharing among implants up to readout-to-next-to-next-to-floating

The local calibrations wrappers ``SVDNoiseCalibrations`` and ``SVDPulseShapeCalibrations`` and ``SVDFADCMaskedStrips`` are also used in the digitizer.

Local Calibrations
------------------

All these wrappers have strip granularity_:

#. ``SVDNoiseCalibrations``, storing strip noise
#. ``SVDPulseShapeCalibrations``, storing strip gain (ADC/e-), peak time and pulse width
#. ``SVDPedestalCalibrations``, storing strip pedestals
#. ``SVDFADCMaskedStrips``, storing strips masked on FADC


Detector Configuration
----------------------
``SVDDectectorConfiguration`` wrapper with detector granularity_:

#. ``SVDGlobalConfigParameters`` storing the detector configuration for the Belle2 run
#. ``SVDLocalConfigParameters`` storing the detector configuration for a local run

Reconstruction Calibrations
---------------------------

``SVDClusterCalibrations`` wrapper with sensor-side granularity_ of:

#. ``SVDClusterCuts``: stores parameters for the clusterization
#. ``SVDHitTimeSelection``: stores functions and their parameters for the selection of clusters based on the cluster time

``SVDOccupancyCalibrations`` wrapper with hot strips as determined by ``SVDHotStripFinder``, strip granularity

``SVDHotStripsCalibrations`` wrapper with hot strips as determined by ``SVDHotStripFinder``, strip granularity


Time Calibrations
-----------------

All these wrappers have sensor-side granularity_:

#. ``SVDCoGTimeCalibrations``, CoG6 Time calibration wrapper
#. ``SVD3SampleCoGTimeCalibrations``, CoG3 Time calibration wrapper
#. ``SVD3SampleELSTimeCalibrations``, ELS3 Time calibration wrapper

