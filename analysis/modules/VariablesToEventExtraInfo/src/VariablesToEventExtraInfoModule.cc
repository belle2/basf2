/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yo Sato                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/VariablesToEventExtraInfo/VariablesToEventExtraInfoModule.h>

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;

REG_MODULE(VariablesToEventExtraInfo)

VariablesToEventExtraInfoModule::VariablesToEventExtraInfoModule()
{
  setDescription("For each particle in the input list the selected variables are saved in an event-extra-info field with the given name. Can be used to save MC truth information, for example, in a ntuple of reconstructed particles.");
  setPropertyFlags(c_ParallelProcessingCertified);

  std::map<std::string, std::string> emptymap;
  addParam("particleList", m_inputListName, "Name of particle list with reconstructed particles.");
  addParam("variables", m_variables,
           "Dictionary of variables and extraInfo names to save in the event-extra-info field.\n"
           "Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptymap);
  addParam("overwrite", m_overwrite,
           "-1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case if extra info with given name already exists",
           0);
}

VariablesToEventExtraInfoModule::~VariablesToEventExtraInfoModule() = default;

void VariablesToEventExtraInfoModule::initialize()
{
  StoreArray<Particle>().isRequired();
  m_inputList.isRequired(m_inputListName);

  //collection function pointers
  for (const auto& pair : m_variables) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(pair.first);
    if (!var) {
      B2ERROR("Variable '" << pair.first << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
      m_extraInfoNames.push_back(pair.second);
    }
  }

}

void VariablesToEventExtraInfoModule::event()
{
  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }

  const unsigned int numParticles = m_inputList->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    Particle* p = m_inputList->getParticle(i);
    addEventExtraInfo(p);
  }

}

void VariablesToEventExtraInfoModule::addEventExtraInfo(const Particle* source)
{
  if (not m_eventExtraInfo.isValid()) m_eventExtraInfo.create();

  const unsigned int nVars = m_functions.size();
  for (unsigned int iVar = 0; iVar < nVars; iVar++) {
    double value = m_functions[iVar](source);

    if (m_eventExtraInfo->hasExtraInfo(m_extraInfoNames[iVar])) {
      double current = m_eventExtraInfo->getExtraInfo(m_extraInfoNames[iVar]);
      if (m_overwrite == -1) {
        if (value < current)
          m_eventExtraInfo->setExtraInfo(m_extraInfoNames[iVar], value);
      } else if (m_overwrite == 1) {
        if (value > current)
          m_eventExtraInfo->setExtraInfo(m_extraInfoNames[iVar], value);
      } else if (m_overwrite == 0) {
        B2WARNING("Extra info with given name " << m_extraInfoNames[iVar] << " already set, I won't set it again.");
      } else if (m_overwrite == 2) {
        m_eventExtraInfo->setExtraInfo(m_extraInfoNames[iVar], value);
      }

    } else {
      m_eventExtraInfo->addExtraInfo(m_extraInfoNames[iVar], value);
    }
  }
}
