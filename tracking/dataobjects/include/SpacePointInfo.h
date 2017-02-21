/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TVector3.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  // helper class to store the SpacePoint information as coding convention prohibits to use the SpacePoint class here
  class SpacePointInfo : public TObject {
  public:
    SpacePointInfo() :
      m_position( {0., 0., 0.}),
                m_positionError({1., 1., 1.}),
                m_normalizedLocalU(0.),
                m_normalizedLocalV(0.),
                m_clustersAssignedU(false),
                m_clustersAssignedV(false),
                m_vxdID(Belle2::VxdID::baseType(0)),
                m_sensorType(-1),
                m_qualityIndicator(0.5),
                m_isAssigned(false)
    {
    };

    ~SpacePointInfo() {};

    void setPosition(TVector3 v) { m_position = v; };
    double getX() { return m_position.X(); };
    double getY() { return m_position.Y(); };
    double getZ() { return m_position.Z(); };

    void setPositionError(TVector3 v) { m_positionError = v; };
    double getErrorX() { return m_positionError.X(); };
    double getErrorY() { return m_positionError.Y(); };
    double getErrorZ() { return m_positionError.Z(); };

    void setNormalizedLocalU(double val) { m_normalizedLocalU = val; };
    double getNormalizedLocalU() { return m_normalizedLocalU; };

    void setNormalizedLocalV(double val) { m_normalizedLocalV = val; };
    double getNormalizedLocalV() { return m_normalizedLocalV; };

    bool getClustersAssignedU() { return m_clustersAssignedU;};
    void setClustersAssignedU(bool b) { m_clustersAssignedU = b; };

    bool getClustersAssignedV() { return m_clustersAssignedV;};
    void setClustersAssignedV(bool b) { m_clustersAssignedV = b; };

    Belle2::VxdID::baseType getVxdID() { return m_vxdID; };
    void setVxdID(Belle2::VxdID::baseType anId) { m_vxdID = anId; };

    int getSensorType() { return m_sensorType;};
    void setSensorType(int type) { m_sensorType = type;};

    double getQualityIndicator() { return m_qualityIndicator;};
    void setQualityIndicator(double qi) { m_qualityIndicator = qi; };

    bool getIsAssigned() { return m_isAssigned; };
    void setIsAssigned(bool ia) {  m_isAssigned = ia; };


  private:
    TVector3 m_position;
    TVector3 m_positionError;
    // normalized coordinates of spacepoint in local coordinates .first is u , .second is v -direction
    double m_normalizedLocalU;
    double m_normalizedLocalV;
    // .first if SP has a u cluster, .second if SP has a v-cluster assigned
    bool m_clustersAssignedU;
    bool m_clustersAssignedV;
    // used the baseType instead of VxdID as root seems to have problems with it
    Belle2::VxdID::baseType m_vxdID;
    int m_sensorType;
    double m_qualityIndicator;
    bool m_isAssigned;

    ClassDef(SpacePointInfo, 1);
  };
}
