.. _tracking:

Tracking
========

.. warning::
  This documentation is under construction!

The main goal of the tracking reconstruction is to find and fit tracks using the hit informations provided by the main tracking sub-detectors: PXD, :ref:`SVD<svd>`, CDC.
In general, tracking consists in two steps:

#. pattern recognition, or track finding: recognise the hits belonging to a single charged particle 

   .. seealso::

      The `Track Finding at Belle II <https://arxiv.org/abs/2003.12466>`_ provides a nice introduction to Belle II tracking and describe the track finding algorithms and their performance on simulated events.

#. track fit: extract the track parameters from a fit to the pattern (collection of hits).

The tracking reconstruction chain is built as a modular system that can be adapted to different background levels and detector performance. We combine stages of pattern recognition and fitting for the different detectors in order to improve efficiency and minimise fakes and clones.

SVD and CDC provide hits for the first steps of the pattern recognition. In order to optimise the performance of the pattern recognition we have different algorithms for the CDC and for the SVD. Hits in the PXD are added to existing tracks to improve the track quality. 

.. note::

   Beside the offline reconstruction, the tracking reconstruction is performed also *online* on the High Level Trigger (**HLT**): the two reconstructions are slightly different. Beside the fact that PXD hits are not available on HLT, tracking on HLT is optimized to satisfy the execution time and memory limits with no significant impact on the physics performance.

Use :ref:`add_tracking_reconstruction<tracking_addtrkreconstruction>` to append the tracking reconstruction to your path:

.. autofunction:: tracking.__init__.add_tracking_reconstruction
		  :noindex:
.. toctree::
    :glob:
    :maxdepth: 2

    trackFinding/trackFinding.rst
    trackFitting
    specials
    pxdDataReduction
    calibration
    scripts.rst
    tools
    glossary

    
More ?
------

The tracking package includes the following sub-packages

.. toctree::
    :glob:
    :maxdepth: 3

    datastore
    dbobjects
    dqmUtils
    eventTimeExtraction
    modules/*
    spacePointCreation
    svdROIFinder
    tests
    validation
    vxdCaTracking
    vxdMomentumEstimation
