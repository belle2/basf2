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
#include <utility> // std::pair
#include <math.h>

namespace Belle2 {
  /** The SpacePoint class for testBeams.
   *
   * This class stores a global space point with its position error and some extra infos.
   * In addition to its base class (SpacePoint), it supports TelClusters too.
   */
  class TBSpacePoint: public SpacePoint {
  public:

    /** Default constructor for the ROOT IO. */
    TBSpacePoint()
    {}



    /** Constructor for the case of TEL Hits.
     *
     * first parameter is pointer to cluster (passing a null-pointer will throw an exception)
     * second is the index number of the cluster in its storeArray.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     */
    TBSpacePoint(const TelCluster* telCluster,
                 unsigned int indexNumber,
                 const VXD::SensorInfoBase* aSensorInfo = NULL);



  protected:

    ClassDef(TBSpacePoint, 0) // last stuff added: telCluster-constructor;
  };
}
