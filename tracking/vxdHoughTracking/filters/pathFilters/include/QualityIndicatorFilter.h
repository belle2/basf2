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

#include <tracking/vxdHoughTracking/filters/pathFilters/BasePathFilter.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Filter for three hits using QualityEstimators.
    class QualityIndicatorFilter : public BasePathFilter {
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
      std::string m_param_EstimationMethod = "tripletFit";
      /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
      std::string m_param_MCRecoTracksStoreArrayName = "MCRecoTracks";
      /// Only required for MCInfo method
      bool m_param_MCStrictQualityEstimator = true;
      /// pointer to the selected QualityEstimator
      std::unique_ptr<QualityEstimatorBase> m_estimator;

      /// cut on quality indicator
      double m_param_QIcut = 0.0;
    };

  }
}
