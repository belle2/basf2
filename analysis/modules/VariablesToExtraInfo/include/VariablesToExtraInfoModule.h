/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kohl                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  class Particle;

  /**
   *  For each particle in the input list the selected variables are saved in an extra-info field with the given name,
   *  Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.");
   */
  class VariablesToExtraInfoModule : public Module {
  public:

    /**
     * Constructor
     */
    VariablesToExtraInfoModule();
    virtual ~VariablesToExtraInfoModule() override;

    virtual void initialize() override;
    virtual void event() override;

  private:
    std::string m_inputListName; /**< name of input particle list. */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */

    /**
     * Map of variables and extraInfo names to save in the extra-info field.
     * Variables are taken from Variable::Manager, and are identical to those
     * available to e.g. ParticleSelector.
     */
    std::map<std::string, std::string> m_variables;

    std::vector<Variable::Manager::FunctionPtr> m_functions; /**< Vector of function pointers corresponding to given variables. */
    std::vector<std::string> m_extraInfoNames; /**< Vector of extra info names */

    /**
     * Adds extra info to the particle.
     */
    void addExtraInfo(Particle* part);

  };

} // Belle2 namespace
