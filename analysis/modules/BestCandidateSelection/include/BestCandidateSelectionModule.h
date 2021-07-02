/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

#include <string>

namespace Belle2 {
  class Particle;

  /**
   * Ranks particles by the values of a variable. The ranking also takes
   * antiparticles into account. The rank information is written to a variable
   * and low ranking candidates can be discarded.
   */
  class BestCandidateSelectionModule : public Module {
  public:

    /** Constructor */
    BestCandidateSelectionModule();
    /** Destructor */
    virtual ~BestCandidateSelectionModule() override;
    /** Initialize the module (set up datastore) */
    virtual void initialize() override;
    /** Process an event */
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

    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    const Variable::Manager::Var* m_variable; /**< Variable which defines the candidate ranking. */

  };

} // Belle2 namespace
