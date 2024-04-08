.. _svddbstore:

SVD DBStore
===========

Here we briefly describe the SVD objects that populate the Conditions Database. We also report the revision number which is used for experiment 0 and 1003.

.. note::

   * the revision can be different for official run-independent MC production of experiment 0 and 1003, please check the corresponding Global Tag.
   * for run-dependent and data reconstruction revisions should be checked inspecting the content of the corresponding Global Tags.
   * you can also check the `SVD Payloads, Revisions and GT <https://confluence.desy.de/pages/viewpage.action?pageId=179781819>`_ Confluence Page.

We add a string, the ``UniqueID``, to the payloads that provides information on the content of the payload. In order to read the ``UniqueID`` you can setup basf2 and then:

.. code::

   > b2conditionsdb dump -r YZT X

where :code:`YZT` is the payload name and :code:`X` is the revision number. The ``UniqueID`` will show up at the end.

.. _granularity:

Wrappers have one of the following granularity:

#. strip granularity: one value/payload per strip
#. sensor-side granularity: one value/payload per sensor side
#. detector granularity: one value/payload for all sensors
#. sensor-on-ladder granularity: one value/payload per sensor side integrated over all ladders


.. warning:
   this is not a detailed description, please check svd/dbobjects and svd/calibration if you want to know more

Simulation Calibrations
-----------------------

In simulation, sensors are grouped by type (L3, origami, backward, forward) and by side (u/P, v/N). Sensors in the same group share the same calibration constants.

.. _svdchargesimulation:

.. cpp:class:: SVDChargeSimulationCalibrations

	       Parameters for strip charge simulation, with sensor-side granularity_:

	       #. electron weight: describing the difference between Geant4 electrons/holes and real electrons/holes
	       #. coupling constants: describe the charge sharing among implants up to readout-to-next-to-next-to-floating

Check `ChargeSimulation@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDChargeSimulationCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **4** is used for experiment 0 and 1003.


.. _svdfudgeposition:

.. cpp:class:: SVDMCClusterPositionFudgeFactor

	       Parameter to fudge position reconstruction in simulation, to match resolution measured in data, with sensor-side granularity_:

	       #. fudge factor: width of the gaussian from which we draw a random number to be added to the reconstructed cluster position, depends on the track incident angle

Check `MCPositionFudge@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDMCClusterPositionFudgeFactor&perpage=25&>`_ all revisions available in the Conditions Database. Revision **5** is used for experiment 0 and 1003.

.. _svdfudgetime:

.. cpp:class:: SVDMCClusterTimeFudgeFactor

	       Parameter to fudge time reconstruction in simulation, to match resolution measured in data, with sensor-side granularity_:

	       #. fudge factor: width of the gaussian from which we draw a random number to be added to the reconstructed cluster time

Check `MCTimeFudge@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDMCClusterTimeFudgeFactor&perpage=25&>`_ all revisions available in the Conditions Database. Revision **3** is used for experiment 0 and 1003, and run-dependent MC.

The local calibrations wrappers  :ref:`SVDNoiseCalibrations<svdnoisecal>`, :ref:`SVDPulseShapeCalibrations<svdpulsecal>` and :ref:`SVDFADCMaskedStrips<svdmaskedcal>` are also used in the digitizer.

Local Calibrations
------------------

.. _svdnoisecal:

.. cpp:class:: SVDNoiseCalibrations

	       Wrapper with strip granularity_  storing the strip noise

Check `Noise@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDNoiseCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **510** is used for experiment 0 and 1003.

.. _svdpulsecal:

.. cpp:class:: SVDPulseShapeCalibrations

	       Wrapper with strip granularity_  storing strip gain (ADC/e-), peak time and pulse width

Check `PulseShape@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDPulseShapeCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **538** is used for experiment 0 and 1003.

.. _svdmaskedcal:

.. cpp:class:: SVDFADCMaskedStrips

	       Wrapper with strip granularity_  storing the strips masked on FADC

Check `MaskedStrips@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDFADCMaskedStrips&perpage=25&>`_ all revisions available in the Conditions Database. Revision **108** is used for experiment 0 and 1003.

Detector Configuration
----------------------

.. _svdglobalconfig:

.. cpp:class:: SVDGlobalConfigParameters

	       Stores the detector configuration for the Belle2 run, with detector granularity_

Check `GlobalConfig@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDGlobalConfigParameters&perpage=25&>`_ all revisions available in the Conditions Database. Revision **70** is used for experiment 0 and 1003.

.. cpp:class:: SVDLocalConfigParameters

	       Stores the detector configuration for a local run, with detector granularity_

Check `LocalConfig@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDLocalConfigParameters&perpage=25&>`_ all revisions available in the Conditions Database. Revision **63** is used for experiment 0 and 1003.


Reconstruction Calibrations
---------------------------


.. _svdrecoconfiguration:

.. cpp:class:: SVDRecoConfiguration

	       Stores SVDRecoConfiguration (time, position, charge, etc), with detector granularity_

   Check `SVDRecoConfiguration@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDRecoConfiguration&perpage=25&>`_ all revisions available in the Conditions Database. Revision **10** is used for experiment 0 (with grouping module turned ON), revision **11** is used for experiment 1002/3/4.

.. _svdcogonlyerr:

.. cpp:class:: SVDCoGOnlyPositionError

	       Stores the position error formulas for the ``CoGOnly`` algorithm, with the same granularity_ as simulation objects

Check `CoGOnlyErr@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDCoGOnlyPositionError&perpage=25&>`_ all revisions available in the Conditions Database.  Revision **5** is used for exp 0, 1003 and data reconstruction.

.. _svdcogonlycal:

.. cpp:class:: SVDCoGOnlyErrorScaleFactors

	       Stores the position error scaling factors for the ``CoGOnly`` algorithm, with the same granularity_ as simulation objects

Check `CoGOnlySF@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDCoGOnlyErrorScaleFactors&perpage=25&>`_ all revisions available in the Conditions Database. Revision **2** (scale factors = 1) is used for exp 0, 1003 and data reconstruction.
While revision **6** (scale factors != 1) is used for data.

.. _svdolddefaultcal:

.. cpp:class:: SVDOldDefaultErrorScaleFactors

	       Stores the position error scaling factors for the ``OldDefault`` algorithm, with the same granularity_ as simulation objects

Check `OldDefaultSF@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDOldDefaultErrorScaleFactors&perpage=25&>`_ all revisions available in the Conditions Database. Revision **3** is used for experiment 0 and 1003. Revision **2** (scale factors = 1) should be used for data reconstruction.

.. _svdclustercuts:

.. cpp:class:: SVDClustering

	       Stores the parameters for the clusterization with sensor-side granularity_

Check `Clustering@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDClustering&perpage=25&>`_ all revisions available in the Conditions Database. Revision **1** is used for experiment 0 and 1003, and data.

.. _svdhittimeselection:

.. cpp:class:: SVDHitTimeSelection

	       Stores the functions and their parameters for the selection of clusters based on the cluster time in the SpacePoint creation step, with sensor-side granularity_.

Check `HitTimeSelection@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDHitTimeSelection&perpage=25&>`_ all revisions available in the Conditions Database. Revision **1** (no cuts) is used for experiment 1003 (and data), and revision **5** (u/v cluster time difference < 20 ns & u-or-v cluster absolute time < 50 ns) is used for experiment 0.

.. cpp:class:: SVDOccupancyCalibrations
	       
	       wrapper with the strip occupancy averaged over a run, strip granularity_

.. _svdhotstrips:

.. cpp:class:: SVDHotStripsCalibrations

	       wrapper with the hot strips as determined by ``SVDHotStripFinder``, strip granularity_

.. _svdtimegroupingconfiguration:

.. cpp:class:: SVDTimeGroupingConfiguration

	       Stores parameters used in SVDTimeGrouping module, with detector granularity_

   Check `SVDTimeGroupingConfiguration@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDTimeGroupingConfiguration&perpage=25&>`_ all revisions available in the Conditions Database. Revision **3** is used for experiment 0 and 1003/4, revision **4** is calculated from *Exp24-Run1726* and should be used for data.


Time Calibrations
-----------------

.. _svdcog6timecal:

.. cpp:class:: SVDCoGTimeCalibrations

	       CoG6 Time calibration wrapper, with sensor-side granularity_

Check `CoG6Time@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDCoGTimeCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **203** is used for experiment 0 and 1003, revision **7** correspond to no calibration.


.. _svdcog3timecal:

.. cpp:class:: SVD3SampleCoGTimeCalibrations

	       CoG3 Time calibration wrapper, with sensor-side granularity_

Check `CoG3Time@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVD3SampleCoGTimeCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **2** is used for experiment 0 and 1003, revision **1** correspond to no calibration.

.. _svdels3timecal:

.. cpp:class:: SVD3SampleELSTimeCalibrations
	       
	       ELS3 Time calibration wrapper, with sensor-side granularity_

Check `ELS3Time@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVD3SampleELSTimeCalibrations&perpage=25&>`_ all revisions available in the Conditions Database. Revision **2** is used for experiment 0 and 1003, revision **1** correspond to no calibration.

.. _svdclustertimeshifter:

.. cpp:class:: SVDClusterTimeShifter

	       Time shift adjustment depending on cluster-size, with sensor-on-ladder granularity_

   Check `SVDClusterTimeShifter@CDB <https://cdbweb.sdcc.bnl.gov/Payload?basf2=SVDClusterTimeShifter&perpage=25&>`_ all revisions available in the Conditions Database. Revision **1** is used for experiment 0 and 1002/3/4 which corresponds to no shift, revision **3** is calculated from *Exp24,Run1726* and should only be used for data.
