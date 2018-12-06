/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

#include <string>

namespace Belle2 {
  class Particle;

  /** Selects Particles with the highest values of 'variable' in the input list and removes all other particles from the list. Particles will receive an extra-info field '${variable}_rank' containing their rank as an integer starting at 1 (best). The ranking also takes antiparticles into account, so there will only be one B+- candidate with rank=1. Candidates with same value of 'variable' will have different ranks, with undefined order. The remaining list is sorted from best to worst candidate (each charge, e.g. B+/B-, separately).  */
  class BestCandidateSelectionModule : public Module {
  public:

    /**
     * Constructor
     */
    BestCandidateSelectionModule();
    virtual ~BestCandidateSelectionModule() override;

    virtual void initialize() override;
    virtual void event() override;

  private:
    std::string m_inputListName; /**< name of input particle list. */
    std::string m_variableName; /**< Variable which defines the candidate ranking. */
    std::string m_outputVariableName; /**< Name of generated Ranking-Variable, if specified by user */
    bool m_selectLowest; /**< Select the candidate with the lowest value (instead of highest). */
    bool m_allowMultiRank; /**< Give the same rank to candidates with the same value */
    int m_numBest; /**< Number of best candidates to keep. */
    std::string m_cutParameter; /**< Selection for candidates to be ranked. */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    const Variable::Manager::Var* m_variable; /**< Variable which defines the candidate ranking. */

  };

} // Belle2 namespace
