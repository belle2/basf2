/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

using namespace std;
using namespace Belle2;

ClassImp(SpacePoint)

SpacePoint::SpacePoint(const PXDCluster& pxdCluster, unsigned int indexNumber, const VXD::SensorInfoBase* aSensorInfo) :
  m_vxdID(pxdCluster.getSensorID())
{
  m_indexNumbers.push_back(indexNumber);

  // missing, detector type import... (distinguishing between TELescope and PXD Hits)

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  m_position = aSensorInfo->pointToGlobal(
                 TVector3(
                   pxdCluster.getU(),
                   pxdCluster.getV(),
                   0
                 )
               );

  //As only variances, but not the sigmas transform linearly,
  // we need to use some acrobatics
  // (and some more (abs) since we do not really transform a vector).
  TVector3 globalizedVariances = aSensorInfo->vectorToGlobal(
                                   TVector3(
                                     pxdCluster.getUSigma() * pxdCluster.getUSigma(),
                                     pxdCluster.getVSigma() * pxdCluster.getVSigma(),
                                     0
                                   )
                                 );
  for (int i = 0; i < 3; i++) {
    m_positionError[i] = sqrt(abs(globalizedVariances[i]));
  }

  //As the 0 is in the middle of sensor in the geometry, and we want
  // to normalize all positions to numbers between [0,1],
  // where the middle will be 0.5,
  // we need to do some calculation.
  float halfSensorSizeU = 0.5 *  aSensorInfo->getUSize();
  float halfSensorSizeV = 0.5 *  aSensorInfo->getVSize();
  float localUPosition = pxdCluster.getU() + halfSensorSizeU;
  float localVPosition = pxdCluster.getV() + halfSensorSizeV;
  m_normalizedLocal[0] = localUPosition / aSensorInfo->getUSize();
  m_normalizedLocal[1] = localVPosition / aSensorInfo->getVSize();
}