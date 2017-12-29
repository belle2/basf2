/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/HitLevelInfoWriter/HitLevelInfoWriter.h>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(HitLevelInfoWriter)

HitLevelInfoWriterModule::HitLevelInfoWriterModule() : Module()
{

  setDescription("Extract dE/dx information for calibration development.");

  addParam("outputFileName", m_filename, "Name for output file", std::string("HLInfo.root"));
  addParam("applyCorrections", m_correct, "Apply corrections (cosmics only)", false);
}

HitLevelInfoWriterModule::~HitLevelInfoWriterModule() { }

void HitLevelInfoWriterModule::initialize()
{

  B2INFO("Creating a ROOT file for the hit level information...");

  // required inputs
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();

  // register output root file
  m_file = new TFile(m_filename.c_str(), "RECREATE");
  m_tree = new TTree("track", "dE/dx information");
  m_tree->SetDirectory(0);

  // event level information (from emd)
  m_tree->Branch("exp", &m_expID, "exp/I");
  m_tree->Branch("run", &m_runID, "run/I");
  m_tree->Branch("event", &m_eventID, "event/I");

  // track level information (from tfr)
  m_tree->Branch("phi", &m_phi, "phi/D");
  m_tree->Branch("vx0", &m_vx0, "vx0/D");
  m_tree->Branch("vy0", &m_vy0, "vy0/D");
  m_tree->Branch("vz0", &m_vz0, "vz0/D");
  m_tree->Branch("db", &m_d0, "db/D");
  m_tree->Branch("dz", &m_z0, "dz/D");
  m_tree->Branch("chi2", &m_chi2, "chi2/D");

  // track level information (from cdt)
  m_tree->Branch("track", &m_trackID, "track/I");
  m_tree->Branch("length", &m_length, "length/D");
  m_tree->Branch("charge", &m_charge, "charge/I");
  m_tree->Branch("costh", &m_cosTheta, "costh/D");
  m_tree->Branch("pF", &m_p, "pF/D");
  m_tree->Branch("eopst", &m_eopst, "eopst/D"); // placeholder for Widget
  m_tree->Branch("pdg", &m_PDG, "pdg/D");

  // calibration constants
  m_tree->Branch("coscor", &m_coscor, "coscor/D");
  m_tree->Branch("coscorext", &m_coscorext, "coscorext/D");
  m_tree->Branch("rungain", &m_rungain, "rungain/D");

  // track level dE/dx measurements
  m_tree->Branch("mean", &m_mean, "mean/D");
  m_tree->Branch("dedx", &m_trunc, "dedx/D");
  m_tree->Branch("dedxorig", &m_truncorig, "dedxorig/D");
  m_tree->Branch("dedxsat", &m_trunc, "dedxsat/D"); // placeholder for Widget
  m_tree->Branch("dedxerr", &m_error, "dedxerr/D");
  m_tree->Branch("chiPi", &m_chipi, "chiPi/D"); // placeholder for Widget

  // layer level information
  m_tree->Branch("lNHits", &l_nhits, "lNHits/I");
  m_tree->Branch("lNHitsUsed", &l_nhitsused, "lNHitsUsed/I");
  m_tree->Branch("lNHitsCombined", l_nhitscombined, "lNHitsCombined[lNHits]/I");
  m_tree->Branch("lWireLongestHit", l_wirelongesthit, "lWireLongestHit[lNHits]/I");
  m_tree->Branch("lLayer", l_layer, "lLayer[lNHits]/I");
  m_tree->Branch("lPath", l_path, "lPath[lNHits]/D");
  m_tree->Branch("lDedx", l_dedx, "lDedx[lNHits]/D");

  // hit level information
  m_tree->Branch("hNHits", &h_nhits, "hNHits/I");
  m_tree->Branch("hLWire", h_lwire, "hLWire[hNHits]/I");
  m_tree->Branch("hWire", h_wire, "hWire[hNHits]/I");
  m_tree->Branch("hLayer", h_layer, "hLayer[hNHits]/I");
  m_tree->Branch("hPath", h_path, "hPath[hNHits]/D");
  m_tree->Branch("hDedx", h_dedx, "hDedx[hNHits]/D");
  m_tree->Branch("hADCRaw", h_adcraw, "hADCRaw[hNHits]/D");
  m_tree->Branch("hDoca", h_doca, "hDoca[hNHits]/D");
  m_tree->Branch("hEnta", h_enta, "hEnta[hNHits]/D");
  m_tree->Branch("hDriftT", h_driftT, "hDriftT[hNHits]/D");
  m_tree->Branch("hWireGain", h_wireGain, "hWireGain[hNHits]/D");
  m_tree->Branch("hTwodcor", &h_twodcor, "hTwodcor[hNHits]/D");
  m_tree->Branch("hOnedcor", &h_onedcor, "hOnedcor[hNHits]/D");

}

void HitLevelInfoWriterModule::event()
{

  const int nbins = 40;
  double coscor[nbins] = {1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
                          0.9998, 0.9651, 0.9538, 0.9542, 0.9356,
                          0.9213, 0.9094, 0.9005, 0.8851, 0.8712,
                          0.8519, 0.8173, 0.7814, 0.7341, 0.6873,
                          0.6873, 0.7341, 0.7814, 0.8173, 0.8519,
                          0.8712, 0.8851, 0.9005, 0.9094, 0.9213,
                          0.9356, 0.9542, 0.9538, 0.9651, 0.9998,
                          1.0000, 1.0000, 1.0000, 1.0000, 1.0000
                         };

  // **************************************************
  //
  //  LOOP OVER dE/dx measurements
  //
  // **************************************************

  for (int idedx = 0; idedx < m_dedxTracks.getEntries(); idedx++) {
    CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];
    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track) {
      B2WARNING("No related track...");
      continue;
    }
    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    if (dedxTrack->size() == 0 || dedxTrack->size() > 200) continue;
    if (dedxTrack->getCosTheta() < -1.0 || dedxTrack->getCosTheta() > 1.0) continue;

    // fill the event meta data
    StoreObjPtr<EventMetaData> evtMetaData;
    m_expID = evtMetaData->getExperiment();
    m_runID = evtMetaData->getRun();
    m_eventID = evtMetaData->getEvent();

    // fill the TTree with the Track information
    fillTrack(fitResult);

    // fill the TTree with the CDCDedxTrack information
    fillDedx(dedxTrack);

    // doing this by hand now, will come from constants eventually
    double binsize = 2.0 / nbins;
    int bin = floor((m_cosTheta + 1.0) / binsize);
    m_coscor = coscor[bin];

    bin = floor((m_cosTheta - 0.5 * binsize + 1.0) / binsize);
    double frac = ((m_cosTheta - 0.5 * binsize + 1.0) / binsize) - bin;

    int thisbin = bin, nextbin = bin + 1;
    if (abs(1 + m_cosTheta) < (binsize / 2.0) || (m_cosTheta > 0 && abs(m_cosTheta) < (binsize / 2.0))) {
      thisbin = bin + 1;
      nextbin = bin + 2;
      frac -= 1;
    } else if (abs(1 - m_cosTheta) < (binsize / 2.0) || (m_cosTheta < 0 && abs(m_cosTheta) < (binsize / 2.0))) {
      thisbin = bin - 1;
      nextbin = bin;
      frac += 1;
    }
    m_coscorext = (coscor[nextbin] - coscor[thisbin]) * frac + coscor[thisbin];

    m_rungain = 48.0;

    m_truncorig = m_trunc;

    if (m_correct) {
      m_trunc = m_trunc / m_coscor;
      m_trunc = m_trunc / m_rungain;
    }

    m_tree->Fill();
  }
}

void HitLevelInfoWriterModule::terminate()
{

  B2INFO("Done writing out the hit level information...\t" << m_tree->GetEntries() << " tracks");

  // write the ttree to a root file
  m_file->cd();
  m_tree->Write();
  m_file->Close();
}

void
HitLevelInfoWriterModule::fillTrack(const TrackFitResult* fitResult)
{
  TVector3 trackMom = fitResult->getMomentum();
  TVector3 trackPos = fitResult->getPosition();

  m_phi = trackMom.Phi();

  m_vx0 = trackPos.x();
  m_vy0 = trackPos.y();
  m_vz0 = trackPos.z();

  m_d0 = fitResult->getD0();
  m_z0 = fitResult->getZ0();
  m_chi2 = fitResult->getPValue();
}

void
HitLevelInfoWriterModule::fillDedx(CDCDedxTrack* dedxTrack)
{
  // clear the containers first
  clearEntries();

  m_trackID = dedxTrack->trackID();
  m_length = dedxTrack->getLength();
  m_charge = dedxTrack->getCharge();
  m_cosTheta = dedxTrack->getCosTheta();
  m_PDG = dedxTrack->getPDG();

  m_p = dedxTrack->getMomentum();
  if (m_charge < 0) m_p *= -1;

  h_nhits = dedxTrack->size();
  l_nhits = dedxTrack->getNLayerHits();
  l_nhitsused = dedxTrack->getNLayerHitsUsed();

  m_mean = dedxTrack->getDedx();
  m_trunc = dedxTrack->getTruncatedMean();
  m_error = dedxTrack->getError();

  m_coscor = dedxTrack->getCosineCorrection();
  m_rungain = dedxTrack->getRunGain();

  // Get the vector of dE/dx values for all layers
  for (int il = 0; il < l_nhits; ++il) {
    l_nhitscombined[il] = dedxTrack->getNHitsCombined(il);
    l_wirelongesthit[il] = dedxTrack->getWireLongestHit(il);
    l_layer[il] = dedxTrack->getLayer(il);
    l_path[il] = dedxTrack->getLayerPath(il);
    l_dedx[il] = dedxTrack->getLayerDedx(il);
  }

  // Get the vector of dE/dx values for all hits
  for (int ihit = 0; ihit < h_nhits; ++ihit) {
    h_lwire[ihit] = dedxTrack->getWireInLayer(ihit);
    h_wire[ihit] = dedxTrack->getWire(ihit);
    h_layer[ihit] = dedxTrack->getHitLayer(ihit);
    h_path[ihit] = dedxTrack->getPath(ihit);
    h_dedx[ihit] = dedxTrack->getDedx(ihit);
    h_adcraw[ihit] = dedxTrack->getADCCount(ihit);
    h_doca[ihit] = dedxTrack->getDoca(ihit);
    h_enta[ihit] = dedxTrack->getEnta(ihit);
    h_driftT[ihit] = dedxTrack->getDriftT(ihit);
    h_wireGain[ihit] = dedxTrack->getWireGain(ihit);
    h_twodcor[ihit] = dedxTrack->getTwoDCorrection(ihit);
    h_onedcor[ihit] = dedxTrack->getOneDCorrection(ihit);
  }
}

void
HitLevelInfoWriterModule::clearEntries()
{

  for (int il = 0; il < 200; ++il) {
    l_nhitscombined[il] = 0;
    l_wirelongesthit[il] = 0;
    l_layer[il] = 0;
    l_path[il] = 0;
    l_dedx[il] = 0;
  }

  for (int ihit = 0; ihit < 200; ++ihit) {
    h_lwire[ihit] = 0;
    h_wire[ihit] = 0;
    h_layer[ihit] = 0;
    h_path[ihit] = 0;
    h_dedx[ihit] = 0;
    h_adcraw[ihit] = 0;
    h_doca[ihit] = 0;
    h_enta[ihit] = 0;
    h_driftT[ihit] = 0;
    h_wireGain[ihit] = 0;
    h_twodcor[ihit] = 0;
    h_onedcor[ihit] = 0;
  }
}
