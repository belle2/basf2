/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yo Sato                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  class Particle;

  /**
   *  For each particle in the input list the selected variables are saved in an event-extra-info field with the given name,
   *  Can be used to save MC truth information, for example, in a ntuple of reconstructed particles.
   */
  class VariablesToEventExtraInfoModule : public Module {
  public:

    /**
     * Constructor
     */
    VariablesToEventExtraInfoModule();
    virtual ~VariablesToEventExtraInfoModule() override;

    /** initialise */
    virtual void initialize() override;
    /** process event: actually adds the extra info */
    virtual void event() override;

  private:
    /** name of input particle list. */
    std::string m_inputListName;
    /** input particle list */
    StoreObjPtr<ParticleList> m_inputList;
    /** event extra info object pointer */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo;
    /** Map of variables and extraInfo names to save in the event-extra-info field. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::map<std::string, std::string> m_variables;
    /** Vector of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** Vector of extra info names */
    std::vector<std::string> m_extraInfoNames;

    /** -1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case if extra info with given name already exists */
    int m_overwrite = 0;

    /**
     * Adds extra info to the event. The value is estimated for the source
     * and added as an event-extra-info to the destination.
     */
    void addEventExtraInfo(const Particle* source);

  };

} // Belle2 namespace
