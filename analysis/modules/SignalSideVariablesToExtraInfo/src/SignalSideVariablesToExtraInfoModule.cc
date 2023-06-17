/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/SignalSideVariablesToExtraInfo/SignalSideVariablesToExtraInfoModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SignalSideVariablesToExtraInfo);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SignalSideVariablesToExtraInfoModule::SignalSideVariablesToExtraInfoModule() : Module()
{
  // Set module properties
  setDescription("The module writes property (value of specified variable) of single particle\n"
                 "found in the input ParticleList as an ExtraInfo to the Particle related to\n"
                 "the current ROE. This module is intended to be executed only in for_each ROE\n"
                 "path.");

  // Parameter definitions
  std::map<std::string, std::string> emptymap;
  addParam("particleListName", m_particleListName, "The input particleList name.\n"
           "This list should either contain at most 1 particle or all requested variables should be event-based.",
           std::string(""));
  addParam("variableToExtraInfo", m_variableToExtraInfo,
           "Dictionary of variables and extraInfo names to save in the extra-info field.",
           emptymap);
}

void SignalSideVariablesToExtraInfoModule::initialize()
{
  StoreArray<Particle>().isRequired();
  m_inputList.isRequired(m_particleListName);

  for (const auto& pair : m_variableToExtraInfo) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(pair.first);
    if (!var) {
      B2ERROR("Variable '" << pair.first << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
      m_extraInfoNames.push_back(pair.second);
      if (m_allVariablesEventbased and var->description.find("[Eventbased]") == std::string::npos)
        m_allVariablesEventbased = false;
    }
  }
}

void SignalSideVariablesToExtraInfoModule::event()
{
  StoreObjPtr<ParticleList> plist(m_particleListName);
  unsigned int n = plist->getListSize();

  if (n == 0)
    return;

  if (n > 1 and !m_allVariablesEventbased)
    B2WARNING("Input ParticleList " << m_particleListName << " contains more than 1 particle. plist.size = " << n);

  StoreObjPtr<RestOfEvent> roe("RestOfEvent");
  if (roe.isValid()) {
    auto* signalSide = roe->getRelatedFrom<Particle>();

    const unsigned int nVars = m_functions.size();
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      if (signalSide->hasExtraInfo(m_extraInfoNames[iVar])) {
        B2WARNING("Extra info with given name " << m_extraInfoNames[iVar] << " already set, I won't set it again.");
      } else {
        double value = std::numeric_limits<double>::quiet_NaN();
        auto var_result = m_functions[iVar](plist->getParticle(0));
        if (std::holds_alternative<double>(var_result)) {
          value = std::get<double>(var_result);
        } else if (std::holds_alternative<int>(var_result)) {
          value = std::get<int>(var_result);
        } else if (std::holds_alternative<bool>(var_result)) {
          value = std::get<bool>(var_result);
        }
        signalSide->addExtraInfo(m_extraInfoNames[iVar], value);
      }
    }
  }
}
