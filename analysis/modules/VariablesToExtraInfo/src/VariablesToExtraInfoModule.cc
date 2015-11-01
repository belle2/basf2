/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kohl                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/VariablesToExtraInfo/VariablesToExtraInfoModule.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;


REG_MODULE(VariablesToExtraInfo)


VariablesToExtraInfoModule::VariablesToExtraInfoModule()
{
  setDescription("For each particle in the input list the selected variables are saved in an extra-info field with the given name. Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  std::map<std::string, std::string> emptymap;
  addParam("particleList", m_inputListName, "Name of particle list with reconstructed particles.");
  addParam("variables", m_variables,
           "Dictionary of variables and extraInfo names to save in the extra-info field. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptymap);
}

VariablesToExtraInfoModule::~VariablesToExtraInfoModule()
{
}

void VariablesToExtraInfoModule::initialize()
{
  StoreArray<Particle>::required();
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

void VariablesToExtraInfoModule::event()
{
  StoreArray<Particle> particles;

  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }


  const unsigned int numParticles = m_inputList->getListSize();
  const unsigned int nVars = m_functions.size();

  for (unsigned int i = 0; i < numParticles; i++) {
    Particle* p = m_inputList->getParticle(i);
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      double value = m_functions[iVar](p);
      p->addExtraInfo(m_extraInfoNames[iVar], value);
    }
  }
}
