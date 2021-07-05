/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/OnHitApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>

namespace Belle2 {
  namespace vxdHoughTracking {
    /**
    * A special findlet, which chooses the filter based on the current path length.
    */
    template <class AHit, class AFindlet>
    class PathLengthToggledApplier : public OnHitApplier<AHit> {
      /// the parent class
      using Super = OnHitApplier<AHit>;

    public:
      /// Add the subfilters as listeners.
      PathLengthToggledApplier();

      /// Expose parameters of the subfilters and the layer to change.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// The weight is calculated using the subfilter based on the geometrical layer of the state.
      void apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                 std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits) override;

    private:
      /// Findlet used for currentPath.size() == 1
      AFindlet m_twoHitFilterFindlet;
      /// Findlet used for currentPath.size() == 2
      AFindlet m_threeHitFilterFindlet;
      /// Findlet used for currentPath.size() == 3
      AFindlet m_fourHitFilterFindlet;
      /// Findlet used for currentPath.size() == 4
      AFindlet m_fiveHitFilterFindlet;
    };

  }
}
