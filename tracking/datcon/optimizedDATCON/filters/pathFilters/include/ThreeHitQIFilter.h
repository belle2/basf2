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

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <math.h>

namespace Belle2 {
  /// Filter for three hits using QualityEstimators.
  class ThreeHitQIFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// initialize quality estimator
    void initialize() override;
    /// set BField value for estimator
    void beginRun() override;

  private:
    /// Identifier which estimation method to use. Valid identifiers are:
    /// mcInfo, circleFit, tripletFit, helixFit
    std::string m_EstimationMethod = "helixFit";
    /// pointer to the selected QualityEstimator
    std::unique_ptr<QualityEstimatorBase> m_estimator;
  };
}
