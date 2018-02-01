/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "reconstruction/modules/DedxSkim/DedxSkimModule.h"

#include <mdst/dataobjects/TrackFitResult.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <cmath>

#define mass_e    0.511e-3
#define mass_mu 105.658e-3
#define mass_pi 139.570e-3
#define mass_k  493.677e-3
#define mass_p  938.272e-3

using namespace Belle2;

REG_MODULE(DedxSkim)

DedxSkimModule::DedxSkimModule() : Module()
{

  setDescription("Apply clean up cuts for dE/dx purposes.");

  // set default parameters
  m_eventType = {0, 1};
  m_EoverP = {0.85, 1.15};
  m_EccOverEcm = {0.75, 1.15};

  addParam("eventType", m_eventType,
           "Event type: (-1) clean up tracks, (0) bhabha, (1) radiative bhabha, (2) two photon (e+e-), (3) di-muon, (4) radiative di-muon (5) D-decays (D*->D0 pi; D0 -> K pi)",
           m_eventType);

  addParam("unmatchedCluster", m_unmatchedCluster, "number of unmatched clusters per event", int(0));
  addParam("EoverP", m_EoverP, "range for E/p per track", m_EoverP);
  addParam("EccOverEcm", m_EccOverEcm, "ranger for total energy depostied in the calorimeter divided by the cm energy per event",
           m_EccOverEcm);

  m_eventID = -1;
  m_trackID = 0;
}

DedxSkimModule::~DedxSkimModule() {}

void DedxSkimModule::initialize()
{

  // requred inputs
  m_tracks.isRequired();
}

void DedxSkimModule::event()
{

  m_eventID++;

  // a boolean to toss out bad events
  bool pass(true);

  // booleans for individual sample types
  bool m_Bhabha(false), m_RadBhabha(false), m_TwoPhoton(false);
  bool m_DiMuon(false), m_RadDiMuon(false);
  bool m_Base(false);

  for (unsigned int i = 0; i < m_eventType.size(); i++) {
    switch (m_eventType[i]) {
      case 4:
        m_RadDiMuon = true;
        break;
      case 3:
        m_DiMuon = true;
        break;
      case 2:
        m_TwoPhoton = true;
        break;
      case 1:
        m_RadBhabha = true;
        break;
      case 0:
        m_Bhabha = true;
        break;
      case -1:
        // make sure at least one good track per event
        m_Base = true;
        pass = false;
        break;
    }
  }

  int nGoodElectrons = 0;
  int nGoodMuons = 0;

  // loop over each track in the event and cut on track quality information
  m_trackID = 0;
  for (int iTrack = 0; iTrack < m_tracks.getEntries(); iTrack++) {
    const Track* track = m_tracks[iTrack];
    m_trackID++;

    // if no type is specified, just clean up based on missing track fits
    // -> make sure there is at least one good track per event
    if (m_Base == true && isGoodTrack(track, Const::pion)) {
      pass = true;
      break;
    }

    // only using the electron hypothesis
    if (m_Bhabha == true || m_RadBhabha == true || m_TwoPhoton == true) {

      const TrackFitResult* fitResult = track->getTrackFitResult(Const::electron);
      TVector3 trackMom = fitResult->getMomentum();
      double trackEnergy = sqrt(trackMom.Mag2() + mass_e * mass_e);
      double EoverP = trackEnergy / trackMom.Mag();

      // track level cuts
      if (EoverP < m_EoverP[1] && EoverP > m_EoverP[0])
        nGoodElectrons++;
    } // end of bhabha selection


    // only using the muon hypothesis
    if (m_DiMuon == true || m_RadDiMuon == true) {

      const TrackFitResult* fitResult = track->getTrackFitResult(Const::muon);
      TVector3 trackMom = fitResult->getMomentum();
      double trackEnergy = sqrt(trackMom.Mag2() + mass_mu * mass_mu);
      double EoverP = trackEnergy / trackMom.Mag();

      // track level cuts
      if (EoverP < m_EoverP[1] && EoverP > m_EoverP[0])
        nGoodMuons++;
    } // end of di-muon selection
  } // end of loop over tracks

  if ((m_Bhabha == true || m_RadBhabha == true || m_TwoPhoton == true) && nGoodElectrons != 2)
    pass = false;
  if ((m_DiMuon == true || m_RadDiMuon == true) && nGoodMuons != 2)
    pass = false;

  setReturnValue(pass);
}

bool DedxSkimModule::isGoodTrack(const Track* track, const Const::ChargedStable& chargedStable)
{

  // check if the track fit failed
  const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(chargedStable);
  if (!fitResult) {
    B2WARNING("No related fit for this track, skipping");
    return false;
  }

  // check if there are (enough) cdc hits for this track
  RecoTrack* recoTrack = track->getRelatedTo<RecoTrack>();
  if (!recoTrack || recoTrack->getNumberOfTotalHits() == 0) {
    B2WARNING("Track has no associated hits, skipping");
    return false;
  }

  // extract some information
  double trackPVal = fitResult->getPValue();
  double d0 = fitResult->getD0();
  double z0 = fitResult->getZ0();
  int nCDCHits = recoTrack->getNumberOfTotalHits();

  // apply track quality cuts
  if (trackPVal < 0.00001 || nCDCHits < 1 || std::abs(d0) <= 0.1 || std::abs(z0) <= 10) {
    return false;
  }

  return true;
}

void DedxSkimModule::terminate()
{

  B2INFO("DedxSkimModule exiting after processing " << m_trackID <<
         " tracks in " << m_eventID + 1 << " events.");
}
