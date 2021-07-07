#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

#include <string>

namespace Belle2 {

  /** Module to calculate variable specified by the user and set return value accordingly
   */
  class VariableToReturnValueModule : public Module {
  public:
    /** Constructor. */
    VariableToReturnValueModule();

    /** Initialize */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;

  private:
    /** function pointer corresponding to given variable. */
    Variable::Manager::FunctionPtr m_function;
    /** variable name (module parameter) */
    std::string m_variable;

  };
} // end namespace Belle2
