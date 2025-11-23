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
#include <analysis/modules/ParticleMassHypothesesUpdater/ParticleMassHypothesesUpdaterModule.h>
#include <analysis/utility/ParticleCopy.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <TDatabasePDG.h>

#include <map>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMassHypothesesUpdater);

ParticleMassHypothesesUpdaterModule::ParticleMassHypothesesUpdaterModule(): Module()
{
  // Set module properties
  setDescription("This module updates the mass hypothesis of particleList to pdgCode. "
                 "The module creates a new particle list containing copies of the original particles, with updated mass hypotheses. "
                 "The newly created particle list is named after the input one plus the suffix ``_converted_from_`` and the old mass hypothesis, "
                 "e.g. ``e+:mylist`` to pdgCode = 13 becomes ``mu+:mylist_converted_from_e``. "
                 "The only supported mass hypotheses are electrons, muons, kaons, pions and protons (for both input and output lists).");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definition
  addParam("particleList", m_particleList, "Input ParticleList", string());
  addParam("pdgCode", m_newPdgCode, "Target PDG code for mass reference.", Const::electron.getPDGCode());
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void ParticleMassHypothesesUpdaterModule::initialize()
{
  m_newPdgCode = abs(m_newPdgCode);

  DecayDescriptor decayDescriptor;
  const bool valid = decayDescriptor.init(m_particleList);
  if (!valid)
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_particleList);

  map<int, string> allowedPDGs = {
    {11, "e"}, {13, "mu"}, {211, "pi"}, {321, "K"}, {2212, "p"}
  };

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  int pdgCode = mother->getPDGCode();
  if (allowedPDGs.find(abs(pdgCode)) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen particle list contains unsupported particles with PDG code " <<
            pdgCode);
  if (allowedPDGs.find(m_newPdgCode) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen target PDG code " << m_newPdgCode << " not supported.");

  string label = mother->getLabel();
  string pName = mother->getName();
  pName.pop_back();
  m_newParticleList = allowedPDGs[m_newPdgCode] + "+:" + label + "_converted_from_" + pName;

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;

  StoreObjPtr<ParticleList> newList(m_newParticleList);
  newList.registerInDataStore(flags);

  m_newAntiParticleList = ParticleListName::antiParticleListName(m_newParticleList);
  StoreObjPtr<ParticleList> antiParticleList(m_newAntiParticleList);
  antiParticleList.registerInDataStore(flags);
}

void ParticleMassHypothesesUpdaterModule::event()
{
  StoreObjPtr<ParticleList> originalList(m_particleList);
  if (!originalList) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  }
  if (originalList->getListSize() == 0)  // Do nothing if empty
    return;

  DecayDescriptor newDecayDescriptor;
  const bool valid = newDecayDescriptor.init(m_newParticleList);
  if (!valid)
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_newParticleList);

  const DecayDescriptorParticle* newMother = newDecayDescriptor.getMother();
  m_newPdgCode = newMother->getPDGCode();  // Get the right sign

  StoreObjPtr<ParticleList> newList(m_newParticleList);
  if (newList.isValid()) { // Check whether it already exists in this path
    B2WARNING("The new particle list already exists, and it should not. Did you call the module twice?");
    return;
  }

  newList.create();  // Create and initialize the list
  newList->initialize(m_newPdgCode, m_newParticleList);

  StoreObjPtr<ParticleList> newAntiList(m_newAntiParticleList);
  if (newAntiList.isValid()) // Check whether it already exists in this path
    B2ERROR("The particle list did not exist but the anti-list did. Something fishy is happening.");

  newAntiList.create();  // Create and initialize the list
  newAntiList->initialize(-1 * m_newPdgCode, m_newAntiParticleList);
  newAntiList->bindAntiParticleList(*(newList));

  for (unsigned int i = 0; i < originalList->getListSize(); ++i) {
    const Particle* originalParticle = originalList->getParticle(i);  // Get particle and check it comes from a track
    if (originalParticle->getParticleSource() != Particle::c_Track) {
      B2WARNING("Particle not built from a track. Skipping.");
      continue;
    }

    Particle* newPart = ParticleCopy::copyParticle(originalParticle);
    newPart->updateMass(m_newPdgCode);
    const int charge = newPart->getCharge();
    if (TDatabasePDG::Instance()->GetParticle(m_newPdgCode)->Charge() / 3.0 == charge)
      newPart->setPDGCode(m_newPdgCode);
    else
      newPart->setPDGCode(-1 * m_newPdgCode);

    newList->addParticle(newPart);  // Add particle to list
  }  // Close loop over tracks
}

void ParticleMassHypothesesUpdaterModule::terminate() {}
