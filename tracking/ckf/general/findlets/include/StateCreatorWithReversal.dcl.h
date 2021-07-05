/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/StateCreator.dcl.h>

#include <vector>
#include <string>

namespace Belle2 {
  class RecoTrack;
  class ModuleParamList;

  template <class AState>
  /// An adaption of the normal state creator introducing another parameter to reverse the seed.
  class StateCreatorWithReversal : public StateCreator<RecoTrack, AState> {
    /// Parent class
    using Super = StateCreator<RecoTrack, AState>;

  public:
    /// Create states from the space points, including a reverse flag or not
    void apply(const std::vector<RecoTrack*>& objects, std::vector<AState>& states) final;

    /// Expose the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

  private:
    /// Reverse the seed.
    bool m_param_reverseSeed = false;
  };
}
