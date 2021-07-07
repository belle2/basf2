/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/FullSecID.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <vector>
#include <utility> // std::pair


namespace Belle2 {

  /** simple Hit class used for sectorMap-training. */
  class SecMapTrainerHit: public SpacePoint {
  public:
    /** Constructor of class SecMapTrainerHit. */
    SecMapTrainerHit(const FullSecID& fullSecID, const SpacePoint& sp):
      SpacePoint(sp),
      m_secID(fullSecID) {}

    /** constructor needed for the Virtual IP */
    SecMapTrainerHit(const FullSecID& fullSecID, const B2Vector3<double>& position):
      SpacePoint(position, B2Vector3<double> (0., 0., 0.),
                 std::pair<double, double> (0., 0.),
                 std::pair<bool, bool>() , VxdID(),
                 Belle2::VXD::SensorInfoBase::SensorType()),
      m_secID(fullSecID) {}

    /** another constructor needed for the Virtual IP... eccpp*/
    SecMapTrainerHit(double x, double y, double z):
      SpacePoint(B2Vector3<double> (x, y, z),
                 B2Vector3<double> (0., 0., 0.),
                 std::pair<double, double> (0., 0.),
                 std::pair<bool, bool>() , VxdID(),
                 Belle2::VXD::SensorInfoBase::SensorType()),
      m_secID() {}

    /** returns global hit position of current hit. */
    //    B2Vector3D getHitPosition() const { return m_hitPos; }

    /** returns global x-position. */
    // double X() const { return m_hitPos.X(); }

    /** returns global y-position. */
    // double Y() const { return m_hitPos.Y(); }

    /** returns global z-position. */
    // double Z() const { return m_hitPos.Z(); }

    /** returns secID of sector containing this hit. */
    std::string getSectorIDString() const { return m_secID.getFullSecString(); }

    /** returns secID of sector containing this hit. */
    FullSecID getSectorID() const { return m_secID; }

  protected:
    FullSecID m_secID; /**< ID of sector containing this hit. */
    //    B2Vector3D m_hitPos; /**< global hit position. */
  };
}

