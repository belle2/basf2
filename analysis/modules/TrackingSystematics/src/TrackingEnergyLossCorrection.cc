/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/TrackingSystematics/TrackingEnergyLossCorrection.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/ParticleList.h>

#include <Math/Vector4D.h>

#include <cmath>
#include <map>

using namespace Belle2;

REG_MODULE(TrackingEnergyLossCorrection);

TrackingEnergyLossCorrectionModule::TrackingEnergyLossCorrectionModule() : Module()
{
  setDescription(
    R"DOC(Module to modify Energy of tracks from the lists. Include in your code as

.. code:: python

    mypath.add_module("TrackingEnergyLossCorrection", particleLists=['pi+:cut'], correction=0.001)

The module modifies the input particleLists by subtracting the correction value to the track energy and rescaling the momenta
		     
		     )DOC");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("correction", m_correction, "correction value to be subtracted from the particle energy",
           nan(""));                                        // Nan
  addParam("payloadName", m_payloadName, "ID of table used for reweighing", std::string(""));
  addParam("correctionName", m_correctionName, "Label for the correction in the look up table", std::string(""));
}

void TrackingEnergyLossCorrectionModule::initialize()
{
  if (!std::isnan(m_correction) && !m_payloadName.empty()) {
    B2FATAL("It's not allowed to provide both a valid value for the scale parameter and a non-empty table name. Please decide for one of the two options!");
  } else if (std::isnan(m_correction) && m_payloadName.empty()) {
    B2FATAL("Neither a valid value for the scale parameter nor a non-empty table name was provided. Please set (exactly) one of the two options!");
  } else if (!m_payloadName.empty()) {
    m_ParticleWeightingLookUpTable = std::make_unique<DBObjPtr<ParticleWeightingLookUpTable>>(m_payloadName);

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

void TrackingEnergyLossCorrectionModule::event()
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
      if (particle->getParticleSource() != Particle::EParticleSourceObject::c_Composite and
          particle->getParticleSource() != Particle::EParticleSourceObject::c_V0 and
          particle->getParticleSource() != Particle::EParticleSourceObject::c_Track) {
        B2WARNING("particle source " << particle->getParticleSource() <<
                  " is not within the expected values. please check before continuing");
        continue;
      }
      setEnergyLossCorrection(particle);
    }
  }
}


// Getting LookUp info for given particle in given event
double TrackingEnergyLossCorrectionModule::getCorrectionValue(Particle* particle)
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
    particle->writeExtraInfo(m_payloadName + "_" + entry.first, entry.second);
  }

  return particle->getExtraInfo(m_payloadName + "_" + m_correctionName);
}



void TrackingEnergyLossCorrectionModule::setEnergyLossCorrection(Particle* particle)
{
  if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
      particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
    for (auto daughter : particle->getDaughters()) {
      setEnergyLossCorrection(daughter);
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
    const ROOT::Math::PxPyPzEVector vec(px, py, pz, E);
    particle->set4Vector(vec);
  } else if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Track) {
    if (!std::isnan(m_correction)) {
      particle->setEnergyLossCorrection(m_correction);
    } else if (!m_correctionName.empty()) {
      particle->setEnergyLossCorrection(getCorrectionValue(particle));
    }
  }


}
