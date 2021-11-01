/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariablesToExtraInfo/VariablesToExtraInfoModule.h>

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

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
           "-1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case if extra info with given name already exists",
           0);
}

VariablesToExtraInfoModule::~VariablesToExtraInfoModule() = default;

void VariablesToExtraInfoModule::initialize()
{
  m_particles.isRequired();
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
      for (auto& selparticle : selparticles) {
        Particle* daug = m_particles[selparticle->getArrayIndex()];
        addExtraInfo(p, daug);
      }
    }
  }
}

void VariablesToExtraInfoModule::addExtraInfo(const Particle* source, Particle* destination)
{
  const unsigned int nVars = m_functions.size();
  for (unsigned int iVar = 0; iVar < nVars; iVar++) {
    double value = std::numeric_limits<double>::quiet_NaN();
    if (std::holds_alternative<double>(m_functions[iVar](source))) {
      value = std::get<double>(m_functions[iVar](source));
    } else if (std::holds_alternative<int>(m_functions[iVar](source))) {
      value = std::get<int>(m_functions[iVar](source));
    } else if (std::holds_alternative<bool>(m_functions[iVar](source))) {
      value = std::get<bool>(m_functions[iVar](source));
    }
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
      } else if (m_overwrite == 2) {
        destination->setExtraInfo(m_extraInfoNames[iVar], value);
      }

    } else {
      destination->addExtraInfo(m_extraInfoNames[iVar], value);
    }
  }
}
