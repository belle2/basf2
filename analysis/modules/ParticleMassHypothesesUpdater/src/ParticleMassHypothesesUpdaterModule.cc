/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/modules/ParticleMassHypothesesUpdater/ParticleMassHypothesesUpdaterModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

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
  addParam("pdgCode", m_pdgCode, "PDG code for mass reference",
           Const::photon.getPDGCode());
}

void ParticleMassHypothesesUpdaterModule::initialize() {}

void ParticleMassHypothesesUpdaterModule::event()
{
  StoreArray<Track> trackArray;
  StoreObjPtr<ParticleList> originalList(m_particleList);
  if (!originalList) {
    B2FATAL("ParticleList " << m_particleList << " not found");
    return;
  } else {
    size_t colon = m_particleList.find(":");
    string newListName = "mu+:" + m_particleList.substr(colon + 1) + "_from_" + m_particleList.substr(0, colon) + "_to_mu";
    StoreObjPtr<ParticleList> newList(newListName);
    // Check whether it already exists in this path and can skip any further steps if it does
    if (newList.isValid())
      return;
    newList.create();
    newList->initialize(m_pdgCode, newListName);
    newList->setEditable(true);
    if (originalList->getListSize() == 0) return;
    for (unsigned int i = 0; i < originalList->getListSize(); ++i) {
      Const::ChargedStable type(abs(m_pdgCode));

      const Particle* originalParticle = originalList->getParticle(i);
      if (originalParticle->getParticleSource() != Particle::c_Track) {
        B2WARNING("Particle not built from a track. Skipping.");
        continue;
      }

      unsigned trackIdx = originalParticle->getMdstArrayIndex();

      const Track* track = trackArray[trackIdx];
      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(type);
      const auto& mcParticleWithWeight = track->getRelatedToWithWeight<MCParticle>();

      if (!trackFit) { // should never happen with the "closest mass" getter - leave as a sanity check
        B2WARNING("Track returned null TrackFitResult pointer for ChargedStable::getPDGCode()  = " << type.getPDGCode());
        continue;
      }

      // if (m_enforceFitHypothesis && (trackFit->getParticleType().getPDGCode() != type.getPDGCode())) {
      //   // the required hypothesis does not exist for this track, skip it
      //   continue;
      // }

      Particle particle(trackIdx, trackFit, type);

      StoreArray<Particle> particleArray;
      Particle* newPart = particleArray.appendNew(particle);
      if (pid)
        newPart->addRelationTo(pid);
      if (mcParticleWithWeight.first)
        newPart->addRelationTo(mcParticleWithWeight.first, mcParticleWithWeight.second);
      newPart->addRelationTo(trackFit);

      newList->addParticle(newPart);
    } // loop over tracks
    newList->setEditable(false);
  }
}

void ParticleMassHypothesesUpdaterModule::terminate() {}
