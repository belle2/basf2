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

    /// Filter for relations based on the polar angle difference.
    class SimpleRelationFilter : public BaseRelationFilter {
    public:
      /// Return the weight based on azimuthal-angle separation
      TrackFindingCDC::Weight operator()(const std::pair<const VXDHoughState*, const VXDHoughState*>& relation) override;
      /// Expose the parameters.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    private:
      /// Cut on relations in theta for overlay region on same layer but different ladder
      double m_param_SimpleThetaCutDeltaL0 = 0.05;
      /// Filter relations in theta between hit states where the layer difference is +-1
      double m_param_SimpleThetaCutDeltaL1 = 0.1;
      /// Filter relations in theta between hit states where the layer difference is +-2
      double m_param_SimpleThetaCutDeltaL2 = 0.2;
    };

  }
}
