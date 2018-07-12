/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/general/findlets/StateCreatorWithReversal.dcl.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {

  template <class AState>
  void StateCreatorWithReversal<AState>::apply(const std::vector<RecoTrack*>& objects, std::vector<AState>& states)
  {
    for (const RecoTrack* object : objects) {
      states.emplace_back(object, m_param_reverseSeed);
    }
  }

  template <class AState>
  void StateCreatorWithReversal<AState>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter("reverseSeed",
                                  m_param_reverseSeed,
                                  "Reverse the seed.");
  }
}