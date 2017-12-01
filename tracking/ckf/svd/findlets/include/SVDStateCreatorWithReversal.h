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
  class CKFToSVDState;
  class ModuleParamList;

  /// An adaption of the normal state creator introducing another parameter to reverse the seed.
  class SVDStateCreatorWithReversal : public StateCreator<RecoTrack, CKFToSVDState> {
    /// Parent class
    using Super = StateCreator<RecoTrack, CKFToSVDState>;

  public:
    /// Create states from the space points, including a reverse flag or not
    void apply(const std::vector<RecoTrack*>& objects, std::vector<CKFToSVDState>& states) final;

    /// Expose the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

  private:
    /// Reverse the seed.
    bool m_param_reverseSeed = false;
  };
}
