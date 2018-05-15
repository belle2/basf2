/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.dcl.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  template<class ... AIOTypes>
  bool BaseEventTimeExtractor<AIOTypes...>::wasSuccessful() const
  {
    return m_wasSuccessful;
  }

  template<class ... AIOTypes>
  void BaseEventTimeExtractor<AIOTypes...>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "overwriteExistingEstimation"),
                                  m_param_overwriteExistingEstimation,
                                  "Is it fine to overwrite the current EventT0?",
                                  m_param_overwriteExistingEstimation);

    Super::exposeParameters(moduleParamList, prefix);
  }

  template<class ... AIOTypes>
  void BaseEventTimeExtractor<AIOTypes...>::initialize()
  {
    Super::initialize();
    m_eventT0.registerInDataStore();
  }

  template<class ... AIOTypes>
  void BaseEventTimeExtractor<AIOTypes...>::beginEvent()
  {
    Super::beginEvent();

    m_wasSuccessful = false;

    if (not m_eventT0.isValid()) {
      m_eventT0.create();
    }

    m_eventT0Before = m_eventT0->getEventT0Component();
  }

  template<class ... AIOTypes>
  void BaseEventTimeExtractor<AIOTypes...>::resetEventT0() const
  {
    if (m_eventT0Before) {
      m_eventT0->setEventT0(*m_eventT0Before);
    } else {
      m_eventT0->clearEventT0();
    }
  }
}
