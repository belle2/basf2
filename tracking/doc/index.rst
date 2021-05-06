.. _tracking:

================
Tracking Package
================

.. warning::
  This documentation is under construction!

The ``tracking`` package provides the algorithms used for track finding and track fitting in Belle II. In addition, a valuable source of informaion on the Belle II tracking algorithms is the `Belle II tracking paper <https://arxiv.org/abs/2003.12466>`_.
This documentation describes the Belle II Tracking, more precisely:

* the concepts and ideas of the algorithms used in tracking in Belle II 
  .. seealso:: `the tracking paper <https://arxiv.org/abs/2003.12466>`_
* the tracking package with its sub-packages
* some key concepts of the implementation of the tracking code
* how to train the various multivariate filters that are employed in tracking.

The tracking package includes the following sub-packages

.. toctree::
    :glob:
    :maxdepth: 3
    
    calibration
    ckf
    ckf/*
    datastore
    dbobjects
    dqmUtils
    eventTimeExtraction
    examples
    modules/*
    pxdDataReduction
    scripts/*
    spacePointCreation
    svdROIFinder
    tests
    tools
    trackFindingCDC/*
    trackFindingVXD/*
    trackFitting
    v0Finding
    validation
    vxdCaTracking
    vxdMomentumEstimation
