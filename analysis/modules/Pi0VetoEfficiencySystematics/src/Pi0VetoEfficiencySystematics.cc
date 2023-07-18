/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/Pi0VetoEfficiencySystematics/Pi0VetoEfficiencySystematics.h>
#include <iostream>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/ParticleList.h>

#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(Pi0VetoEfficiencySystematics);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Pi0VetoEfficiencySystematicsModule::Pi0VetoEfficiencySystematicsModule() : Module()
{
  setDescription("Includes data/MC weights for pi0 veto efficiency as extraInfo for a given particle list. One must call writeP0EtaVeto function in advance. Weights and their errors will be provided for given mode and threshold.");
  // Parameter definitions
  std::vector<std::string> emptylist;
  addParam("particleLists", m_ParticleLists, "input particle lists", emptylist);
  addParam("decayString", m_decayString, "decay string", std::string(""));
  addParam("tableName", m_tableName, "table name of the payloads", std::string(""));
  addParam("threshold", m_threshold, "threshold of pi0 veto", 0.);
  addParam("mode", m_mode, "pi0 veto option name", std::string(""));
  addParam("suffix", m_suffix, "suffix of extrainfo", std::string(""));
}

// Getting LookUp info for given hard photon in given event
WeightInfo Pi0VetoEfficiencySystematicsModule::getInfo(const Particle* particle)
{
  std::vector<std::string> variables =  Variable::Manager::Instance().resolveCollections((
                                          *m_ParticleWeightingLookUpTable.get())->getAxesNames());
  std::map<std::string, double> values;
  for (const auto& i_variable : variables) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(i_variable);
    if (!var) {
      B2ERROR("Variable '" << i_variable << "' is not available in Variable::Manager!");
    }
    values.insert(std::make_pair(i_variable, std::get<double>(var->function(particle))));
  }

  return (*m_ParticleWeightingLookUpTable.get())->getInfo(values);
}

void Pi0VetoEfficiencySystematicsModule::initialize()
{
  bool valid = m_decayDescriptor.init(m_decayString);
  if (!valid) {
    B2ERROR("Invalid input decay string: " << m_decayString);
  }

  if (m_threshold < 0.1 || 0.99 < m_threshold) {
    B2ERROR("Please provide pi0veto threshold from 0.10 to 0.99 for Pi0VetoEfficiencySystematicsModule");
  }

  //Table is identified with mode and threshold
  std::string tableName = m_tableName + "_" + m_mode + std::to_string((int)(m_threshold * 100 + 0.001));
  m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(tableName);
}


void Pi0VetoEfficiencySystematicsModule::event()
{

  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      std::vector<const Particle*> selectedParticles = m_decayDescriptor.getSelectionParticles(particle);
      int nSelected = selectedParticles.size();
      // Hard photon must be specified by decayString
      if (nSelected != 1) {
        B2ERROR("You selected " << nSelected << " particle(s). Select only a hard photon");
        break;
      }
      const Particle* selectedDaughterParticle = selectedParticles[0];
      addPi0VetoEfficiencyRatios(particle, selectedDaughterParticle);
    }
  }

}

void Pi0VetoEfficiencySystematicsModule::addPi0VetoEfficiencyRatios(Particle* B, const Particle* hardPhoton)
{
  if (hardPhoton->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster
      && hardPhoton->getPDGCode() == Const::photon.getPDGCode()) {
    //The selected particle is photon reconstructed from ECL cluster
    WeightInfo info = getInfo(hardPhoton);
    for (const auto& entry : info) {
      const std::string extraInfoName = "Pi0VetoEfficiencySystematics_" + m_mode + m_suffix + "_" + entry.first;
      if (B->hasExtraInfo(extraInfoName)) {
        if (B->getExtraInfo(extraInfoName) == entry.second) {
          B2INFO("extraInfo " << extraInfoName << " has been already set.");
        } else {
          B2INFO("extraInfo " << extraInfoName << " has been already set and will be overwritten. Original: "
                 << B->getExtraInfo(extraInfoName) << ", New: " << entry.second);
          B->setExtraInfo(extraInfoName, entry.second);
        }
      } else {
        B->addExtraInfo(extraInfoName, entry.second);
      }
    }
  } else {
    B2WARNING("The given hard photon is not from ECL or not photon");
  }
}
