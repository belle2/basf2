/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.icc.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {

  template <class AFilter>
  LayerSVDRelationFilter<AFilter>::LayerSVDRelationFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
  }

  template <class AFilter>
  LayerSVDRelationFilter<AFilter>::~LayerSVDRelationFilter() = default;

  template <class AFilter>
  std::vector<CKFToSVDState*>
  LayerSVDRelationFilter<AFilter>::getPossibleTos(CKFToSVDState* currentState,
                                                  const std::vector<CKFToSVDState*>& states) const
  {
    std::vector<CKFToSVDState*> possibleNextStates;

    const unsigned int currentLayer = currentState->getGeometricalLayer();
    const unsigned int nextLayer = std::max(static_cast<int>(currentLayer) - 1 - m_param_hitJumping, 0);

    for (CKFToSVDState* state : states) {
      const unsigned int layer = state->getGeometricalLayer();
      // TODO: Make this more general (not dependent on currentLayer > nextLayer)
      if (currentLayer >= layer and layer >= nextLayer) {
        possibleNextStates.push_back(state);
      }
    }

    return possibleNextStates;
  }

  template <class AFilter>
  void LayerSVDRelationFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumping"), m_param_hitJumping,
                                  "Make it possible to jump over N layers.", m_param_hitJumping);

    m_filter.exposeParameters(moduleParamList, prefix);
  }

  template <class AFilter>
  TrackFindingCDC::Weight LayerSVDRelationFilter<AFilter>::operator()(const CKFToSVDState& from, const CKFToSVDState& to)
  {
    return m_filter(std::make_pair(&from, &to));
  }
}