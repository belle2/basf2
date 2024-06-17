/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ParticleCopy.h>
#include <analysis/modules/ParticleMassHypothesesUpdater/ParticleMassHypothesesUpdaterModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <map>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMassHypothesesUpdater);

ParticleMassHypothesesUpdaterModule::ParticleMassHypothesesUpdaterModule(): Module()
{
  // Set module properties
  setDescription("This module creates a new particle list with the desired pdg code using tracks from an input particle list (basically updates the mass hypotheses of the input list).");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definition
  addParam("particleList", m_particleList, "Input ParticleList", string());
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference", Const::photon.getPDGCode());
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void ParticleMassHypothesesUpdaterModule::initialize()
{
  DecayDescriptor decayDescriptor;
  const bool valid = decayDescriptor.init(m_particleList);
  if (!valid)
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_particleList);

  map<int, string> allowedPDGs = {{11, "e"}, {13, "mu"}, {211, "pi"}, {321, "K"}, {2212, "p"}};

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  int pdgCode = mother->getPDGCode();
  if (allowedPDGs.find(abs(pdgCode)) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen particle list contains unsupported particles with PDG code " <<
            pdgCode);
  if (allowedPDGs.find(abs(m_pdgCode)) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen target PDG code " << m_pdgCode << " not supported.");

  string label = mother->getLabel();
  string pName = mother->getName();
  string sign = string(1, pName.back());
  pName.pop_back();
  m_newParticleList = allowedPDGs[abs(m_pdgCode)] + sign + ":" + label + "_converted_from_" + pName;

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;

  StoreObjPtr<ParticleList> newList(m_newParticleList);
  newList.registerInDataStore(flags);

  m_newAntiParticleList = ParticleListName::antiParticleListName(m_newParticleList);
  m_isSelfConjugatedParticle = (m_newParticleList == m_newAntiParticleList);
  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiParticleList(m_newAntiParticleList);
    antiParticleList.registerInDataStore(flags);
  }

  return;
}

void ParticleMassHypothesesUpdaterModule::event()
{

  StoreObjPtr<ParticleList> originalList(m_particleList);
  if (!originalList) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  } else {
    if (originalList->getListSize() == 0)  // Do nothing if empty
      return;

    DecayDescriptor newDecayDescriptor;
    const bool valid = newDecayDescriptor.init(m_newParticleList);
    if (!valid)
      B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_newParticleList);

    const DecayDescriptorParticle* newMother = newDecayDescriptor.getMother();
    int newPdgCode = newMother->getPDGCode();

    StoreObjPtr<ParticleList> newList(m_newParticleList);
    if (newList.isValid()) { // Check whether it already exists in this path
      B2WARNING("The new particle list already exists, and it should not. Did you call the module twice?");
      return;
    } else {
      newList.create();  // Create and initialize the list
      newList->initialize(newPdgCode, m_newParticleList);
      newList->setEditable(true);
    }

    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> newAntiList(m_newAntiParticleList);
      if (!newAntiList.isValid()) { // Check whether it already exists in this path
        newAntiList.create();  // Create and initialize the list
        newAntiList->initialize(-1 * newPdgCode, m_newAntiParticleList);
        newAntiList->setEditable(true);
      }
      newAntiList->bindAntiParticleList(*(newList));
    }

    for (unsigned int i = 0; i < originalList->getListSize(); ++i) {

      const Particle* originalParticle = originalList->getParticle(i);  // Get particle and check it comes from a track
      if (originalParticle->getParticleSource() != Particle::c_Track) {
        B2WARNING("Particle not built from a track. Skipping.");
        continue;
      }

      Particle* newPart = ParticleCopy::copyParticle(originalParticle);
      newPart->setPDGCode(newPdgCode);
      newPart->updateMass(newPdgCode);

      newList->addParticle(newPart);  // Add particle to list
    }  // Close loop over tracks
    newList->setEditable(false);
  }
}

void ParticleMassHypothesesUpdaterModule::terminate() {}
