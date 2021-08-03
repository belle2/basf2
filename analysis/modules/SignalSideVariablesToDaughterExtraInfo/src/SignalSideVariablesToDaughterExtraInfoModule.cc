/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/SignalSideVariablesToDaughterExtraInfo/SignalSideVariablesToDaughterExtraInfoModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SignalSideVariablesToDaughterExtraInfo)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SignalSideVariablesToDaughterExtraInfoModule::SignalSideVariablesToDaughterExtraInfoModule() : Module()
{
  // Set module properties

  setDescription("The module writes properties (values of specified variables) of the particle related to the current ROE\n"
                 "as an ExtraInfo to the single particle in the input ParticleList.\n"
                 "This module is intended to be executed only in for_each ROE path.");

  // Parameter definitions
  std::map<std::string, std::string> emptymap;
  addParam("particleListName", m_particleListName, "The input particleList name. This list should contain at most 1 particle",
           std::string(""));
  addParam("variablesToExtraInfo", m_variablesToExtraInfo,
           "Dictionary of variables and extraInfo names to save in the extra-info field.",
           emptymap);
  addParam("overwrite", m_overwrite,
           "-1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case if extra info with given name already exists",
           0);
}

void SignalSideVariablesToDaughterExtraInfoModule::initialize()
{
  StoreArray<Particle>().isRequired();
  m_inputList.isRequired(m_particleListName);

  for (const auto& pair : m_variablesToExtraInfo) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(pair.first);
    if (!var) {
      B2ERROR("Variable '" << pair.first << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
      m_extraInfoNames.push_back(pair.second);
    }
  }
}

void SignalSideVariablesToDaughterExtraInfoModule::event()
{
  StoreObjPtr<ParticleList> plist(m_particleListName);
  unsigned int n = plist->getListSize();

  if (n == 0)
    return;

  if (n > 1)
    B2WARNING("Input ParticleList " << m_particleListName << " contains more than 1 particle. plist.size = " << n);

  StoreObjPtr<RestOfEvent> roe("RestOfEvent");
  if (roe.isValid()) {
    auto* signalSide = roe->getRelated<Particle>();
    Particle* daughter = plist->getParticle(0);

    const unsigned int nVars = m_functions.size();
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      double value;
      if (std::holds_alternative<double>(m_functions[iVar](signalSide))) {
        value = std::get<double>(m_functions[iVar](signalSide));
      } else if (std::holds_alternative<int>(m_functions[iVar](signalSide))) {
        value = std::get<int>(m_functions[iVar](signalSide));
      } else if (std::holds_alternative<bool>(m_functions[iVar](signalSide))) {
        value = std::get<bool>(m_functions[iVar](signalSide));
      }
      if (daughter->hasExtraInfo(m_extraInfoNames[iVar])) {
        double current = daughter->getExtraInfo(m_extraInfoNames[iVar]);
        if (m_overwrite == -1) {
          if (value < current)
            daughter->setExtraInfo(m_extraInfoNames[iVar], value);
        } else if (m_overwrite == 1) {
          if (value > current)
            daughter->setExtraInfo(m_extraInfoNames[iVar], value);
        } else if (m_overwrite == 0) {
          B2WARNING("Extra info with given name " << m_extraInfoNames[iVar] << " already set, I won't set it again.");
        } else if (m_overwrite == 2) {
          daughter->setExtraInfo(m_extraInfoNames[iVar], value);
        }
      } else {
        daughter->addExtraInfo(m_extraInfoNames[iVar], value);
      }
    }
  }
}
