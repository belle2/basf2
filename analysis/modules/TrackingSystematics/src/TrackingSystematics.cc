/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/TrackingSystematics/TrackingSystematics.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/ParticleList.h>

#include <map>
#include <TRandom.h>

using namespace Belle2;

REG_MODULE(TrackingEfficiency);
REG_MODULE(TrackingMomentum);

TrackingEfficiencyModule::TrackingEfficiencyModule() : Module()
{
  setDescription(
    R"DOC(Module to remove tracks from the lists at random. Include in your code as

    .. code:: python

        mypath.add_module("TrackingEfficiency", particleLists=['pi+:cut'], frac=0.01)

The module modifies the input particleLists by randomly removing tracks with the probability frac.
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("frac", m_frac, "probability to remove the particle", 0.0);
}

void TrackingEfficiencyModule::event()
{
  // map from mdstIndex to decision
  std::map <unsigned, bool> indexToRemove;

  // determine list of mdst tracks:
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);
    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particleList->getPDGCode())))) {
      B2ERROR("The provided particlelist " << iList << " does not contain track-based particles.");
    }

    std::vector<unsigned int> toRemove;
    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      unsigned mdstIndex = particle->getMdstArrayIndex();
      bool remove;
      if (indexToRemove.find(mdstIndex) !=  indexToRemove.end()) {
        // found, use entry
        remove = indexToRemove.at(mdstIndex);
      } else {
        // not found, generate and store it
        auto prob = gRandom->Uniform();
        remove = prob < m_frac;
        indexToRemove.insert(std::pair{mdstIndex, remove});
      }
      if (remove) toRemove.push_back(particle->getArrayIndex());
    }
    particleList->removeParticles(toRemove);
  }
}

TrackingMomentumModule::TrackingMomentumModule() : Module()
{
  setDescription(
    R"DOC(Module to modify momentum of tracks from the lists. Include in your code as

    .. code:: python

        mypath.add_module("TrackingMomentum", particleLists=['pi+:cut'], scale=0.999)

The module modifies the input particleLists by scaling track momenta as given by the parameter scale
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("scale", m_scale, "scale factor to be applied to 3-momentum", nan(""));
  addParam("tableName", m_tableName, "ID of table used for reweighing", std::string(""));
  addParam("scalingFactorName", m_scalingFactorName, "Label for the scale factor in the look up table", std::string("SF"));
}

void TrackingMomentumModule::initialize()
{
  if (!isnan(m_scale) && !m_tableName.empty()) {
    B2FATAL("It's not allowed to provide both a valid value for the scale parameter and a non-empty table name. Please decide for one of the two options!");
  } else if (isnan(m_scale) && m_tableName.empty()) {
    B2FATAL("Neither a valid value for the scale parameter nor a non-empty table name was provided. Please set (exactly) one of the two options!");
  } else if (!m_tableName.empty()) {
    m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_tableName);

    std::vector<std::string> variables =  Variable::Manager::Instance().resolveCollections((
                                            *m_ParticleWeightingLookUpTable.get())->getAxesNames());
    for (const auto& i_variable : variables) {
      const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(i_variable);
      if (!var) {
        B2FATAL("Variable '" << i_variable << "' is not available in Variable::Manager!");
      }
    }
  }
}

void TrackingMomentumModule::event()
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
      setMomentumScalingFactor(particle);
    }
  }
}


// Getting LookUp info for given particle in given event
double TrackingMomentumModule::getScale(Particle* particle)
{
  std::vector<std::string> variables =  Variable::Manager::Instance().resolveCollections((
                                          *m_ParticleWeightingLookUpTable.get())->getAxesNames());

  std::map<std::string, double> values;
  for (const auto& i_variable : variables) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(i_variable);
    double value = std::get<double>(var->function(particle));
    values.insert(std::make_pair(i_variable, value));
  }

  WeightInfo info = (*m_ParticleWeightingLookUpTable.get())->getInfo(values);
  for (const auto& entry : info) {
    particle->writeExtraInfo(m_tableName + "_" + entry.first, entry.second);
  }

  return particle->getExtraInfo(m_tableName + "_" + m_scalingFactorName);
}




void TrackingMomentumModule::setMomentumScalingFactor(Particle* particle)
{
  if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
      particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
    for (auto daughter : particle->getDaughters()) {
      setMomentumScalingFactor(daughter);
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
  } else if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Track) {
    double scale = (!isnan(m_scale)) ? m_scale : getScale(particle) ;
    particle->setMomentumScalingFactor(scale);
  }


}
