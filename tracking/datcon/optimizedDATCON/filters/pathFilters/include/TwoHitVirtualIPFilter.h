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
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <math.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class TwoHitVirtualIPFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    QualityEstimatorRiemannHelixFit helixFitEstimator;
    /// cut on the POCA distance in xy obtained from the helixFitEstimator
    double m_helixFitPocaVirtIPDCut = 1.0;
  };
}
