.. _tracking_qualityIndicators: 

Track Quality Indicator
^^^^^^^^^^^^^^^^^^^^^^^

There are three track quality indicators based on BDTs. The ``CDC Quality Estimator (CDC QE)`` utilizes CDC level quantities.
Analogously, the ``VXDTF2 QE`` is based on VXD level quantities and the ``RecoTrack QE`` is using quantities that are available on ``RecoTrack`` level.

All of them show large potential to reject fake and clone tracks while truth tracks are kept.
As BDTs tend to enhance data-MC disagreements at their border regions (very truth-like or very fake-like), the quality indicators were
revisited with respect to the data-MC disagreement that they introduce.

Due to limited man power, only the CDC QE is activated at the moment. It was trained on MC13 with a very reduced set of 
quantities that showed reasonable data-MC agreements. The most important variables to separate fakes and clones from truth tracks are the 
number of super layers that contain hits of the track, z0 and the sz-slope.

To further reduce the risk of introducing data-MC disagreements, a very conservative cut value of 0.7 was chosen. It was shown that it would 
be beneficial to select a tighter cut value (like 0.9) in future. One way of avoiding to introduce data-MC discrepancies would be to choose 
slightly different values for data and MC that agree with respect of the ratio of tracks that are kept.

The CDC QE rejection cut is conducted just after the :ref:`CDC Track Finding<tracking_trackFindingCDC>`. Thus, less ``CDCTracks`` are forwarded
to the SVD track finding which means that less SVD hits are connected to these tracks and more SVD hits remain "free" for the 
:ref:`SVD Track Finding<tracking_trackFindingSVD>`. It was shown that several truth tracks, that were rejected by the CDC QE cut, are found again 
in the SVD. Due to less "noise" due to fake and clone CDCTracks, the SVD track finding seems to find even more truth tracks than without CDC QE cut.
As a consequence, **the overall track finding efficiency increases** slighty (by up to 0.5%). This could be improved to more than 2% while 
the fake and clone rate are reduced further by choosing a tighter cut on the CDC QE as mentioned above.


It is assumed that the modelling of more properties with large separation potential between fakes, clones and truth tracks improved in the
last MC productions. Moreover, **the activation of the VXD- and Recotrack-QE would further increase the fake- and clone rejection by 50% and 30%**
compared to the current value while maintaining the same track finding efficiency.