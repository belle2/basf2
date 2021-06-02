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
#include <analysis/modules/PhotonEfficiencyCorrection/PhotonEfficiencyCorrection.h>
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

REG_MODULE(PhotonEfficiencyCorrection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhotonEfficiencyCorrectionModule::PhotonEfficiencyCorrectionModule() : Module()
{
  setDescription(
    R"DOC(Module to modify energy from the lists. Include in your code as

    .. code:: python

        mypath.add_module("PhotonEfficiencyCorrection", particleLists=['gamma:cut'], scale=tableName_Weight)

The module modifies the input particleLists by scaling energy as given by the scale in the LookUpTable
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("tableName", m_tableName, "ID of table used for reweighing");
}

// Getting LookUp info for given particle in given event
WeightInfo PhotonEfficiencyCorrectionModule::getInfo(const Particle* particle)
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

void PhotonEfficiencyCorrectionModule::beginRun()
{
  //check if this module is used only for MC
  if (!Environment::Instance().isMC()) {
    B2ERROR("Attempting to run PhotonEfficiencyCorrection Data but this should be only used on MC");
  }

  m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_tableName);
}


void PhotonEfficiencyCorrectionModule::event()
{
  //check if this module is used only for data
  if (!Environment::Instance().isMC()) {
    return;
  }

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
      setEnergyScalingFactor(particle);
    }
  }

}

void EnergyBiasCorrectionModule::addPhotonDetectionEfficiencyWeights(Particle* particle)
{
  // Should this necessarily be a photon??? TODO
  if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite) {
    B2ERROR("This should be photon??????");
  }

} else if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster
           && particle->getPDGCode() == Const::photon.getPDGCode())
{
  //particle is photon reconstructed from ECL cluster
  WeightInfo info = getInfo(particle);
  for (const auto& entry : info) {
    particle->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
  }
}
// Not sure at the moment if debug messages are needed
//B2DEBUG(10, "Called setMomentumScalingFactor for an unspecified, track-based or KLM cluster-based particle");
}

