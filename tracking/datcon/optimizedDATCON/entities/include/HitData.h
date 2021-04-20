/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Wessel                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  class SpacePoint;

  /// Simple container for hit information to be used during intercept finding

  class HitData {
  public:
    /// Default constructor
    HitData() = default;
    /// Move constructor needed for STL containers
    HitData(HitData&&) = default;
    /// No copy constructor
    HitData(const HitData&) = delete;
    /// No copy constructor
    HitData& operator=(const HitData&) = delete;
    /// Move assignment constructor
    HitData& operator=(HitData&&) = default;
    /// Destructor
    ~HitData() = default;

    /// Initialize the state as non-root with a related hit (and with a seed)
    explicit HitData(const SpacePoint* hit, const B2Vector3D& BeamSpotPosition) : m_hit(hit)
    {
      m_dataCache.sensorID = hit->getVxdID();
      m_dataCache.layer = hit->getVxdID().getLayerNumber();
      m_dataCache.ladder = hit->getVxdID().getLadderNumber();
      const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(hit->getVxdID());
      m_dataCache.x = hit->X();
      m_dataCache.y = hit->Y();
      m_dataCache.z = hit->Z();
      const double conformalTransform = 2. / ((hit->X() - BeamSpotPosition.X()) * (hit->X() - BeamSpotPosition.X()) +
                                              (hit->Y() - BeamSpotPosition.Y()) * (hit->Y() - BeamSpotPosition.Y()));
      m_dataCache.xConformal = hit->X() * conformalTransform;
      m_dataCache.yConformal = hit->Y() * conformalTransform;
      m_dataCache.localNormalizedu = hit->getNormalizedLocalU();
      m_dataCache.localNormalizedv = hit->getNormalizedLocalV();
      m_dataCache.sensorCenterPhi = sensorInfo.pointToGlobal(TVector3(0., 0., 0.), true).Phi();
      m_dataCache.phi = hit->getPosition().Phi();
      m_dataCache.theta = hit->getPosition().Theta();
      m_dataCache.uTime = hit->TimeU();
      m_dataCache.vTime = hit->TimeV();
    }

    /// Return the SP this state is related to. May be nullptr.
    const SpacePoint* getHit() const { return m_hit; }

    /// Getter for the automaton cell.
    TrackFindingCDC::AutomatonCell& getAutomatonCell() { return m_automatonCell; }


    /// Cache containing the most important information of this state which will often be needed
    struct DataCache {
      /// SensorID of this hit
      VxdID sensorID = VxdID(0);
      /// Geometrical Layer this state is based on
      unsigned short layer = std::numeric_limits<unsigned short>::quiet_NaN();
      /// Ladder this state is based on (only use for SpacePoint based states)
      unsigned short ladder = std::numeric_limits<unsigned short>::quiet_NaN();
      /// x coordinate of this hit
      double x = std::numeric_limits<double>::quiet_NaN();
      /// y coordinate of this hit
      double y = std::numeric_limits<double>::quiet_NaN();
      /// z coordinate of this hit
      double z = std::numeric_limits<double>::quiet_NaN();
      /// conformal transformed x coordinate of this hit
      double xConformal = std::numeric_limits<double>::quiet_NaN();
      /// conformal transformed y coordinate of this hit
      double yConformal = std::numeric_limits<double>::quiet_NaN();
      /// Local normalized uCoordinate of this state, only set if based on SpacePoint
      double localNormalizedu = std::numeric_limits<double>::quiet_NaN();
      /// Local normalized vCoordinate of this state, only set if based on SpacePoint
      double localNormalizedv = std::numeric_limits<double>::quiet_NaN();
      /// Phi value of SpacePoint
      double sensorCenterPhi = std::numeric_limits<double>::quiet_NaN();
      /// Phi value of SpacePoint
      double phi = std::numeric_limits<double>::quiet_NaN();
      /// Theta value of SpacePoint
      double theta = std::numeric_limits<double>::quiet_NaN();
      /// Time of the u-side cluster
      double uTime = std::numeric_limits<double>::quiet_NaN();
      /// Time of the v-side cluster
      double vTime = std::numeric_limits<double>::quiet_NaN();
    };
    /// Get the cached data of this state
    const DataCache getDataCache() const { return m_dataCache; }

  protected:
    /// Cache the most important data of this state for better runtime performance
    DataCache m_dataCache;

  private:
    /// Pointer to hit
    const SpacePoint* m_hit = nullptr;
    /// Memory for the automaton cell.
    TrackFindingCDC::AutomatonCell m_automatonCell;
  };

}
