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

//stl
#include <vector>


namespace Belle2 {
  /** The SpacePoint class.
   *
   *  This class stores a global space point with its position error and some extra infos
   */
  class SpacePoint: public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    SpacePoint() //:
//      m_normalizedLocal[0](0),
//      m_normalizedLocal[1](0)
    {}

    /** Default constructor for the ROOT IO. */
    SpacePoint(const PXDCluster& pxdCluster) :
//      m_normalizedLocal[0](0),
//      m_normalizedLocal[1](0)
      m_vxdID(pxdCluster.getSensorID()) {
      const VXD::SensorInfoBase& sensorInfoBase =
        VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);

      m_position = sensorInfoBase.pointToGlobal(
                     TVector3(pxdCluster.getU(), pxdCluster.getV(), 0));

      float halfSensorSizeU = 0.5 *  sensorInfoBase.getUSize();
      float halfSensorSizeV = 0.5 *  sensorInfoBase.getVSize();
      float localUPosition = pxdCluster.getU() + halfSensorSizeU;
      float localVPosition = pxdCluster.getV() + halfSensorSizeV;

      m_normalizedLocal[0] = localUPosition / sensorInfoBase.getUSize();
      m_normalizedLocal[1] = localVPosition / sensorInfoBase.getVSize();

      m_positionError = sensorInfoBase.vectorToGlobal(
                          TVector3(pxdCluster.getUSigma(), pxdCluster.getVSigma(), 0));
    }


  protected:
//  {
//    float halfSensorSizeU = 0.5 *  aSensorInfo.getUSize();
//        float halfSensorSizeV = 0.5 *  aSensorInfo.getVSize(pxdCluster.getU());
//        float localUPosition = pxdCluster.getU() + halfSensorSizeU;
//        float localVPosition = pxdCluster.getV() + halfSensorSizeV;
//  }

    /** position vector [0]: x , [1] : y, [2] : z */
    TVector3 m_position;

    /** error vector [0]: x , [1] : y, [2] : z */
    TVector3 m_positionError;

    /** the position in local coordinates defined between 0 and 1, first entry is u, second is v*/
    float m_normalizedLocal[2];

    /** stores the vxdID */
    VxdID::baseType m_vxdID;

    ClassDef(SpacePoint, 1)
  };
}
