/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Henrikas Svidras                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/Pi0VetoEfficiencySystematics/Pi0VetoEfficiencySystematics.h>
#include <iostream>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>
#include <analysis/VariableManager/Manager.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

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
  setDescription(
    R"DOC(Module to include data/MC weights for photon detection efficiency. Include in your code as

    .. code:: python

        mypath.add_module("Pi0VetoEfficiencySystematics", particleLists=['gamma:cut'], tableName=tableName_Weight)

     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("decayString", m_decayString, "decay string");
  addParam("threshold", m_threshold, "threshold of pi0 veto");
  addParam("mode", m_mode, "pi0 veto option name");
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

void Pi0VetoEfficiencySystematicsModule::beginRun()
{
  //Table is identified with mode and threshold
  std::string tableName = "pi0vetoDataMC" + m_mode + std::to_string((int)(m_threshold * 100 + 0.001));
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
      DecayDescriptor descriptor;
      descriptor.init(m_decayString);
      std::vector<const Particle*> selectedParticles = descriptor.getSelectionParticles(particle);
      int nSelected = selectedParticles.size();
      // Hard photon must be specified by decayString
      if (!nSelected) {
        B2ERROR("Select a hard photon.");
        break;
      }
      //Particle* selectedParentParticle = selectedParticles[0];
      const Particle* selectedDaughterParticle = selectedParticles[0];
      //addPi0VetoEfficiencyRatios(selectedParentParticle, selectedDaughterParticle);
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
      B->addExtraInfo("Pi0VetoEfficiencySystematics_" + entry.first, entry.second);
    }
  }
}
