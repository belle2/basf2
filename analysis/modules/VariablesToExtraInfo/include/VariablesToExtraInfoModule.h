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

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  class Particle;

  /**
   *  For each particle in the input list the selected variables are saved in an extra-info field with the given name,
   *  Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.");
   *
   * The module can also write any variable as an extra-info filed to any of the daughter particles specified via the DecayString.
   * This is usefull for example when calculating various vetos (pi0, J/psi, ...). Note that in general it is not advised
   * to write anything to daughter particle, since the daughter particle is not unique, it can be daughter of some other particle
   * in an event as well.
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
    /**< name of input particle list. */
    std::string m_inputListName;
    /**< input particle list */
    StoreObjPtr<ParticleList> m_inputList;
    /** Map of variables and extraInfo names to save in the extra-info field. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::map<std::string, std::string> m_variables;
    /**< Vector of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /**< Vector of extra info names */
    std::vector<std::string> m_extraInfoNames;

    std::string m_decayString;  /**< DecayString specifying the daughter Particle to which the extra-info field will be added */
    DecayDescriptor m_pDDescriptor;         /**< Decay descriptor of the particle being selected */
    bool m_writeToDaughter = false;

    /**
     * Adds extra info to the particle. The value is estimated for the source
     * and added as an extra-info to the destination.
     */
    void addExtraInfo(const Particle* source, Particle* destination);

  };

} // Belle2 namespace
