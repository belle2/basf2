/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/SignalSideParticleListCreator/SignalSideParticleListCreatorModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SignalSideParticleListCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SignalSideParticleListCreatorModule::SignalSideParticleListCreatorModule() : Module()
{
  // Set module properties
  setDescription("The module creates a ParticleList and fills it with one of the daughter Particles.");

  // Parameter definitions
  addParam("particleListName", m_particleListName,
           "Name of the ParticleList to be created and filled with signal side daughter Particle", std::string(""));
  addParam("decayString", m_decayString, "DecayString specifying the daughter Particle to be included in the ParticleList",
           std::string(""));

}

void SignalSideParticleListCreatorModule::initialize()
{
  m_pdgCode  = 0;

  // check the validity of output ParticleList name
  bool valid = m_pListDDescriptor.init(m_particleListName);
  if (!valid)
    B2ERROR("SignalSideParticleListCreatorModule::initialize Invalid output ParticleList name: " << m_particleListName);

  // Output particle
  const DecayDescriptorParticle* mother = m_pListDDescriptor.getMother();

  m_pdgCode  = mother->getPDGCode();

  m_antiParticleListName = ParticleListName::antiParticleListName(m_particleListName);
  m_isSelfConjugatedParticle = (m_particleListName == m_antiParticleListName);

  StoreObjPtr<ParticleList> particleList(m_particleListName);
  particleList.registerInDataStore(DataStore::c_DontWriteOut);
  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiParticleList(m_antiParticleListName);
    antiParticleList.registerInDataStore(m_antiParticleListName);
  }

  // check the decay descriptor
  valid = m_pDDescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("SignalSideParticleListCreatorModule::initialize Invalid Decay Descriptor: " << m_decayString);

  std::vector<std::string> strNames = m_pDDescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  if (nDecayProducts != 1)
    B2ERROR("SignalSideParticleListCreatorModule::initialize Only one particle can be selected. The specified decayString " <<
            m_decayString << " has " << nDecayProducts);

}

void SignalSideParticleListCreatorModule::event()
{
  StoreObjPtr<ParticleList> plist(m_particleListName);
  bool existingList = plist.isValid();

  if (!existingList) {
    // new particle list: create it
    plist.create();
    plist->initialize(m_pdgCode, m_particleListName);

    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> antiPlist(m_antiParticleListName);
      antiPlist.create();
      antiPlist->initialize(-1 * m_pdgCode, m_antiParticleListName);

      antiPlist->bindAntiParticleList(*(plist));
    }
  }

  StoreObjPtr<RestOfEvent> roe("RestOfEvent");
  if (roe.isValid()) {
    const Particle* particle = roe->getRelated<Particle>();
    if (particle) {
      std::vector<const Particle*> selparticles = m_pDDescriptor.getSelectionParticles(particle);

      if (selparticles[0])
        plist->addParticle(selparticles[0]);
    }
  }

}


