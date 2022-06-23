/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Lukas Bierwirth                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <iostream>

#include <analysis/modules/ParticleMomentumUpdater/ParticleMomentumUpdaterModule.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMomentumUpdater)

ParticleMomentumUpdaterModule::ParticleMomentumUpdaterModule() : Module()
{

  //Set module properties
  setDescription("This module replaces the momentum of the particles in the selected target particle list by p(beam) - p(selected daughters)");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleList", m_particleList, "Name of particle list with reconstructed particles.");
  addParam("decayStringTarget", m_decayStringTarget,
           "DecayString specifying the target particle whose momentum will be updated",
           std::string(""));
  addParam("decayStringDaughters", m_decayStringDaughters,
           "DecayString specifying the daughter particles used to replace the momentum of the target particle by p(beam)-p(daughters)",
           std::string(""));
  addParam("copyDaughters", m_copyDaughters,
           "Create Copy of daughters? Yes or no.",
           false);
}

void ParticleMomentumUpdaterModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();

  size_t countSelection = std::count(m_decayStringTarget.begin(), m_decayStringTarget.end(), '^');
  if (countSelection != 1)
    B2ERROR("ParticleMomentumUpdaterModule::please select exactly one target: " << m_decayStringDaughters);

  bool valid = m_pDDescriptorDaughters.init(m_decayStringDaughters);
  if (!valid)
    B2ERROR("ParticleMomentumUpdaterModule::initialize Invalid Decay Descriptor: " << m_decayStringDaughters);

  valid = m_pDDescriptorTarget.init(m_decayStringTarget);
  if (!valid)
    B2ERROR("ParticleMomentumUpdaterModule::initialize Invalid Decay Descriptor: " << m_decayStringTarget);
}

void ParticleMomentumUpdaterModule::event()
{

  StoreArray<Particle> particles;
  StoreObjPtr<ParticleList> plist(m_particleList);

  if (!plist) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  }

  PCmsLabTransform T;
  ROOT::Math::PxPyPzEVector boost4Vector = T.getBeamFourMomentum();
  ROOT::Math::PxPyPzEVector daughters4Vector;

  const unsigned int numParticles = plist->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    Particle* iParticle = plist->getParticle(i);

    if (m_copyDaughters)
      ParticleCopy::copyDaughters(iParticle);

    std::vector<const Particle*> selParticlesTarget = m_pDDescriptorTarget.getSelectionParticles(iParticle);
    std::vector<const Particle*> selParticlesDaughters = m_pDDescriptorDaughters.getSelectionParticles(iParticle);

    daughters4Vector = {0., 0., 0., 0.};
    for (auto& selParticle : selParticlesDaughters) {
      daughters4Vector += selParticle->get4Vector();
    }

    Particle* targetP = particles[selParticlesTarget[0]->getArrayIndex()];
    targetP->set4Vector(boost4Vector - daughters4Vector);
  }
}

void ParticleMomentumUpdaterModule::terminate()
{
}

