.. _tracking_eventTimeExtraction: 

Event Time Extraction
=====================
  
The event time (EventT0) is the time of the collision (in ns) with respect to the trigger.
There are several estimators in the tracking package:

#. CDC Hit Based estimator: used in the CDC pattern recognition step

#. CDC Chi2-based Full Grid estimator: the most precise estimator, but has a long execution time

#. SVD estimator: the default estimator

SVD Event Time Estimator
------------------------
This is a simple estimator that is computed after tracking: the SVD clusters used
in the reconstruction of tracks with :math:`p_T > 250` MeV/c are selected and then 
compute the time of the event as the average of the cluster times. This is the default
time of the event computed by tracking.

CDC Hit Based Estimator
-----------------------
This estimator is run at the beginning of CDC track finding. It's efficiency is very high,
i.e. provides an EventT0 for all events provided there are enough CDC hits, but it's less
precise than the other estimators.

CDC Chi2-based Full Grid Estimator
----------------------------------
This is the most precise estimator of the time of the event, and it is used for the time calibration 
of the other detectors (SVD, ECL, ...). It is not run during reconstruction because
its execution time is significantly higher than the SVD EventT0 estimator.