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

    /// Function doing the main work: extrapolate a given mSoP to the given plane
    bool extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const genfit::SharedPlanePtr& plane) const;
  private:
    /// Parameter: use material effects during extrapolation.
    bool m_param_useMaterialEffects = true;
  };

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
