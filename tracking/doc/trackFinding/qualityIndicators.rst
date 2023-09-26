.. _tracking_qualityIndicators:

Track Quality Indicator
^^^^^^^^^^^^^^^^^^^^^^^

There are three track quality indicators based on BDTs. The ``CDC Quality Indicator (CDC QI)`` utilizes CDC level quantities.
Analogously, the ``VXDTF2 QI`` is based on VXD level quantities and the ``RecoTrack QI`` is using quantities that are available on ``RecoTrack`` level.

All of them show large potential to reject fake and clone tracks while truth tracks are kept.
As BDTs tend to enhance data-MC disagreements at their border regions (very truth-like or very fake-like), the quality indicators were
revisited with respect to the data-MC disagreement that they introduce.

Due to limited person power, only the CDC QI is activated at the moment. It was trained on MC13 with a very reduced set of
quantities that showed reasonable data-MC agreements. The most important variables to separate fakes and clones from truth tracks are the
number of super layers that contain hits of the track, z0 and the sz-slope.

To further reduce the risk of introducing data-MC disagreements, a very conservative cut value of 0.7 was chosen. It was shown that it would
be beneficial to select a tighter cut value (like 0.9) in future. One way of avoiding to introduce data-MC discrepancies would be to choose
slightly different values for data and MC that agree with respect of the ratio of tracks that are kept.

The CDC QI rejection cut is conducted just after the :ref:`CDC Track Finding<tracking_trackFindingCDC>`. Thus, less ``CDCTracks`` are forwarded
to the SVD track finding which means that less SVD hits are connected to these tracks and more SVD hits remain "free" for the
:ref:`SVD Track Finding<tracking_trackFindingSVD>`. It was shown that several truth tracks, that were rejected by the CDC QI cut, are found again
in the SVD. Due to less "noise" due to fake and clone CDCTracks, the SVD track finding seems to find even more truth tracks than without CDC QI cut.
As a consequence, **the overall track finding efficiency increases** slighty (by up to 0.5%). This could be improved to more than 2% while
the fake and clone rate are reduced further by choosing a tighter cut on the CDC QI as mentioned above.


It is assumed that the modelling of more properties with large separation potential between fakes, clones and truth tracks improved in the
last MC productions. Moreover, **the activation of the VXD- and Recotrack-QI would further increase the fake- and clone rejection by 50% and 30%**
compared to the current value while maintaining the same track finding efficiency.

More details concerning the different QIs and how to train them can be found on Confluence: "MVA Track Quality Indicator".