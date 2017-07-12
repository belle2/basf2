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

  /**
   * Algorithm class to extrapolate a state onto the plane of its related space point.
   */
  class AdvanceAlgorithm : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// General extrapolate function for a templated reco hit. Further down are a few implementations.
    template <class ARecoHit>
    bool extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const;

    /// Expose the useMaterialEffects parameter.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Main function: extrapolate the state to its related space point. Returns NAN, if the extrapolation fails.
    template <class AStateObject>
    TrackFindingCDC::Weight operator()(AStateObject& currentState) const;

  private:
    /// Parameter: use material effects during extrapolation.
    bool m_param_useMaterialEffects = true;
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

    // This is the mSoP we will edit.
    genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlane();

    if (not extrapolate(measuredStateOnPlane, *hit)) {
      return NAN;
    }

    currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
    currentState.setAdvanced();
    return 1;
  }

  template <class ARecoHit>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, ARecoHit& recoHit) const
  {
    // The mSoP plays no role here (it is unused in the function)
    const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);
    return extrapolate(measuredStateOnPlane, plane);
  }

  template <>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint& spacePoint) const;
  template <>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, SVDCluster& svdCluster) const;
  template <>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, PXDCluster& pxdCluster) const;
  template <>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const genfit::SharedPlanePtr& plane) const;
  template <>
  bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                     const genfit::MeasuredStateOnPlane& plane) const;
}
