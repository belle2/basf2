/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/OnStateApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <vector>

namespace Belle2 {
  /**
   * A special findlet, which is chooseable based on a given findlet
   * for layers higher than N, N and for the rest. The toggle layer N is also configurable.
   */
  template <class AState, class AFindlet>
  class LayerToggledApplier : public OnStateApplier<AState> {
    /// the parent class
    using Super = OnStateApplier<AState>;

  public:
    /// Add the subfilters as listeners.
    LayerToggledApplier();

    /// Expose parameters of the subfilters and the layer to change.
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final;

    /// The weight is calculated using the subfilter based on the geometrical layer of the state.
    void apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates) override;

  private:
    /// Findlet used for layers > N
    AFindlet m_highLayerFindlet;
    /// Findlet used for layers == N
    AFindlet m_equalLayerFindlet;
    /// Findlet used for layers < N
    AFindlet m_lowLayerFindlet;

    /// On which N to toggle the layers
    int m_param_toggleOnLayer = -99;
  };
}
