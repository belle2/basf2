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

SpacePoint::SpacePoint(const PXDCluster& pxdCluster, unsigned int indexNumber) :
  m_vxdID(pxdCluster.getSensorID())
{
  m_indexNumbers.push_back(indexNumber);

  VxdID myID = VxdID(m_vxdID); // missing, detector type import...

  //We need some handle to translate IDs to local and global
  // coordinates.
  const VXD::SensorInfoBase& sensorInfoBase =
    VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);

  m_position = sensorInfoBase.pointToGlobal(
                 TVector3(
                   pxdCluster.getU(),
                   pxdCluster.getV(),
                   0
                 )
               );

  //As only variances, but not the sigmas transform linearly
  // we need to use some acrobatics.
  TVector3 globalizedVariances = sensorInfoBase.vectorToGlobal(
                                   TVector3(
                                     pxdCluster.getUSigma() * pxdCluster.getUSigma(),
                                     pxdCluster.getVSigma() * pxdCluster.getVSigma(),
                                     0
                                   )
                                 );
  for (int i = 0; i < 3; i++) {
    m_positionError[i] = sqrt(globalizedVariances[i]);
  }

  //As the 0 is in the middle of sensor in the geometry, and we want
  // to normalize all positions to numbers between [0,1],
  // we need to do some calculation.
  float halfSensorSizeU = 0.5 *  sensorInfoBase.getUSize();
  float halfSensorSizeV = 0.5 *  sensorInfoBase.getVSize();
  float localUPosition = pxdCluster.getU() + halfSensorSizeU;
  float localVPosition = pxdCluster.getV() + halfSensorSizeV;
  m_normalizedLocal[0] = localUPosition / sensorInfoBase.getUSize();
  m_normalizedLocal[1] = localVPosition / sensorInfoBase.getVSize();
}