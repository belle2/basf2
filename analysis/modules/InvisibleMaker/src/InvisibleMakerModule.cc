/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Lukas Bierwirth                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/InvisibleMaker/InvisibleMakerModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ParticleCopy.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <iostream>

using namespace Belle2;

// Register module in the framework
REG_MODULE(InvisibleMaker)

InvisibleMakerModule::InvisibleMakerModule() : Module()
{
  //Set module properties
  setDescription("This module makes one particle in the decay invisible to the treefitter module.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("motherParticleList", m_strMotherParticleList, "Name of the mother particleList", std::string());
  addParam("decayStringTarget", m_decayStringTarget, "Select the daughter which will be made invisible",
           std::string());
  addParam("eraseFourMomentum", m_eraseFourMomentum, "If true, the four-momemtum of the invisible particle will be set to zero",
           true);
  addParam("dummyCovMatrix", m_dummyCovMatrix, "Sets the value of the diagonal covariance matrix of the target Particle", 10000.);
}

void InvisibleMakerModule::initialize()
{

  StoreObjPtr<ParticleList>().isRequired(m_strMotherParticleList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();

  auto valid = m_pDDescriptorTarget.init(m_decayStringTarget);
  if (!valid)
    B2ERROR("InvisibleMakerModule::initialize Invalid Decay Descriptor: " << m_decayStringTarget);
}

void InvisibleMakerModule::event()
{

  StoreArray<Particle> particles;
  StoreObjPtr<ParticleList> pListMother(m_strMotherParticleList);

  if (!pListMother) {
    B2ERROR("ParticleList " << m_strMotherParticleList << " not found");
    return;
  }

  const unsigned int numMothers = pListMother->getListSize();
  TMatrixFSym covariance(7);
  for (int row = 0; row < 7; ++row) { //diag
    covariance(row, row) = m_dummyCovMatrix;
  }

  for (unsigned int i = 0; i < numMothers; i++) {
    Particle* iMother = pListMother->getParticle(i);

    std::vector<const Particle*> selParticlesTarget = m_pDDescriptorTarget.getSelectionParticles(iMother);
    if (selParticlesTarget.size() != 1)
      B2ERROR("InvisibleMakerModule::event Length of selected Particles not equal one: " << m_decayStringTarget);

    Particle* targetD = particles[selParticlesTarget[0]->getArrayIndex()];

    std::vector<Particle*> daughters = targetD->getDaughters();

    for (auto& daughter : daughters) {
      targetD -> removeDaughter(daughter, false);
    }

    targetD->writeExtraInfo("treeFitterTreatMeAsInvisible", 1);
    targetD->setMomentumVertexErrorMatrix(covariance);
    ROOT::Math::PxPyPzEVector zero4Vector = {0., 0., 0., 0.};
    if (m_eraseFourMomentum)
      targetD->set4Vector(zero4Vector);

  }
}

void InvisibleMakerModule::terminate()
{
}

