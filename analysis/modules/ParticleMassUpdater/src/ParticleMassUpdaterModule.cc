/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/ParticleMassUpdater/ParticleMassUpdaterModule.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMassUpdater)

ParticleMassUpdaterModule::ParticleMassUpdaterModule() : Module()
{
  //Set module properties
  setDescription("This module replaces the mass of the particles inside the given particleLists with the invariant mass of the particle corresponding to the given pdgCode.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference", Const::photon.getPDGCode());
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
      if (particlelist->getListSize() == 0) continue;
      for (unsigned int i = 0; i < particlelist->getListSize(); ++i) {
        Particle* iParticle = particlelist->getParticle(i);
        iParticle -> updateMass(m_pdgCode);
      }
    }
  }
}

void ParticleMassUpdaterModule::terminate()
{
}

