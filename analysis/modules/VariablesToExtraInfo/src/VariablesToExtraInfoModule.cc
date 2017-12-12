/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kohl, Anze Zupanc                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/VariablesToExtraInfo/VariablesToExtraInfoModule.h>

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
           "Dictionary of variables and extraInfo names to save in the extra-info field.\n"
           "Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptymap);
  addParam("decayString", m_decayString, "DecayString specifying the daughter Particle to be included in the ParticleList",
           std::string(""));
  addParam("overwrite", m_overwrite,
           "-1/0/1: Overwrite if lower / don't overwrite / overwrite if higher, in case if extra info with given name already exists", 0);
}

VariablesToExtraInfoModule::~VariablesToExtraInfoModule()
{
}

void VariablesToExtraInfoModule::initialize()
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

  if (not m_decayString.empty()) {
    m_writeToDaughter = true;

    bool valid = m_pDDescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("VariablesToExtraInfoModule::initialize Invalid Decay Descriptor: " << m_decayString);
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
  for (unsigned int i = 0; i < numParticles; i++) {
    Particle* p = m_inputList->getParticle(i);

    if (not m_writeToDaughter) {
      addExtraInfo(p, p);
    } else {
      std::vector<const Particle*> selparticles = m_pDDescriptor.getSelectionParticles(p);
      for (unsigned int iDaug = 0; iDaug < selparticles.size(); iDaug++) {
        Particle* daug = particles[selparticles[iDaug]->getArrayIndex()];
        addExtraInfo(p, daug);
      }
    }
  }
}

void VariablesToExtraInfoModule::addExtraInfo(const Particle* source, Particle* destination)
{
  const unsigned int nVars = m_functions.size();
  for (unsigned int iVar = 0; iVar < nVars; iVar++) {
    double value = m_functions[iVar](source);
    if (destination->hasExtraInfo(m_extraInfoNames[iVar])) {
      double current = destination->getExtraInfo(m_extraInfoNames[iVar]);
      if (m_overwrite == -1) {
        if (value < current)
          destination->setExtraInfo(m_extraInfoNames[iVar], value);
      } else if (m_overwrite == 1) {
        if (value > current)
          destination->setExtraInfo(m_extraInfoNames[iVar], value);
      } else if (m_overwrite == 0) {
        B2WARNING("Extra info with given name " << m_extraInfoNames[iVar] << " already set, I won't set it again.");
      }

    } else {
      destination->addExtraInfo(m_extraInfoNames[iVar], value);
    }
  }
}
