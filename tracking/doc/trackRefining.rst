.. _trk_refining:

Track Refining
--------------

The quality of the tracks after the finding and fitting steps is further improved by the following algorithms.


.. _trk_flipNrefit:

Flip & Refit
^^^^^^^^^^^^

Some tracks may have the correct pattern, i. e. hits are correct, but the charge assignment is wrong. This happens
mostly for low momentum tracks in the transverse plane (small :math:`p_t`, even smaller :math:`p_z`).
To fix the charge of these tracks, and improve the quality of the track parameters, we apply the following algorithm
at the very end of the tracking chain:

1) select the tracks that may have the wrong charge assignment based on low level quantities
2) flip these tracks and refit them
3) for each of them, the orginal and flipped tracks are compared and the best one is kept

Step 1 and 3 are achieved using two dedicated MVAs (one per step) that are trained on simulated events 
(details on training are :ref:`tracking_calibration_flipNrefit`).

