/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  void StateCreatorWithReversal<AState>::exposeParameters(ModuleParamList* moduleParamList, const std::string&)
  {
    moduleParamList->addParameter("reverseSeed",
                                  m_param_reverseSeed,
                                  "Reverse the seed.");
  }
}
