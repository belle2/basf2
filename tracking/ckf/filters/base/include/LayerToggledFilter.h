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
  template <class AFilterFactory>
  class LayerToggledFilter : public AFilterFactory::CreatedFilter {
  public:
    LayerToggledFilter() : AFilterFactory::CreatedFilter()
    {
      AFilterFactory::CreatedFilter::addProcessingSignalListener(&m_highLayerFilter);
      AFilterFactory::CreatedFilter::addProcessingSignalListener(&m_lowLayerFilter);
    }

    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toggleOnLayer"),
      m_param_toggleOnLayer, "", m_param_toggleOnLayer);

      m_highLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "high"));
      m_lowLayerFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "low"));
    }

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
    int m_param_toggleOnLayer = 0;

    TrackFindingCDC::ChooseableFilter<AFilterFactory> m_highLayerFilter;
    TrackFindingCDC::ChooseableFilter<AFilterFactory> m_lowLayerFilter;
  };
}
