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
  m_tree->Branch("ioasym", &m_ioasym, "ioasym/D");

  // calibration constants
  m_tree->Branch("scale", &m_scale, "scale/D");
  m_tree->Branch("coscor", &m_cosCor, "coscor/D");
  m_tree->Branch("rungain", &m_runGain, "rungain/D");

  // track level dE/dx measurements
  m_tree->Branch("mean", &m_mean, "mean/D");
  m_tree->Branch("dedx", &m_trunc, "dedx/D");
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
  m_tree->Branch("hTwodcor", h_twodCor, "hTwodcor[hNHits]/D");
  m_tree->Branch("hOnedcor", h_onedCor, "hOnedcor[hNHits]/D");

}

void HitLevelInfoWriterModule::event()
{

  StoreArray<CDCDedxTrack> dedxTracks;

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

    // fill the TTree
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

  m_mean = dedxTrack->getDedxMean();
  m_trunc = dedxTrack->getDedx();
  m_error = dedxTrack->getDedxError();

  // Get the calibration constants
  m_scale = m_DBScaleFactor->getScaleFactor();
  m_runGain = m_DBRunGain->getRunGain();
  m_cosCor = m_DBCosineCor->getMean(m_cosTheta);

  // Get the vector of dE/dx values for all layers
  double lout = 0, lin = 0, increment = 0;
  int lastlayer = 0;
  for (int il = 0; il < l_nhits; ++il) {
    l_nhitscombined[il] = dedxTrack->getNHitsCombined(il);
    l_wirelongesthit[il] = dedxTrack->getWireLongestHit(il);
    l_layer[il] = dedxTrack->getLayer(il);
    l_path[il] = dedxTrack->getLayerPath(il);
    l_dedx[il] = dedxTrack->getLayerDedx(il);

    if (l_layer[il] > lastlayer) lout++;
    else if (l_layer[il] < lastlayer) lin++;
    else continue;

    lastlayer = l_layer[il];
    increment++;
  }
  m_ioasym = (lout - lin) / increment;

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

    // Get the calibration constants
    h_wireGain[ihit] = m_DBWireGains->getWireGain(h_wire[ihit]);
    h_twodCor[ihit] = m_DB2DCell->getMean(h_layer[ihit], h_doca[ihit], h_enta[ihit]);
    h_onedCor[ihit] = m_DB1DCell->getMean(h_layer[ihit], h_enta[ihit]);
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
    h_twodCor[ihit] = 0;
    h_onedCor[ihit] = 0;
  }
}
