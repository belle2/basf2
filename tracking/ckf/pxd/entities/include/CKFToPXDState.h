/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFState.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <genfit/SharedPlanePtr.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class PXDRecoHit;

  /// Specialized CKF State for extrapolating into the PXD
  class CKFToPXDState : public CKFState<RecoTrack, SpacePoint> {
  public:
    /// Cache containing the most important information of this state which will often be needed
    struct stateCache {
      /// Is this state based on a hit (=SpacePoint) or a seed (=RecoTrack)
      bool isHitState = false;
      /// VxdID of this state, only set if based on SpacePoint
      VxdID sensorID = VxdID(0);
      /// Geometrical Layer this state is based on. Will be set to the SVD layer number if it's a state based on a SpacePoint, to an artificial layer else by using getGeometricalLayer()
      unsigned short geoLayer = std::numeric_limits<unsigned short>::quiet_NaN();
      /// Ladder this state is based on (only use for SpacePoint based states)
      unsigned short ladder = std::numeric_limits<unsigned short>::quiet_NaN();
      /// Phi of the sensor this state is based on, only set if based on SpacePoint
      double sensorCenterPhi = std::numeric_limits<double>::quiet_NaN();
      /// Phi if this state (from SpacePoint or RecoTrack)
      double phi = std::numeric_limits<double>::quiet_NaN();
      /// Theta if this state (from SpacePoint or RecoTrack)
      double theta = std::numeric_limits<double>::quiet_NaN();
      /// Local normalized uCoordinate of this state, only set if based on SpacePoint
      double localNormalizedu = std::numeric_limits<double>::quiet_NaN();
      /// Local normalized vCoordinate of this state, only set if based on SpacePoint
      double localNormalizedv = std::numeric_limits<double>::quiet_NaN();
    };
    /// When constructed by a hit, set the reco hit
    explicit CKFToPXDState(const SpacePoint* hit);

    /// Constructor setting the state to the position of the first CDC track seed hit
    explicit CKFToPXDState(const RecoTrack* seed, bool reversed = false);

    /// Extract the real layer this state sits on
    unsigned int getGeometricalLayer() const;

    /// Return the plane of the first PXD cluster
    genfit::SharedPlanePtr getPlane(const genfit::MeasuredStateOnPlane& state) const;

    /// Helper function for getting the already created reco hit (runtime reasons)
    const PXDRecoHit& getRecoHit() const;

    /// Helper function for getting the already created reco hits (runtime reasons)
    const std::vector<PXDRecoHit>& getRecoHits() const;

    /// Get the cached data of this state
    const struct stateCache& getStateCache() const;

  private:
    /// Precache the PXDRecoHits for runtime performance reasons.
    std::vector<PXDRecoHit> m_recoHits;

    /// Cache the most important data of this state for better runtime performance
    stateCache m_stateCache;
  };
}
