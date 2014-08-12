/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

using namespace std;
using namespace Belle2;

ClassImp(TBSpacePoint)

TBSpacePoint::TBSpacePoint(const TelCluster* telCluster, unsigned int indexNumber, const VXD::SensorInfoBase* aSensorInfo)
{
  SpacePoint::m_vxdID = telCluster->getSensorID();
  if (telCluster == NULL) { throw SpacePoint::InvalidNumberOfClusters(); }
  SpacePoint::m_indexNumbers.push_back(indexNumber);

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(SpacePoint::m_vxdID);
  }

  SpacePoint::m_position = aSensorInfo->pointToGlobal(
                             TVector3(
                               telCluster->getU(),
                               telCluster->getV(),
                               0
                             )
                           );

  SpacePoint::setPositionError(telCluster->getUSigma(), telCluster->getVSigma(), aSensorInfo);

  SpacePoint::m_normalizedLocal = SpacePoint::convertLocalToNormalizedCoordinates({ telCluster->getU(), telCluster->getV() } , SpacePoint::m_vxdID, aSensorInfo);

  SpacePoint::m_sensorType = aSensorInfo->getType();
}
