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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>

namespace Belle2 {
  template<class AVarSet>
  class StateTeacher : public TrackFindingCDC::Findlet<typename AVarSet::Object*> {
  public:
    /// The pair of seed and hit vector to check
    using State = typename AVarSet::Object;
    /// The parent class
    using Super = TrackFindingCDC::Findlet<State*>;

    StateTeacher() : m_varSetFilter("truth")
    {
      this->addProcessingSignalListener(&m_varSetFilter);
    }

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_varSetFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter("enableStateTeacher", m_param_enableStateTeacher,
                                    "Enable adding truth information from the teacher to the states.",
                                    m_param_enableStateTeacher);
    }

    /// Main function of this findlet: add truth information from the passed var set
    void apply(std::vector<State*>& states) final {
      if (not m_param_enableStateTeacher)
      {
        return;
      }
      for (State* state : states)
      {
        const auto& isCorrect = m_varSetFilter(*state);

        state->setTruthInformation(isCorrect != 0);
      }
    }

  private:
    /// Subfindlet: A filter compiled from the given var set
    TrackFindingCDC::NamedChoosableVarSetFilter<AVarSet> m_varSetFilter;

    // Parameters
    /// Parameter: Enable adding truth information from the teacher to the states.
    bool m_param_enableStateTeacher = false;
  };
}
