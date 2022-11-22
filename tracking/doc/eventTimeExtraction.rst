.. _tracking_eventTimeExtraction: 

Time Extraction
=====================

Track Time
----------
The track time is the time at which the track is produced with respect to the collision time (given by SVD EventT0).
We define the outgoing (ingoing) arm as the part of the helix that exits (enters) the IR. 
They are identified using the order of the detectors associated to the track hits:
for example, the arm is considered outgoing (ingoing) if the hits are found in PXD-SVD-CDC 
(CDC-SVD-PXD). Outgoing (ingoing) arm time is computed with the SVD clusters used in the reconstruction of 
the outgoing (ingoing) arm of the track as the average of the cluster times.
If the track has only the outgoing (ingoing) arm, the track time is computed as the difference of the outgoing
(ingoing) arm time and the event time (EventT0, see next section). 
In case the track has both outgoing and ingoing arms, the one with the smallest time is used.

.. note::
   Tracks can have roughly:

   #. outgoing arm only (98.5%)

   #. ingoing arm only (0.9%)

   #. both outgoing and ingoing arms (0.6%) 
   

EventT0
-------
  
The event time (EventT0) is the time of the collision (in ns) with respect to the trigger.
There are several estimators in the tracking package:

#. CDC Hit Based estimator: used in the CDC pattern recognition step

#. CDC Chi2-based Full Grid estimator: the most precise estimator, but has a long execution time

#. SVD estimator: the default estimator

SVD Event Time Estimator
^^^^^^^^^^^^^^^^^^^^^^^^
This is a simple estimator that is computed after tracking: the SVD clusters, belonging to the 
outgoing arm (or to the ingoing arm, if its time is smaller than the outgoing arm time or if it 
is the only one available), used in the reconstruction of tracks with :math:`p_T > 250` MeV/c 
are selected and then compute the time of the event as the average of the cluster times. This 
is the default time of the event computed by tracking.

CDC Hit Based Estimator
^^^^^^^^^^^^^^^^^^^^^^^
This estimator is run at the beginning of CDC track finding. It's efficiency is very high,
i.e. provides an EventT0 for all events provided there are enough CDC hits, but it's less
precise than the other estimators.

CDC Chi2-based Full Grid Estimator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This is the most precise estimator of the time of the event, and it is used for the time calibration 
of the other detectors (SVD, ECL, ...). It is not run during reconstruction because
its execution time is significantly higher than the SVD EventT0 estimator.
