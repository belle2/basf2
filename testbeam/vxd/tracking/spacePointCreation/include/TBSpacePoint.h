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

#include <tracking/spacePointCreation/SpacePoint.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>

// stl:
#include <vector>

namespace Belle2 {
  /** The SpacePoint class for testBeams.
   *
   * This class stores a global space point with its position error and some extra infos.
   * In addition to its base class (SpacePoint), it supports TelClusters too.
   * INFO: it should not be stored in a storeArray<TBSpacePoint> , but in a storeArray<SpacePoint>,
   * since their only difference is another constructor for the TB-version.
   * This allows using TBSpacePoints in the normal trackFinder without dependencies from the testbeam package.
   */
  class TBSpacePoint: public SpacePoint {
  public:

    /** Default constructor for the ROOT IO. */
    TBSpacePoint()
    {}



    /** destructor to prevent undefined behavior for inherited classes */
    ~TBSpacePoint() {}


    /** Constructor for the case of TEL Hits.
     *
     * first parameter is pointer to cluster (passing a null-pointer will throw an exception)
     * second is a pointer to a sensorInfo, which can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     */
    TBSpacePoint(const Belle2::TelCluster* telCluster,
                 const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** Constructor for the case of PXD Hits.
     *
     * first parameter is pointer to cluster (passing a null-pointer will throw an exception)
    * second is a pointer to a sensorInfo, which can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     *
     * Loops through to the constructor of the base class
     */
    TBSpacePoint(const Belle2::PXDCluster* pxdCluster,
                 const VXD::SensorInfoBase* aSensorInfo = NULL) :
      SpacePoint(pxdCluster, aSensorInfo)
    {}



    /** Constructor for the case of SVD Hits.
     *
     * 1-2 clusters can be added this way
     *
     * first parameter is a container carrying pointers to the svdClusters,
     * It should _not_ be filled with NULL-Pointers (passing a null-pointer will throw an exception).
     * 1 - 2 Clusters are allowed that way, if there are passed more than that or less, an exception will be thrown.
     * second parameter, a sensorInfo can be passed for testing purposes.
     * If no sensorInfo is passed, the constructor gets its own pointer to it.
     *
     */
    TBSpacePoint(std::vector<const Belle2::SVDCluster*>& clusters,
                 const VXD::SensorInfoBase* aSensorInfo = NULL) :
      SpacePoint(clusters, aSensorInfo)
    {}



    /** returns a vector of genfit::PlanarMeasurement, which is needed for genfit::track.
     *
     * This member ensures compatibility with genfit2.
     * The return type is detector independent,
     * but each entry will be of the same detector type,
     * since a spacePoint can not contain clusters of different sensors
     * and therefore of different detector types.
     *
     * This is the overloaded version of SpacePoint::getGenfitCompatible(), now supporting TelClusters.
     */
    std::vector<genfit::PlanarMeasurement> getGenfitCompatible() const ;



  protected:

    ClassDef(TBSpacePoint, 2) // last stuff added: telCluster-constructor;
  };
}
