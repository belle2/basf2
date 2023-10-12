.. _tracking_trackFindingSVD:

SVD Track Finding
^^^^^^^^^^^^^^^^^

.. warning::
  This documentation is under construction!

The VXDTF2 is the standalone pattern recognition algorithm for the both of the silicon detectors. It is capable of 
doing full 6 layer tracking (SVD+PXD). Though in the current tracking chain PXD hits are treated separately thus the 
VXDTF2 is used for SVD track finding only. In this page there is a brief description of the logic behind this pattern recognition algorithm.

Reduction of the combinatorial burden with the SectorMaps
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""
The goal is to extract track patterns from a huge number of possible combinations of space points (3D hits) in the 4 SVD layers.
The logic behind the reduction of the number of combinations is the following:
divide the sensors into sectors (NxM). Only combine SpacePoints belonging to friend-sectors, where two sectors are defined as friend if they are directly connected by a track. 
Background SpacePoints are rejected using dedicated filters which use simple geometric relations between sectors and hit time information.  

.. figure:: figures/friends.png
   :align: center
   :width: 80%

The connections defining friend sectors are learned from simulation. In order to avoid attaching background hits to patterns, filters based on 2- or 3-hit combinations are also learned from simulation.
The SectorMap stores the information about the friendship relations between sectors and a set of selection requirements (filters = {variable, range}) used to reject background hits. The filters 
are defined (and stored in the SectorMap) individually for each occurring combination of friend sector relations, meaning that different combination of friend sectors usually have 
different ranges for the filters defined.

.. container:: twocol

    .. container:: leftside

      .. figure:: figures/sectormap.png
	      :align: center

      Simplified view of the friendship relations

    .. container:: rightside
    
      .. figure:: figures/filter.png
        :align: center
        :width: 50%
    
      Two-hit filter example

Variables used in the filters are geometric (distances, slopes) or properties of the SVD clusters, like the cluster time. There are two types of filters defined within 
the framework. There are 2-hit filters, defined for the selection of 2-hit combination, and 3-hit filters, defined for the selection of 3-hit combinations, defined. 
Variables used in the current implementation of filters are listed in the following tables:

# defined in tracking/trackFindingVXD/filterMap/

+----------------------------+-------------------------------------------------------------------------------------------+
| variables for 2-hit filter | description                                                                               |
+----------------------------+-------------------------------------------------------------------------------------------+
|Distance3DSquared.h         | squared distance of the two SpacePoints                                                   |
+----------------------------+-------------------------------------------------------------------------------------------+
|Distance3DNormed.h          | squared distance in the x-y plane divied by the 3D distance sqared of the two SpacePoints |
+----------------------------+-------------------------------------------------------------------------------------------+
|Distance2DXYSquared.h       | xy squared distance of the two SpacePoints in the x-y plane                               |
+----------------------------+-------------------------------------------------------------------------------------------+
|Distance1DZ.h               | distance of the two SpacePoints in the z direction                                        |
+----------------------------+-------------------------------------------------------------------------------------------+
|SlopeRZ.h                   | angle between the z direction and the direction defined by the two Space Points           |
+----------------------------+-------------------------------------------------------------------------------------------+
|DistanceInTimeUside.h       | time difference of the two u-side clusters of the SpacePoints                             |
+----------------------------+-------------------------------------------------------------------------------------------+         
|DistanceInTimeVside.h       | time difference of the two v-side clusters of the SpacePoints                             |
+----------------------------+-------------------------------------------------------------------------------------------+


+-----------------------------+-----------------------------------------------------------------------------------------------------
| variables for 3-hit filters | description 
+-----------------------------+-----------------------------------------------------------------------------------------------------
| DistanceInTime              | time difference between u- and v- cluster of the center hit (of the 3-hit combination) 
+-----------------------------+-----------------------------------------------------------------------------------------------------
Angle3DSimple
+-----------------------------+-----------------------------------------------------------------------------------------------------
CosAngleXY
+-----------------------------+-----------------------------------------------------------------------------------------------------
AngleRZSimple
+-----------------------------+-----------------------------------------------------------------------------------------------------
CircleDist2IP
+-----------------------------+-----------------------------------------------------------------------------------------------------
DeltaSlopeRZ
+-----------------------------+-----------------------------------------------------------------------------------------------------
DeltaSlopeZoverS
+-----------------------------+-----------------------------------------------------------------------------------------------------
DeltaSoverZ
+-----------------------------+-----------------------------------------------------------------------------------------------------
HelixParameterFit
+-----------------------------+-----------------------------------------------------------------------------------------------------
Pt
+-----------------------------+-----------------------------------------------------------------------------------------------------
CircleRadius
+-----------------------------+-----------------------------------------------------------------------------------------------------




The allowed range for each filter is learned from simulation, filling the distribution of the variable during the training and defining threshold as max and min of the distribution (or 0.1% and 99.9% quantile?).
Using the friendship relations and the filters, segments connecting two, three or four SpacePoints are built. At this stage a single SpacePoint can be shared by more than one segment and we are ready to build track candidates that are identified and collected by a Cellular Automaton.

SectorMap Training
""""""""""""""""""

The training of the SectorMap is a critical step for the performance of the pattern recognition. Important aspects of the training are:

* division of the sensor in sectors: we use a 4x4 division
* sample used for the training (size, types of simulated events): we use :math:`10^6\ B-\bar{B}` + X bhabha events
* pruning: removal of friendship relations that are less used: the threshold is set at 70%
* difference in misalignment between simulation and real detector: we train with perfectly aligned MC

Track Candidates identified by the Cellular Automaton
"""""""""""""""""""""""""""""""""""""""""""""""""""""

The SectorMaps is used to produce the Segment Network, a set of segments (a pair of SpacePoints) that satisfy the friendship relations and the filters.
A Cellular Automaton which uses segments as cells is run to gather the longest paths.

.. figure:: figures/cellularautomaton.png
   :align: center

Best Candidate Selection
""""""""""""""""""""""""

Finally, a best candidate selection is performed by looking at a simple quality indicator:

* a Triplet Fit is applied to each path and sub paths obtained by excluding one or more space points
* for each track candidate the sum of the chi2 of each triplet is computed
* the p-value of each track candidate is used to select the track candidates competing for one or more space points

What about the  with the Hopfield Neural Network? is it used?
