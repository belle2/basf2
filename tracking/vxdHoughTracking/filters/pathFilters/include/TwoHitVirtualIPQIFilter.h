/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/pathFilters/BasePathFilter.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Filter for two hits plus a virtual IP using QualityEstimators.
    class TwoHitVirtualIPQIFilter : public BasePathFilter {
    public:
      /// Return the weight based on the quality estimator.
      /// Returns the QI value obtained from the fit with the chosen QualityEstimator
      TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// initialize quality estimator
      void initialize() override;
      /// set BField value for estimator
      void beginRun() override;

    private:
      /// virtual IP SpacePoint
      SpacePoint m_virtualIPSpacePoint;

      /// Identifier which estimation methsod to use. Valid identifiers are:
      /// mcInfo, tripletFit, helixFit
      std::string m_param_EstimationMethod = "helixFit";
      /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
      std::string m_param_MCRecoTracksStoreArrayName = "MCRecoTracks";
      /// Only required for MCInfo method
      bool m_param_MCStrictQualityEstimator = true;
      /// pointer to the selected QualityEstimator
      std::unique_ptr<QualityEstimatorBase> m_estimator;
    };

  }
}
