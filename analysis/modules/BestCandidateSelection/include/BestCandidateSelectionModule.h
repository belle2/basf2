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

#include <string>

namespace Belle2 {
  class Particle;

  /** Selects Particles with the highest values of 'variable' in the input list and removes all other particles from the list. The remaining list is sorted from best to worst candidate.
  */
  class BestCandidateSelectionModule : public Module {
  public:

    /**
     * Constructor
     */
    BestCandidateSelectionModule();
    virtual ~BestCandidateSelectionModule();

    virtual void initialize();
    virtual void event();

  private:
    std::string m_inputListName; /**< name of input particle list. */
    std::string m_variableName; /**< Variable which defines the candidate ranking. */
    bool m_selectLowest; /**< Select the candidate with the lowest value (instead of highest). */
    int m_numBest; /**< Number of best candidates to keep. */

    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */
    const Variable::Manager::Var* m_variable; /**< Variable which defines the candidate ranking. */

  };

} // Belle2 namespace
