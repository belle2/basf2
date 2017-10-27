/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariableToReturnValue/VariableToReturnValueModule.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>

#include <cmath>

using namespace Belle2;

// Register module in the framework
REG_MODULE(VariableToReturnValue)


VariableToReturnValueModule::VariableToReturnValueModule() :
  Module(), m_function(0)
{
  //Set module properties
  setDescription("Calculate event-based variable specified by the user and sets return value of the module accordingly.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("variable", m_variable,
           "Variable taken from Variable::Manager, see output of 'basf2 variables.py'.", std::string(""));
}

void VariableToReturnValueModule::initialize()
{
  const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(m_variable);
  if (!var) {
    B2ERROR("Variable '" << m_variable << "' is not available in Variable::Manager!");
  } else {
    m_function = var->function;
  }
}

void VariableToReturnValueModule::event()
{
  this->setReturnValue(static_cast<int>(std::lround(m_function(nullptr))));
}
