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
  /// helper class to store the SpacePoint information as coding convention prohibits to use the SpacePoint class here
  class SpacePointInfo : public TObject {
  public:
    /// constructor
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

    /// destructor
    ~SpacePointInfo() {};

    /** setter for the position
     * @param v: 3-vector with new position
    */
    void setPosition(TVector3 v) { m_position = v; };

    /// getter for the x-position
    double getX() { return m_position.X(); };

    /// getter for the y-position
    double getY() { return m_position.Y(); };

    /// getter for the y-position
    double getZ() { return m_position.Z(); };

    /** setter for the uncertainty on the position
     * @param v: vector with new position uncertainties
    */
    void setPositionError(TVector3 v) { m_positionError = v; };

    /// getter for uncertainty on x
    double getErrorX() { return m_positionError.X(); };

    /// getter for uncertainty on y
    double getErrorY() { return m_positionError.Y(); };

    /// getter for uncertainty on z
    double getErrorZ() { return m_positionError.Z(); };

    /** setter for the normalized u coordinate
     * @param val: new value for normalized u coordinate
    */
    void setNormalizedLocalU(double val) { m_normalizedLocalU = val; };

    /// getter function for normalized u coordinate
    double getNormalizedLocalU() { return m_normalizedLocalU; };

    /** setter for the normalized v coordinate
     * @param val: new value for normalized v coordinate
    */
    void setNormalizedLocalV(double val) { m_normalizedLocalV = val; };

    /// getter function for normalized v coordinate
    double getNormalizedLocalV() { return m_normalizedLocalV; };

    /// returns true if u cluster is assigned
    bool getClustersAssignedU() { return m_clustersAssignedU;};

    /** setter for is u cluster assigned
     * @param b: new value
    */
    void setClustersAssignedU(bool b) { m_clustersAssignedU = b; };

    /// returns true if u cluster is assigned
    bool getClustersAssignedV() { return m_clustersAssignedV;};

    /** setter for is v cluster assigned
     * @param b: new value
    */
    void setClustersAssignedV(bool b) { m_clustersAssignedV = b; };

    /// getter for the VxdID
    Belle2::VxdID::baseType getVxdID() { return m_vxdID; };

    /** setter for the VxdID:
     * @param anID: new vxdid
    */
    void setVxdID(Belle2::VxdID::baseType anId) { m_vxdID = anId; };

    /// getter for sensor type
    int getSensorType() { return m_sensorType;};

    /** setter for sensor type:
     * @param type: the new sensor type
    */
    void setSensorType(int type) { m_sensorType = type;};

    /// getter for the quality indicator
    double getQualityIndicator() { return m_qualityIndicator;};

    /** setter for the quality indicator
     * @param qi: new quality indicator
    */
    void setQualityIndicator(double qi) { m_qualityIndicator = qi; };

    /// getter for is assigned
    bool getIsAssigned() { return m_isAssigned; };

    /** setter for is assigned.
     * @param ia: new value for isAssigned
    */
    void setIsAssigned(bool ia) {  m_isAssigned = ia; };


  private:
    /// 3-vector with positon in global coordinates
    TVector3 m_position;

    /// uncertainty on the position
    TVector3 m_positionError;

    /// normalized coordinates of spacepoint in local coordinates u-direction
    double m_normalizedLocalU;

    /// normalized coordinates of spacepoint in local coordinates v -direction
    double m_normalizedLocalV;

    /// true if SP has a u-cluster assigned
    bool m_clustersAssignedU;

    /// true if SP has a v-cluster assigned
    bool m_clustersAssignedV;

    /// used the baseType instead of VxdID as root seems to have problems with it
    Belle2::VxdID::baseType m_vxdID;

    /// the sensor type
    int m_sensorType;

    /// the quality indicator for that SP
    double m_qualityIndicator;

    /// true if is assigned
    bool m_isAssigned;

    //! needed by root
    ClassDef(SpacePointInfo, 1);
  };
}
