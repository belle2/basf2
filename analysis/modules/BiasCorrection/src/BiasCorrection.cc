// Own include
#include <analysis/modules/BiasCorrection/BiasCorrection.h>
#include <iostream>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>

#include <map>
#include <TRandom.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(EnergyBiasCorrection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EnergyBiasCorrectionModule::EnergyBiasCorrectionModule() : Module()
{
  setDescription(
    R"DOC(Module to modify energy from the lists. Include in your code as

    .. code:: python

        mypath.add_module("EnergyBiasCorrection", particleLists=['gamma:cut'], scale=tableName_Weight)

The module modifies the input particleLists by scaling energy as given by the scale in the LookUpTable
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("tableName", m_tableName, "ID of table used for reweighing");
}

// Getting LookUp info for given particle in given event
WeightInfo EnergyBiasCorrectionModule::getInfo(const Particle* particle)
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

void EnergyBiasCorrectionModule::beginRun()
{
  m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_tableName);
}


void EnergyBiasCorrectionModule::event()
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
      setEnergyScalingFactor(particle);
    }
  }
}

void EnergyBiasCorrectionModule::setEnergyScalingFactor(Particle* particle)
{
  if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
      particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
    for (auto daughter : particle->getDaughters()) {
      setEnergyScalingFactor(daughter);
    }
    double px = 0;
    double py = 0;
    double pz = 0;
    double E = 0;
    for (auto daughter : particle->getDaughters()) {
      px += daughter->getPx();
      py += daughter->getPy();
      pz += daughter->getPz();
      E  += daughter->getEnergy();
    }
    const TLorentzVector vec(px, py, pz, E);
    particle->set4Vector(vec);
  } else if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
    WeightInfo info = getInfo(particle);
    for (const auto& entry : info) {
      particle->addExtraInfo(m_tableName + "_" + entry.first, entry.second);
    }
    particle->setMomentumScalingFactor(particle->getExtraInfo(m_tableName + "_Weight"));
  }
}

