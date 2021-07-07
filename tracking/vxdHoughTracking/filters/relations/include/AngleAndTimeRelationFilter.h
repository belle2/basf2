/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/relations/BaseRelationFilter.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Filter for relations based on the polar angle difference and the time difference of the hits.
    class AngleAndTimeRelationFilter : public BaseRelationFilter {
    public:
      /// Return the weight based on azimuthal-angle separation and the time difference of the hits on both sides.
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
