/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vector>
#include <algorithm>

#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/modules/ParticleMassHypothesesUpdater/ParticleMassHypothesesUpdaterModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleMassHypothesesUpdater);

ParticleMassHypothesesUpdaterModule::ParticleMassHypothesesUpdaterModule()
  : Module()
{
  // Set module properties
  setDescription(
    "This module replaces the mass hypotheses of the particles inside the "
    "given particleList with the given pdgCode.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definition
  addParam("particleList", m_particleList, "Input ParticleList", string());
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference", Const::photon.getPDGCode());
}

void ParticleMassHypothesesUpdaterModule::initialize()
{
  m_pdgCode = abs(m_pdgCode);

  DecayDescriptor decayDescriptor;
  const bool valid = decayDescriptor.init(m_particleList);
  if (!valid)
    B2ERROR("ParticleMassHypothesesUpdaterModule::initialize Invalid input DecayString: " << m_particleList);

  const DecayDescriptorParticle* mother = decayDescriptor.getMother();
  int pdgCode = abs(mother->getPDGCode());
  vector<int> allowedPDGs = {11, 13, 211, 321, 2212};
  if (std::find(allowedPDGs.begin(), allowedPDGs.end(), pdgCode) == allowedPDGs.end())
    B2ERROR("ParticleMassHypothesesUpdaterModule::initialize Chosen particle list contains unsupported particles with PDG code " <<
            pdgCode);
  if (std::find(allowedPDGs.begin(), allowedPDGs.end(), m_pdgCode) == allowedPDGs.end())
    B2ERROR("ParticleMassHypothesesUpdaterModule::initialize Chosen target PDG code " << pdgCode << " not supported.");

  string label = mother->getLabel();
  string pName = mother->getName();
  pName.pop_back();
  string sign = mother->getName().substr(pName.length());
  m_newParticleList = "mu" + sign + ":" + label + "_from_" + pName + "_to_mu";
  StoreObjPtr<ParticleList> newList(m_newParticleList);
  newList.registerInDataStore();
  return;
}

void ParticleMassHypothesesUpdaterModule::event()
{
  StoreArray<Track> trackArray;

  StoreObjPtr<ParticleList> originalList(m_particleList);
  if (!originalList) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  } else {
    StoreObjPtr<ParticleList> newList(m_newParticleList);
    if (newList.isValid()) // Check whether it already exists in this path
      return;

    newList.create(); // Create and initialize the list
    newList->initialize(m_pdgCode, m_newParticleList);
    newList->setEditable(true);

    if (originalList->getListSize() == 0) // Do nothing if empty
      return;

    for (unsigned int i = 0; i < originalList->getListSize(); ++i) {
      Const::ChargedStable type(m_pdgCode);

      const Particle* originalParticle = originalList->getParticle(i); // Get particle and check it comes from a track
      if (originalParticle->getParticleSource() != Particle::c_Track) {
        B2WARNING("Particle not built from a track. Skipping.");
        continue;
      }

      unsigned trackIdx = originalParticle->getMdstArrayIndex(); // Get track
      const Track* track = trackArray[trackIdx];
      if (track == nullptr) {
        B2ERROR("Associated track not valid. Skipping.");
        continue;
      }

      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>(); // Get related objects
      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(type);
      const auto& mcParticleWithWeight = track->getRelatedToWithWeight<MCParticle>();

      if (!trackFit) { // Should never happen with the "closest mass" getter
        B2WARNING("Track returned null TrackFitResult pointer. Skipping.");
        continue;
      }

      if (trackFit->getParticleType().getPDGCode() != type.getPDGCode()) { // Skip if fit hypothesis not available
        B2WARNING("Requested track fit hypothesis does not exist. Skipping.");
        continue;
      }

      Particle particle(trackIdx, trackFit, type); // Create new particle with new mass hypothesis

      StoreArray<Particle> particleArray;
      Particle* newPart = particleArray.appendNew(particle);

      if (pid)
        newPart->addRelationTo(pid);
      if (mcParticleWithWeight.first)
        newPart->addRelationTo(mcParticleWithWeight.first, mcParticleWithWeight.second);
      newPart->addRelationTo(trackFit);

      newList->addParticle(newPart); // Add particle to list
    } // Close loop over tracks
    newList->setEditable(false);
  }
}

void ParticleMassHypothesesUpdaterModule::terminate() {}
