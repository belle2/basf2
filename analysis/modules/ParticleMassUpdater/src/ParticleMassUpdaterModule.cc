/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Fernando Abudinen                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/ParticleMassUpdater/ParticleMassUpdaterModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cmath>
#include <algorithm>
#include <TParameter.h>
using namespace std;
using namespace Belle2;
using namespace boost::algorithm;

// Register module in the framework
REG_MODULE(ParticleMassUpdater)

ParticleMassUpdaterModule::ParticleMassUpdaterModule() : Module()
{
  //Set module properties
  setDescription("This module replaces the mass of the particles inside the given particleLists with the invariant mass of the particle corresponding to the given pdgCode. It can also update the masses of two daughters of a V0 particle.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference", 22);
  addParam("updateDaughters", m_updateDaughters,
           "If true, update daughters' masses of the particle in the list, and nothing is done for the particle. This is only for use of V0 particles.",
           false);
  addParam("pdg_dau0", m_pdg_dau0, "PDG code of first daughter", 11);
  addParam("pdg_dau1", m_pdg_dau1, "PDG code of second daughter", 11);

}

void ParticleMassUpdaterModule::initialize()
{
}

void ParticleMassUpdaterModule::event()
{


  for (auto& iList : m_strParticleLists) {

    StoreObjPtr<ParticleList> particlelist(iList);
    if (!particlelist) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    } else {
      if (particlelist->getListSize() == 0)continue;
      for (unsigned int i = 0; i < particlelist->getListSize(); ++i) {
        if (!m_updateDaughters) {
          Particle* iParticle = particlelist->getParticle(i);
          iParticle -> updateMass(m_pdgCode);
        } else {
          Particle* iParticle = particlelist->getParticle(i);
          std::vector<Belle2::Particle*> dau = iParticle -> getDaughters();
          if (dau.size() != 2)
            B2ERROR("This V0 particle has " << dau.size() << " daughters, the number of daughters has to be 2.");
          else {
            dau[0]->updateMass(m_pdg_dau0);
            dau[1]->updateMass(m_pdg_dau1);
          }
        }
      }
    }

  }
}

void ParticleMassUpdaterModule::terminate()
{
}

