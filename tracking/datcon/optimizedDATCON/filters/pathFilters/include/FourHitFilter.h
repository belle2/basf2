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
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <math.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class FourHitFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// TODO: Cut on relations in theta for overlay region on same layer but different ladder
    double m_param_CircleRadiusDifferenceCut = 10.;
    /// TODO: Filter relations in theta between hit states where the layer difference is +-1
    double m_param_CircleCenterPositionDifferenceCut = 10.;

    /// cut on the POCA distance in xy obtained from the helixFitEstimator
    double m_helixFitPocaDCut = 1.0;

    /// Get track quality estimate from a circle fit
    QualityEstimatorCircleFit circleFitEstimator;
    /// Get track quality estimate from a helix fit
    QualityEstimatorRiemannHelixFit helixFitEstimator;
    /// Get track quality estimate from a triplet fit
    QualityEstimatorTripletFit tripletFitEstimator;
  };
}
