/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/recoTrackParticleLoader/RecoTrackParticleLoaderModule.h>

#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TVector3.h>
#include <TMatrixDSym.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(RecoTrackParticleLoader)

RecoTrackParticleLoaderModule::RecoTrackParticleLoaderModule() :
  Module()
{
  setDescription(
    "Takes fitted RecoTracks and creates Particles from them directly, skipping the step of creating Tracks and TrackFitResults.");
  setPropertyFlags(c_ParallelProcessingCertified);

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
    int pdg = rep->getPDG();
    auto firstHit = recoTrack.getMeasuredStateOnPlaneFromFirstHit(rep);
    genfit::MeasuredStateOnPlane extrapolatedMSoP = firstHit;
    try {
      extrapolatedMSoP.extrapolateToLine(TVector3(0.0, 0.0, 0.0), TVector3(0.0, 0.0, 1.0));
    } catch (...) {
      B2WARNING("Could not extrapolate the fit result for pdg " << pdg <<
                " to the IP. Why, I don't know.");
      continue;
    }
    TVector3 pos;
    TVector3 mom;
    TMatrixDSym cov;
    extrapolatedMSoP.getPosMomCov(pos, mom, cov);
    double mass = rep->getMass(extrapolatedMSoP);
    double charge = rep->getCharge(extrapolatedMSoP); // mplTrackRep returns magnetic charge
    double E = std::sqrt(mom.x() * mom.x() + mom.y() * mom.y() + mom.z() * mom.z() + mass * mass);
    double pValue = recoTrack.getTrackFitStatus(rep)->getPVal();
    ROOT::Math::PxPyPzEVector lorentzMom(mom.x(), mom.y(), mom.z(), E);

    Particle* newPart = particles.appendNew(lorentzMom, pdg);
    newPart->setVertex(ROOT::Math::XYZVector(pos));
    newPart->setPValue(pValue);
    newPart->writeExtraInfo("magCharge", charge);
    newPart->writeExtraInfo("massFromFit", mass);
    newPart->addRelationTo(&recoTrack);
    if (std::abs(pdg) == m_pdgCode) pList->addParticle(newPart);
  }
}
