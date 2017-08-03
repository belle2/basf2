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

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/MaterialEffects.h>
#include <genfit/Exception.h>

namespace Belle2 {
  class SpacePoint;
  class SVDCluster;
  class PXDCluster;

  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }

  /**
   * Algorithm class to extrapolate a state onto the plane of its related space point.
   */
  class AdvanceAlgorithm : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// General extrapolate function for a templated reco hit. We get the plane of this hit and extrapolate to this plane.
    template <class ARecoHit>
    bool extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
    {
      const genfit::SharedPlanePtr& plane = getPlane(measuredStateOnPlane, recoHit);
      return extrapolateToPlane(measuredStateOnPlane, plane);
    }

    /// General helper function for a templated reco hit, to return the plane this reco hit is on
    template <class ARecoHit>
    genfit::SharedPlanePtr getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    /// Expose the useMaterialEffects parameter.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Main function: extrapolate the state to its related space point. Returns NAN, if the extrapolation fails.
    template <class AStateObject>
    TrackFindingCDC::Weight operator()(AStateObject& currentState) const;

  private:
    /// Parameter: use material effects during extrapolation.
    bool m_param_useMaterialEffects = true;

    /// Function doing the main work: extrapolate a given mSoP to the given plane
    bool extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const genfit::SharedPlanePtr& plane) const;
  };

  template <class AStateObject>
  TrackFindingCDC::Weight AdvanceAlgorithm::operator()(AStateObject& currentState) const
  {
    B2ASSERT("Encountered invalid state", not currentState.isFitted() and not currentState.isAdvanced());

    const auto* hit = currentState.getHit();

    if (not hit) {
      // If we do not have a hit, we do not need to do anything here.
      currentState.setAdvanced();
      return 1;
    }

    // Get the parent to check, if we have already seem it and have something in our cache
    B2ASSERT("How could a state without a parent end up here?", currentState.getParent());

    // Normally, we would just use the (updated) mSoP of the parent and extrapolate it to the
    // plane, this state/hit is located on. So we start with a copy of the mSoP of the parent here ...
    genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlane();
    // ... however, to speed things up, we first check if the plane of the cached mSoP of the parent ...
    const genfit::MeasuredStateOnPlane& cachedMeasuredStateOnPlane = currentState.getCachedMeasuredStateOnPlaneOfParent();
    // ... and our own plane is the same.
    const genfit::SharedPlanePtr& plane = getPlane(measuredStateOnPlane, *hit);
    // If the two normals are equal, we have already calculated this! so we can just reuse the cached mSoP
    if (plane->getNormal() == cachedMeasuredStateOnPlane.getPlane()->getNormal()) {
      // we do not have to update the cached mSoP in this case (would not change anything anyway)
      currentState.setMeasuredStateOnPlane(cachedMeasuredStateOnPlane);
      currentState.setAdvanced();
      return 1;
    }

    // this means the two are not equal, so we have to do the extrapolation. We start with the mSoP of the
    // parent state.
    if (not extrapolateToPlane(measuredStateOnPlane, plane)) {
      return NAN;
    }

    // we update the cache for the next hit - anticipating we will visit this sensor plane again in the next round
    currentState.setMeasuredStateOnPlane(measuredStateOnPlane, true);
    currentState.setAdvanced();

    return 1;
  }

  template <class ARecoHit>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
  {
    return recoHit.constructPlane(measuredStateOnPlane);
  }
  template <>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                    const SpacePoint& spacePoint) const;
  template <>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, SVDCluster& svdCluster) const;
  template <>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, PXDCluster& pxdCluster) const;
  template <>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                    const TrackFindingCDC::CDCRLWireHit& rlWireHit) const;
  template <>
  genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                    const genfit::MeasuredStateOnPlane& plane) const;
}
