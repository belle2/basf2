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

#include <tracking/datcon/optimizedDATCON/filters/pathFilters/BasePathFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FourHitVariables.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <math.h>

namespace Belle2 {
  /// Filter for four hits.
  /// Basic working principle: use FourHitVariables and ThreeHitVariables and provide four (three B2Vector3D) to each variable.
  /// These are oHit (outer hit), coHit (middle outer hit), ciHit (middle inner hit) and iHit (inner hit) and then calculate
  /// the variables specified in FourHitVariables and ThreeHitVariables for the four positions
  class FourHitFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// set BField value for estimator
    void beginRun() override;

  private:
    /// Cut on difference of the two circle radii estimated from two triplets
    double m_param_CircleRadiusDifferenceCut = 10.;
    /// Cut on difference of the two circle center estimated from two triplets
    double m_param_CircleCenterPositionDifferenceCut = 10.;

    /// Construct empty ThreeHitVariables instance
    FourHitVariables m_fourHitVariables;
  };
}
