.. _tracking_trackFinding: 

Track Finding Algorithms
========================

.. warning::
  This documentation is under construction!

We have three classes of algorithms for the hit finding:

#. SVD-only pattern recognition: find tracks using :ref:`SVD SpacePoints<svdsps>`
#. CDC-only pattern recognition: find tracks using CDC hits
#. inter-detector hit finding: find hits extrapolating existing tracks toward other detectors.

.. toctree::
    :glob:
    :maxdepth: 1

    trackFindingSVD
    trackFindingCDC
    ckf
