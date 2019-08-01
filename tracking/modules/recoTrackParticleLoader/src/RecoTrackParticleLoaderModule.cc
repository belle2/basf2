/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/recoTrackParticleLoader/RecoTrackParticleLoaderModule.h>

#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixDSym.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(RecoTrackParticleLoader)

RecoTrackParticleLoaderModule::RecoTrackParticleLoaderModule() :
  Module()
{
  setDescription(
    "Takes fitted RecoTracks and creates Particles from them directly, skipping the step of creating Tracks and TrackFitResults.");
//   setPropertyFlags(c_ParallelProcessingCertified); //FIXME is it?

  addParam("recoTrackColName", m_recoTrackColName,
           "Name of the collection holding the input RecoTrack", m_recoTrackColName);
  addParam("particleListName", m_particleListName,
           "Name of the particleList holding the output Particles", m_particleListName);
  addParam("pdgCode", m_pdgCode,
           "PDG code of the hypothesis of the output Particles", m_pdgCode);
}

void RecoTrackParticleLoaderModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  recoTracks.isRequired();

  StoreArray<Particle> particles;
  particles.registerInDataStore();
  particles.registerRelationTo(recoTracks);

  StoreObjPtr<ParticleExtraInfoMap> extraInfo;
  extraInfo.registerInDataStore();

  StoreObjPtr<ParticleList> pList(m_particleListName);
  pList.registerInDataStore();
}

void RecoTrackParticleLoaderModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  StoreArray<Particle> particles;
  StoreObjPtr<ParticleList> pList(m_particleListName);
  pList.create();
  pList->initialize(m_pdgCode, m_particleListName);

  for (auto& recoTrack : recoTracks) {
    if (!recoTrack.wasFitSuccessful()) {
      B2DEBUG(20, "Skipping unfitted RecoTrack.");
      continue;
    }
    auto rep = recoTrack.getCardinalRepresentation();
    auto firstHit = recoTrack.getMeasuredStateOnPlaneFromFirstHit(rep);
    TVector3 pos;
    TVector3 mom;
    TMatrixDSym cov;
    firstHit.getPosMomCov(pos, mom, cov);
    int pdg = rep->getPDG();
    double mass = rep->getMass(firstHit);
    double charge = rep->getCharge(firstHit); // mplTrackRep returns magnetic charge
    double E = std::sqrt(mom.x() * mom.x() + mom.y() * mom.y() + mom.z() * mom.z() + mass * mass);
    double pValue = recoTrack.getTrackFitStatus(rep)->getPVal();
    TLorentzVector lorentzMom(mom.x(), mom.y(), mom.z(), E);

    Particle* newPart = particles.appendNew(lorentzMom, pdg);
    newPart->setVertex(pos);
    newPart->setPValue(pValue);
    newPart->writeExtraInfo("magCharge", charge);
    newPart->writeExtraInfo("massFromFit", mass);
    newPart->addRelationTo(&recoTrack);
    if (std::abs(pdg) == m_pdgCode) pList->addParticle(newPart);
  }
}
