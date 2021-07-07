/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/PhotonEfficiencySystematics/PhotonEfficiencySystematics.h>
#include <iostream>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>
#include <analysis/VariableManager/Manager.h>

#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(PhotonEfficiencySystematics);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhotonEfficiencySystematicsModule::PhotonEfficiencySystematicsModule() : Module()
{
  setDescription(
    R"DOC(Module to include data/MC weights for photon detection efficiency. Include in your code as

    .. code:: python

        mypath.add_module("PhotonEfficiencySystematics", particleLists=['gamma:cut'], tableName=tableName_Weight)

		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("tableName", m_tableName, "ID of table used for reweighing");
}

// Getting LookUp info for given particle in given event
WeightInfo PhotonEfficiencySystematicsModule::getInfo(const Particle* particle)
{
  std::vector<std::string> variables =  Variable::Manager::Instance().resolveCollections((
                                          *m_ParticleWeightingLookUpTable.get())->getAxesNames());
  std::map<std::string, double> values;
  for (const auto& i_variable : variables) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(i_variable);
    if (!var) {
      B2ERROR("Variable '" << i_variable << "' is not available in Variable::Manager!");
    }
    values.insert(std::make_pair(i_variable, var->function(particle)));
  }

  return (*m_ParticleWeightingLookUpTable.get())->getInfo(values);
}

void PhotonEfficiencySystematicsModule::beginRun()
{
  m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_tableName);
}


void PhotonEfficiencySystematicsModule::event()
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
      addPhotonDetectionEfficiencyRatios(particle);
    }
  }

}

void PhotonEfficiencySystematicsModule::addPhotonDetectionEfficiencyRatios(Particle* particle)
{
  if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster
      && particle->getPDGCode() == Const::photon.getPDGCode()) {
    //particle is photon reconstructed from ECL cluster
    WeightInfo info = getInfo(particle);
    for (const auto& entry : info) {
      particle->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
    }
  }
}

