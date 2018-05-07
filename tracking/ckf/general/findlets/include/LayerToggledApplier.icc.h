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

#include <tracking/ckf/general/findlets/LayerToggledApplier.dcl.h>
#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {

  /// Add the subfilters as listeners.
  template <class AState, class AFindlet>
  LayerToggledApplier<AState, AFindlet>::LayerToggledApplier() : Super()
  {
    Super::addProcessingSignalListener(&m_highLayerFindlet);
    Super::addProcessingSignalListener(&m_equalLayerFindlet);
    Super::addProcessingSignalListener(&m_lowLayerFindlet);
  }

  /// Expose parameters of the subfilters and the layer to change.
  template <class AState, class AFindlet>
  void LayerToggledApplier<AState, AFindlet>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toggleOnLayer"),
                                  m_param_toggleOnLayer, "Where to toggle between low, equal and high filter",
                                  m_param_toggleOnLayer);

    m_highLayerFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "high"));
    m_equalLayerFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "equal"));
    m_lowLayerFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "low"));
  }

  /// The weight is calculated using the subfilter based on the geometrical layer of the state.
  template <class AState, class AFindlet>
  void LayerToggledApplier<AState, AFindlet>::apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
                                                    std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    const AState* previousState = currentPath.back();
    const int layer = previousState->getGeometricalLayer();

    if (layer > m_param_toggleOnLayer) {
      m_highLayerFindlet.apply(currentPath, childStates);
    } else if (layer == m_param_toggleOnLayer) {
      m_equalLayerFindlet.apply(currentPath, childStates);
    } else {
      m_lowLayerFindlet.apply(currentPath, childStates);
    }
  }
}
