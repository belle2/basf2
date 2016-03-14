/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIGNALSIDEVARIABLESTOEXTRAINFOMODULE_H
#define SIGNALSIDEVARIABLESTOEXTRAINFOMODULE_H

#include <framework/core/Module.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreArray.h>
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

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SignalSideVariablesToExtraInfoModule();

    /** Register input and output data */
    virtual void initialize();

    /**  */
    virtual void event();


  private:

    std::string m_particleListName;  /**< The input particleList name. This list should contain only 1 particle */
    StoreObjPtr<ParticleList> m_inputList; /**< input particle list */

    /**
     * Map of variable and extraInfo name to save in the extra-info field.
     * Variable is taken from Variable::Manager and is evaluated for the
     * single particle found in the input particleList.
     */
    std::map<std::string, std::string> m_variableToExtraInfo;

    Variable::Manager::FunctionPtr m_function; /**< Function pointer corresponding to given variable. */
    std::string m_extraInfoName; /**< extra info name to be added */
  };
}

#endif /* SIGNALSIDEVARIABLESTOEXTRAINFOMODULE_H */
