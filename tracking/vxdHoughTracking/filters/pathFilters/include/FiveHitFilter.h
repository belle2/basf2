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

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Basic working principle: use ThreeHitVariables and provide three B2Vector3D to each variable.
    /// These are oHit (outer hit), cHit (middle hit), and iHit (inner hit) and then calculate
    /// the variables specified in ThreeHitVariables for the three positions, often using the difference
    /// (oHit - cHit) and (cHit - iHit).
    class FiveHitFilter : public BasePathFilter {
    public:
      /// Return the weight based on azimuthal-angle separation
      TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// set BField value for estimator
      void beginRun() override;

    private:
      /// cut on the POCA distance in xy obtained from the helixFitEstimator
      double m_param_helixFitPocaDCut = 1.0;
    };

  }
}
