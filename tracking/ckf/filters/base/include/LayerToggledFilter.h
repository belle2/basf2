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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <framework/core/ModuleParamList.h>
#include <vector>

namespace Belle2 {
  /**
   * A special findlet, which is chooseable based on a given findlet
   * for layers higher than N, N and for the rest. The toggle layer N is also configurable.
   */
  template <class AFindlet, class AStateObject>
  class LayerToggledFilter : public TrackFindingCDC::Findlet<AStateObject> {
    /// the parent class
    using Super = TrackFindingCDC::Findlet<AStateObject>;
  public:
    /// Add the subfilters as listeners.
    LayerToggledFilter() : Super()
    {
      Super::addProcessingSignalListener(&m_highLayerFilter);
      Super::addProcessingSignalListener(&m_equalLayerFilter);
      Super::addProcessingSignalListener(&m_lowLayerFilter);
    }

    /// Expose parameters of the subfilters and the layer to change.
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toggleOnLayer"),
      m_param_toggleOnLayer, "", m_param_toggleOnLayer);

      m_highLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "high"));
      m_equalLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "equal"));
      m_lowLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "low"));
    }

    /// The weight is calculated using the subfilter based on the geometrical layer of the state.
    void apply(std::vector<AStateObject>& childStates) override
    {
      GeometryLayerExtractor extractGeometryLayer;

      if (childStates.empty()) {
        return;
      }

      const auto* firstElement = childStates.front();
      const int layer = extractGeometryLayer(*firstElement);

      if (layer > m_param_toggleOnLayer) {
        m_highLayerFilter.apply(childStates);
      } else if (layer == m_param_toggleOnLayer) {
        m_equalLayerFilter.apply(childStates);
      } else {
        m_lowLayerFilter.apply(childStates);
      }
    }

  private:
    /// When to switch between the two filters.
    int m_param_toggleOnLayer = 0;

    /// The filter to use for layers higher then the toggle layer.
    AFindlet m_highLayerFilter;
    /// The filter to use for layers equal then the toggle layer.
    AFindlet m_equalLayerFilter;
    /// The filter to use for layers smaller or equal then the toggle layer.
    AFindlet m_lowLayerFilter;
  };
}
