/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/V0DaughterMassUpdater/V0DaughterMassUpdaterModule.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(V0DaughterMassUpdater);

V0DaughterMassUpdaterModule::V0DaughterMassUpdaterModule() : Module()
{
  //Set module properties
  setDescription("This module replaces the mass of two daughters of the selected V0 particles inside the given particleLists with masses of given pdgCode. The particle in selected particleList has to have 2 daughters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
  addParam("pdgCodeOfV0posDaughter", m_pdg_pos_dau, "PDG code of daughter with positive charge", Const::electron.getPDGCode());
  addParam("pdgCodeOfV0negDaughter", m_pdg_neg_dau, "PDG code of daughter with negative charge", Const::electron.getPDGCode());

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
        if (iParticle->getParticleSource() != Particle::EParticleSourceObject::c_Composite)
          B2FATAL("This V0 particle is not a composite particle!");
        else {
          std::vector<Belle2::Particle*> dau = iParticle -> getDaughters();
          if (dau.size() != 2)
            B2FATAL("This V0 particle has " << dau.size() << " daughters, the number of daughters has to be 2.");
          else {
            // To check daughters order of V0 is correct (first positive, second negative)
            bool correctOrder = true;
            if (dau[0]->getCharge() < 0 && dau[1]->getCharge() > 0)  correctOrder = false;
            if (correctOrder) {
              dau[0]->updateMass(m_pdg_pos_dau);
              dau[1]->updateMass(m_pdg_neg_dau);
            } else {
              dau[0]->updateMass(m_pdg_neg_dau);
              dau[1]->updateMass(m_pdg_pos_dau);
            }
          }
        }
      }
    }
  }
}

