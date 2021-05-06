.. _tracking_datastore: 

Tracking DataStore (dataobjects)
================================

.. warning::
  This documentation is under construction!

This section is meant to give a brief introduction into the tracking DataStore objects.

.. warning::
    This only is a very superficial description of the single objects. For more details please check ``tracking/dataobjects``

.. _bremhit:

.. cpp:class:: BremHit
    
    TODO

.. _exthit:

.. cpp:class:: ExtHit
    
    TODO

.. _filterid:

.. cpp:class:: FilterID

    TODO

.. _filterinfo:

.. cpp:class:: FilterInfo
    
    TODO

.. _fullsecid:

.. cpp:class:: FullSecID
    
    TODO

.. _hitxp:

.. cpp:class:: hitXP
    
    TODO

.. _hitxpdderivative:

.. cpp:class:: hitXPDerivative
    
    TODO

.. _mcparticleinfo:

.. cpp:class:: MCParticleInfo
    
    TODO

.. _observerinfo:

.. cpp:class:: ObserverInfo
    
    TODO

.. _pxdintercept:

.. cpp:class:: PXDIntercept
    
    StoreArray that inherits from :ref:`VXDIntercept<vxdintercept>`. The PXDIntercept is used to calculate Region of Interest (:ref:`ROIid<roiid>`) on the PXD (TODO: reference to PXD once existing) for online data reduction of the PXD data. The PXDIntercept is created by the :b2:mod:`PXDROIFinder`.

.. _recohitinformation:

.. cpp:class:: RecoHitInformation
    
    TODO

.. _recotrack:

.. cpp:class:: RecoTrack
    
    TODO

.. _roiid:

.. cpp:class:: ROIid
    
    StoreArray for the PXD Region of Interest (ROI). Contains:
    
    #. layer, ladder, sensor (VxdID) of the ROI
    #. corners of the ROI in terms of pixels as minU, maxU, minV, maxV

.. _roipayload:

.. cpp:class:: ROIpayload
    
    TODO

.. _roirawid:

.. cpp:class:: ROIrawID
    
    TODO

.. _sectormapconfig:

.. cpp:class:: SectorMapConfig
    
    TODO

.. _spacepointinfo:

.. cpp:class:: SpacePointInfo
    
    TODO

.. _svdintercept:

.. cpp:class:: SVDIntercept
    
    StoreArray that inherits from :ref:`VXDIntercept<vxdintercept>`. The SVDIntercept is used for SVD (TODO: references to SVD once existing) performance studies. In addition to the base class it contains:
    
    #. information of the direction of a track at the position of the intercept.

.. _trackclusterseparation:

.. cpp:class:: TrackClusterSeparation
    
    TODO

.. _trackfindervxdtypedefs:

.. cpp:class:: TrackFinderVXDTypedefs
    
    TODO

.. _v0validationvertex:

.. cpp:class:: V0ValidationVertex
    
    TODO

.. _vxdintercept:

.. cpp:class:: VXDIntercept

    StoreArray that contains information about an extrapolated position to a VXD (TODO: reference to VXD once existing) sensor. Base class of :ref:`PXDIntercept<pxdintercept>` and :ref:`SVDIntercept<svdintercept>` and contains:
    
    #. layer, ladder, sensor (VxdID) of the intercept
    #. local u and v coordinate of the intercept and the corresponding statistical uncertainties
    #. statistical uncertainties of the direction at the extrapolated position
