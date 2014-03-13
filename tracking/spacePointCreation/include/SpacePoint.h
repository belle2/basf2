/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// framework
#include <framework/datastore/RelationsObject.h>
// vxd
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
// pxd
#include <pxd/dataobjects/PXDCluster.h>
//svd
#include <svd/dataobjects/SVDCluster.h>

// stl:
#include <vector>
#include <math.h>

namespace Belle2 {
  /** The SpacePoint class.
   *
   *  This class stores a global space point with its position error and some extra infos
   */
  class SpacePoint: public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    SpacePoint()
    {}

    /** Constructor for the case of PXD or TELHits (both are PXDClusters).
     *
     * first parameter is reference to cluster
     * second is the index number of the cluster in its storeArray.
     * If there is a sensorInfo at hand, it can be passed as a third parameter
     *  (if no sensorInfo is passed, it will be created within the constructor)
     */
    SpacePoint(const PXDCluster& pxdCluster, unsigned int indexNumber, const VXD::SensorInfoBase* aSensorInfo = NULL);


// getter:

    /** return the position vector in global coordinates */
    const TVector3& getPosition() const { return m_position; }

    /** return the hitErrors in sigma of the global position */
    const TVector3& getPositionError() const { return m_positionError; }

    /** return the VxdID of the sensor inhabiting the Cluster of the SpacePoint */
    VxdID::baseType getVxdID() const { return m_vxdID; }

    /** return the normalized local coordinates of the cluster in u (0 <= posU <= 1) */
    float getNormalizedLocalU() const { return m_normalizedLocal[0]; }

    /** return the normalized local coordinates of the cluster in v (0 <= posV <= 1) */
    float getNormalizedLocalV() const { return m_normalizedLocal[1]; }


  protected:
    /** Global position vector.
     *
     *  [0]: x , [1] : y, [2] : z
     */
    TVector3 m_position;

    /** Error "Vector" of global position in sigma.
     *
     *  [0]: x-uncertainty , [1] : y-uncertainty, [2] : z-uncertainty
     */
    TVector3 m_positionError;

    /** Position in local coordinates normalized to the sensor size between 0 and 1.
     *
     *  First entry is u, second is v
     */
    float m_normalizedLocal[2];

    /** stores the vxdID */
    VxdID::baseType m_vxdID;

    /** carries the index numbers of the linked clusters.
     *
     * These index numbers are for the storeArray of svdClusters, if the detector type is Const::SVD, If it is Const::PXD, its for the pxdCluster container, and for Const::Test it is for a pxdCluster-container carrying all the telescope hits */
    std::vector<unsigned int> m_indexNumbers;
    ClassDef(SpacePoint, 1)
  };
}
