/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Yefan Tao    ustctao@ufl.edu                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/V0DaughterMassUpdater/V0DaughterMassUpdaterModule.h>
#include <analysis/dataobjects/ParticleList.h>
using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(V0DaughterMassUpdater)

V0DaughterMassUpdaterModule::V0DaughterMassUpdaterModule() : Module()
{
  //Set module properties
  setDescription("This module replaces the mass of two daughters of the selected V0 particles inside the given particleLists with masses of given pdgCode. The particle in selected particleList has to have 2 daughters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
  addParam("pdgCodeOfV0posDaughter", m_pdg_pos_dau, "PDG code of daughter with positive charge", 11);
  addParam("pdgCodeOfV0posDaughter", m_pdg_neg_dau, "PDG code of daughter with negative charge", 11);

}


void V0DaughterMassUpdaterModule::event()
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
        if (iParticle->getParticleType() != Particle::EParticleType::c_Composite)
          B2FATAL("This V0 particle is not a composite particle!");
        else {
          std::vector<Belle2::Particle*> dau = iParticle -> getDaughters();
          if (dau.size() != 2)
            B2FATAL("This V0 particle has " << dau.size() << " daughters, the number of daughters has to be 2.");
          else {
            dau[0]->updateMass(m_pdg_pos_dau);
            dau[1]->updateMass(m_pdg_neg_dau);
          }
        }
      }
    }
  }
}

