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
}

HitLevelInfoWriterModule::~HitLevelInfoWriterModule() { }

void HitLevelInfoWriterModule::initialize()
{

  B2INFO("Creating a ROOT file for the hit level information...");

  // required inputs
  StoreArray<CDCDedxTrack> dedxArray;
  StoreArray<Track> tracks;
  StoreArray<TrackFitResult> trackfitResults;

  dedxArray.isRequired();
  tracks.isRequired();
  trackfitResults.isRequired();

  // register output root file
  m_file = new TFile(m_filename.c_str(), "RECREATE");
  m_tree = new TTree("track", "dE/dx information");
  m_tree->SetDirectory(0);

  // event level information
  m_tree->Branch("event", &m_eventID, "event/I");
  m_tree->Branch("run", &m_runID, "run/I");
  m_tree->Branch("numTracks", &m_numTracks, "numTracks/I");

  // track level information
  m_tree->Branch("pdg", &m_PDG, "pdg/D");
  m_tree->Branch("track", &m_trackID, "track/I");
  m_tree->Branch("nhits", &m_nhits, "nhits/I");
  m_tree->Branch("vx0", &m_vx0, "vx0/D");
  m_tree->Branch("vy0", &m_vy0, "vy0/D");
  m_tree->Branch("vz0", &m_vz0, "vz0/D");
  m_tree->Branch("phi", &m_phi, "phi/D");
  m_tree->Branch("costh", &m_cosTheta, "costh/D");
  m_tree->Branch("db", &m_d0, "db/D");
  m_tree->Branch("dz", &m_z0, "dz/D");
  m_tree->Branch("chi2", &m_chi2, "chi2/D");
  m_tree->Branch("pF", &m_p, "pF/D");
  m_tree->Branch("numLayerHits", &m_nlhits, "numLayerHits/I");
  m_tree->Branch("numGoodLayerHits", &m_nlhitsused, "numGoodLayerHits/I");
  m_tree->Branch("eopst", &m_eopst, "eopst/D"); // placeholder for Gadget

  // track level dE/dx measurement
  m_tree->Branch("mean", &m_mean, "mean/D");
  m_tree->Branch("dedx", &m_trunc, "dedx/D");
  m_tree->Branch("dedxsat", &m_trunc, "dedxsat/D"); // placeholder for Gadget
  m_tree->Branch("dedxerr", &m_error, "dedxerr/D");
  m_tree->Branch("chiPi", &m_chipi, "chiPi/D"); // placeholder for Gadget

  // layer level information
  m_tree->Branch("layer", m_layer, "layer[numLayerHits]/D");
  m_tree->Branch("layerdx", m_layerdx, "layerdx[numLayerHits]/D");
  m_tree->Branch("layerdedx", m_layerdedx, "layerdedx[numLayerHits]/D");

  // hit level information
  m_tree->Branch("hitlayer", m_hitlayer, "hitlayer[nhits]/I");
  m_tree->Branch("wire", m_wire, "wire[nhits]/I");
  m_tree->Branch("adcraw", m_adcraw, "adcraw[nhits]/D");
  m_tree->Branch("path", m_path, "path[nhits]/D");
  m_tree->Branch("dedxhit", m_dedx, "dedxhit[nhits]/D");
  m_tree->Branch("doca", m_doca, "doca[nhits]/D");
  m_tree->Branch("enta", m_enta, "enta[nhits]/D");
  m_tree->Branch("driftT", m_driftT, "driftT[nhits]/D");
}

void HitLevelInfoWriterModule::event()
{

  // inputs
  StoreArray<CDCDedxTrack> dedxArray;

  // **************************************************
  //
  //  LOOP OVER dE/dx measurements
  //
  // **************************************************

  for (int idedx = 0; idedx < dedxArray.getEntries(); idedx++) {
    //    B2INFO("Filling track " << idedx << "\t" << dedxArray[idedx]->size());
    CDCDedxTrack* dedxMeas = dedxArray[idedx];
    if (dedxMeas->size() > 100) continue;
    const Track* track = dedxMeas->getRelatedFrom<Track>();
    const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    // fill the TTree with the Track information
    fillTrack(fitResult);

    // fill the TTree with the CDCDedxTrack information
    fillDedx(dedxMeas);

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
HitLevelInfoWriterModule::fillDedx(CDCDedxTrack* dedxMeas)
{

  // clear the containers first
  clearEntries();

  m_eventID = dedxMeas->eventID();
  m_trackID = dedxMeas->trackID();

  m_cosTheta = dedxMeas->getCosTheta();

  m_PDG = dedxMeas->getPDG();

  m_nhits = dedxMeas->size();
  m_nlhits = dedxMeas->getNLayerHits();
  m_nlhitsused = dedxMeas->getNLayerHitsUsed();
  m_p = dedxMeas->getMomentum();
  if (m_PDG < 0) m_p *= -1;

  m_mean = dedxMeas->getDedx();
  m_trunc = dedxMeas->getTruncatedMean();
  m_error = dedxMeas->getError();

  // Get the vector of dE/dx values for all hits
  for (int ihit = 0; ihit < m_nhits; ++ihit) {
    m_hitlayer[ihit] = dedxMeas->getHitLayer(ihit);
    m_wire[ihit] = dedxMeas->getWire(ihit);
    m_adcraw[ihit] = dedxMeas->getADCCount(ihit);
    m_path[ihit] = dedxMeas->getDx(ihit);
    m_dedx[ihit] = dedxMeas->getDedx(ihit);
    m_doca[ihit] = dedxMeas->getDoca(ihit);
    m_enta[ihit] = dedxMeas->getEnta(ihit);
    m_driftT[ihit] = dedxMeas->getDriftT(ihit);
  }

  // Get the vector of dE/dx values for all layers
  for (int il = 0; il < m_nlhits; ++il) {
    m_layer[il] = dedxMeas->getLayer(il);
    m_layerdx[il] = dedxMeas->getLayerDist(il);
    m_layerdedx[il] = dedxMeas->getLayerDedx(il);
  }
}

void
HitLevelInfoWriterModule::clearEntries()
{

  for (int ihit = 0; ihit < 100; ++ihit) {
    m_hitlayer[ihit] = 0;
    m_wire[ihit] = 0;
    m_adcraw[ihit] = 0;
    m_path[ihit] = 0;
    m_dedx[ihit] = 0;
    m_doca[ihit] = 0;
    m_enta[ihit] = 0;
  }

  for (int il = 0; il < 100; ++il) {
    m_layer[il] = 0;
    m_layerdx[il] = 0;
    m_layerdedx[il] = 0;
  }
}
