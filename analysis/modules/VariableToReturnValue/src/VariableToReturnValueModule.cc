/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariableToReturnValue/VariableToReturnValueModule.h>

#include <analysis/dataobjects/EventExtraInfo.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <cmath>

using namespace Belle2;

// Register module in the framework
REG_MODULE(VariableToReturnValue)


VariableToReturnValueModule::VariableToReturnValueModule() :
  Module(), m_function(nullptr)
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
  int returnValue;
  if (std::holds_alternative<double>(m_function(nullptr))) {
    returnValue = std::get<double>(m_function(nullptr));
  } else if (std::holds_alternative<int>(m_function(nullptr))) {
    returnValue = std::get<int>(m_function(nullptr));
  } else if (std::holds_alternative<bool>(m_function(nullptr))) {
    returnValue = std::get<bool>(m_function(nullptr));
  }
  this->setReturnValue(returnValue);
}
