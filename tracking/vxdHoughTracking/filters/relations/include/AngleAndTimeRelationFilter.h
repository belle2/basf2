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

#include <tracking/vxdHoughTracking/filters/relations/BaseRelationFilter.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Base filter for CKF PXD states
    class AngleAndTimeRelationFilter : public BaseRelationFilter {
    public:
      /// Return the weight based on azimuthal-angle separation
      TrackFindingCDC::Weight operator()(const std::pair<const VXDHoughState*, const VXDHoughState*>& relation) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    private:
      /// Cut on relations in theta for overlay region on same layer but different ladder
      double m_param_ThetaCutDeltaL0 = 0.03;
      /// Filter relations in theta between hit states where the layer difference is +-1
      double m_param_ThetaCutDeltaL1 = 0.1;
      /// Filter relations in theta between hit states where the layer difference is +-2
      double m_param_ThetaCutDeltaL2 = 0.2;
      /// Cut on difference in u-side cluster time of the two hits
      double m_param_DeltaTU = 15; // ns
      /// Cut on difference in v-side cluster time of the two hits
      double m_param_DeltaTV = 15; // ns
    };

  }
}
