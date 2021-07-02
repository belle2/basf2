/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/VariableManager/Manager.h>

#include <string>
#include <map>

namespace Belle2 {
  /**
   * The module writes property of single particle found in the input ParticleList
   * as an ExtraInfo to the Particle related to the current ROE. This module
   * is intended to be executed only in for_each roe path. It is used to write
   * out the veto output.
   *
   */
  class SignalSideVariablesToExtraInfoModule : public Module {

  public:

    /** constructor */
    SignalSideVariablesToExtraInfoModule();
    /** Register input and output data */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;


  private:

    std::string m_particleListName;  /**< The input particleList name. This list should contain only 1 particle */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */

    /**
     * Map of variable and extraInfo name to save in the extra-info field.
     * Variable is taken from Variable::Manager and is evaluated for the
     * single particle found in the input particleList.
     */
    std::map<std::string, std::string> m_variableToExtraInfo;

    /** Vector of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** Vector of extra info names */
    std::vector<std::string> m_extraInfoNames;
  };
}

