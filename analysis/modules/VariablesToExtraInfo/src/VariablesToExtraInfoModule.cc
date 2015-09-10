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

#include <map>

using namespace std;
using namespace Belle2;


REG_MODULE(VariablesToExtraInfo)


VariablesToExtraInfoModule::VariablesToExtraInfoModule()
{
  setDescription("For each particle in the input list the selected variables are saved in an extra-info field '${variable}_previous'. Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  vector<string> emptylist;
  addParam("particleList", m_inputListName, "Name of particle list with reconstructed particles.");
  addParam("variables", m_variables,
           "List of variables to save in the extra-info field. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);
}

VariablesToExtraInfoModule::~VariablesToExtraInfoModule()
{
}

void VariablesToExtraInfoModule::initialize()
{
  StoreArray<Particle>::required();
  m_inputList.isRequired(m_inputListName);

  //collection function pointers
  for (const string& varStr : m_variables) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
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

  unsigned int nVars = m_variables.size();
  std::vector<std::string> extraInfoNames(nVars);

  for (unsigned int iVar = 0; iVar < nVars; iVar++) {
    extraInfoNames[iVar] = m_variables[iVar] + "_previous";
  }

  const unsigned int numParticles = m_inputList->getListSize();

  for (unsigned int i = 0; i < numParticles; i++) {
    Particle* p = m_inputList->getParticle(i);
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      double value = m_functions[iVar](p);
      p->addExtraInfo(extraInfoNames[iVar], value);
    }
  }
}
