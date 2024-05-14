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
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>

#include <algorithm>
#include <map>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMassHypothesesUpdater);

ParticleMassHypothesesUpdaterModule::ParticleMassHypothesesUpdaterModule(): Module()
{
  // Set module properties
  setDescription("This module replaces the mass hypotheses of the particles inside the given particleList with the given pdgCode.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definition
  addParam("particleList", m_particleList, "Input ParticleList", string());
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference", Const::photon.getPDGCode());
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void ParticleMassHypothesesUpdaterModule::initialize()
{
  m_pdgCode = abs(m_pdgCode);

  DecayDescriptor decayDescriptor;
  const bool valid = decayDescriptor.init(m_particleList);
  if (!valid)
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_particleList);

  map<int, string> allowedPDGs = {{11, "e"}, {13, "mu"}, {211, "pi"}, {321, "K"}, {2212, "p"}};

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  int pdgCode = abs(mother->getPDGCode());
  if (allowedPDGs.find(pdgCode) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen particle list contains unsupported particles with PDG code " <<
            pdgCode);
  if (allowedPDGs.find(m_pdgCode) == allowedPDGs.end())
    B2FATAL("ParticleMassHypothesesUpdaterModule::initialize Chosen target PDG code " << m_pdgCode << " not supported.");

  string label = mother->getLabel();
  string pName = mother->getName();
  pName.pop_back();
  string sign = mother->getName().substr(pName.length());
  m_newParticleList = allowedPDGs[m_pdgCode] + sign + ":" + label + "_from_" + pName + "_to_" + allowedPDGs[m_pdgCode];

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

    StoreObjPtr<ParticleList> newList(m_newParticleList);
    if (!newList.isValid()) { // Check whether it already exists in this path
      newList.create();  // Create and initialize the list
      newList->initialize(m_pdgCode, m_newParticleList);
      newList->setEditable(true);
    }

    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> newAntiList(m_newAntiParticleList);
      if (!newAntiList.isValid()) { // Check whether it already exists in this path
        newAntiList.create();  // Create and initialize the list
        newAntiList->initialize(-1 * m_pdgCode, m_newAntiParticleList);
        newAntiList->setEditable(true);
      }
      newAntiList->bindAntiParticleList(*(newList));
    }

    for (unsigned int i = 0; i < originalList->getListSize(); ++i) {
      Const::ChargedStable type(m_pdgCode);

      const Particle* originalParticle = originalList->getParticle(i);  // Get particle and check it comes from a track
      if (originalParticle->getParticleSource() != Particle::c_Track) {
        B2WARNING("Particle not built from a track. Skipping.");
        continue;
      }

      StoreArray<Track> trackArray;

      unsigned trackIdx = originalParticle->getMdstArrayIndex();  // Get track
      const Track* track = trackArray[trackIdx];
      if (track == nullptr) {
        B2WARNING("Associated track not valid. Skipping.");
        continue;
      }

      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>();  // Get related objects
      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(type);
      const auto& mcParticleWithWeight = track->getRelatedToWithWeight<MCParticle>();

      if (!trackFit) {  // Should never happen with the "closest mass" getter
        B2WARNING("Track returned null TrackFitResult pointer. Skipping.");
        continue;
      }

      // I think this should not be done since not all tracks are fitted with all mass hypotheses.
      //if (trackFit->getParticleType().getPDGCode() !=
      // type.getPDGCode()) { // Skip if fit hypothesis not available
      //   B2WARNING("Requested track fit hypothesis does not exist.
      //   Skipping."); continue;
      // }

      Particle particle(trackIdx, trackFit, type);  // Create new particle with new mass hypothesis

      StoreArray<Particle> particleArray;
      Particle* newPart = particleArray.appendNew(particle);

      if (pid) newPart->addRelationTo(pid);
      if (mcParticleWithWeight.first)
        newPart->addRelationTo(mcParticleWithWeight.first,
                               mcParticleWithWeight.second);
      newPart->addRelationTo(trackFit);

      newList->addParticle(newPart);  // Add particle to list
    }  // Close loop over tracks
    newList->setEditable(false);
  }
}

void ParticleMassHypothesesUpdaterModule::terminate() {}
