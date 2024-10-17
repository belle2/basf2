.. _tracking_spacePointCreation:

Space Point Creation
====================

A Space Point is the 3D positional information of a detector hit. As CDC in general uses wires to detect hits, Space Points are more relevant for the SVD and PXD track finding or hit attachment. Space Points are given in global detector coordinates. Single measurements of sensors are given in local coordinates given in the coordinate system of the sensor, which is usually oriented along the edges of the sensor and labeled *u*, *v*, and *w*. To translate local coordinates into global coordinates a coordinate transformation is performed using the geometry. The class implementing the space points in `basf2` is `SpacePoint`.

SVD is a strip detector, meaning that a single measurement (`SVDCluster`) only gives the 1D position perpendicular to that strip. To obtain a Space Point from SVD measurements one needs to combine the measurements of two strips which are perpendicular oriented to each other (u- and v-side `SVDCluster`).  Note that if there are more than one hit on a single SVD sensor this will result into so called *ghost* hits. For example the  hits of two different particles on a single sensor, will usually result in 2 u-side `SVDCluster` and 2 v-side `SVDCluster` which will cross each other in 4 distinct points on the sensor. Only two of those are the positions of the true hits while the other two are *ghost* hits. 

Due to the pixelated nature of the PXD the measurements of the PXD already present a 2D positional measurement of a hit. Therefor PXD hits can be directly translated into a Space Point by performing a coordinate transformation. 

Note: That both  (SVD- and PXD-) Space Point creation has been moved to the corresponding software packages, and is not part of the tracking package anymore. Thus the module for the creation of Space Points from `SVDCluster`, the `SVDSpacePointCreator`, can be found in the svd package. And the module to create Space Points from :cpp:class:`PXDCluster`, the `PXDSpacePointCreator`, can be found in the pxd package.  
