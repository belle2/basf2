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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

namespace Belle2 {
  /**
   * A special filter, which is chooseable based on a filter factory differently
   * for layers higher than N and for the rest. The toggle layer N is also configurable.
   */
  template <class AFilterFactory>
  class LayerToggledFilter : public AFilterFactory::CreatedFilter {
  public:
    /// Add the subfilters as listeners.
    LayerToggledFilter() : AFilterFactory::CreatedFilter()
    {
      AFilterFactory::CreatedFilter::addProcessingSignalListener(&m_highLayerFilter);
      AFilterFactory::CreatedFilter::addProcessingSignalListener(&m_lowLayerFilter);
    }

    /// Expose parameters of the subfilters and the layer to change.
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toggleOnLayer"),
      m_param_toggleOnLayer, "", m_param_toggleOnLayer);

      m_highLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "high"));
      m_lowLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "low"));
    }

    /// The weight is calculated using the subfilter based on the geometrical layer of the state.
    TrackFindingCDC::Weight operator()(const typename AFilterFactory::CreatedFilter::Object& currentState) final {
      const unsigned int layer = currentState.extractGeometryLayer();
      if (layer > m_param_toggleOnLayer)
      {
        return m_highLayerFilter(currentState);
      } else {
        return m_lowLayerFilter(currentState);
      }
    }

  private:
    /// When to switch between the two filters.
    int m_param_toggleOnLayer = 0;

    /// The filter to use for layers higher then the toggle layer.
    TrackFindingCDC::ChooseableFilter<AFilterFactory> m_highLayerFilter;
    /// The filter to use for layers smaller or equal then the toggle layer.
    TrackFindingCDC::ChooseableFilter<AFilterFactory> m_lowLayerFilter;
  };
}
