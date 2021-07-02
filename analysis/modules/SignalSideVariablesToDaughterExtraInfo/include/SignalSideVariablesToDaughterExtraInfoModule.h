/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>

#include <string>
#include <map>

namespace Belle2 {
  /**
   * The module writes properties (values of specified variables) of the
   * particle related to the current ROE as an ExtraInfo to the single
   * particle in the input ParticleList. This module is intended to be
   * executed only in for_each ROE path.
   */
  class SignalSideVariablesToDaughterExtraInfoModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SignalSideVariablesToDaughterExtraInfoModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Process events */
    virtual void event() override;


  private:

    std::string m_particleListName;  /**< The input particleList name. This list should contain only 1 particle */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */

    /**
     * Map of variable and extraInfo names to save in the extra-info field.
     * Variables are taken from Variable::Manager and are evaluated for the
     * particle related to the current ROE.
     */
    std::map<std::string, std::string> m_variablesToExtraInfo;

    /** Vector of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** Vector of extra info names */
    std::vector<std::string> m_extraInfoNames;
    /** -1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case if extra info with given name already exists */
    int m_overwrite = 0;
  };
}

