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

  addParam("outputBaseName", m_strOutputBaseName, "Suffix for output file name", std::string("HLInfo.root"));
  addParam("particleLists", m_strParticleList, "Vector of ParticleLists to save", std::vector<std::string>());
}

HitLevelInfoWriterModule::~HitLevelInfoWriterModule() { }

void HitLevelInfoWriterModule::initialize()
{

  B2INFO("Creating a ROOT file for the hit level information...");

  // required inputs
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_eclClusters.isOptional();
  m_klmClusters.isOptional();

  // build a map to relate input strings to the right particle type
  std::map<std::string, std::string> pdgMap = {{"pi+", "211"}, {"K+", "321"}, {"mu+", "13"}, {"e+", "11"}, {"p+", "2212"}, {"deuteron", "1000010020"}};

  // if no particle lists are given, write out all tracks
  if (m_strParticleList.size() == 0) bookOutput(m_strOutputBaseName.c_str());

  // create a new output file for each particle list specified
  for (unsigned int i = 0; i < m_strParticleList.size(); i++) {
    // strip the name of the particle lists to make this work
    std::string pdg = pdgMap[m_strParticleList[i].substr(0, m_strParticleList[i].find(":"))];
    std::string filename = std::string(m_strOutputBaseName + pdg + ".root");
    bookOutput(filename);
  }
}

void HitLevelInfoWriterModule::event()
{

  StoreArray<CDCDedxTrack> dedxTracks;
  int nParticleList = m_strParticleList.size();

  // **************************************************
  //
  //  LOOP OVER dE/dx measurements for all tracks if
  //    no particle list is specified.
  //
  // **************************************************

  if (nParticleList == 0) {
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

      // fill the E/P
      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster) {
        m_eop = (eclCluster->getEnergy()) / (fitResult->getMomentum().Mag());

        // fill the muon depth
        const KLMCluster* klmCluster = eclCluster->getRelated<KLMCluster>();
        if (klmCluster) m_klmLayers = klmCluster->getLayers();
      }

      // fill the TTree with the Track information
      fillTrack(fitResult);

      // fill the TTree with the CDCDedxTrack information
      fillDedx(dedxTrack);

      // fill the TTree
      m_tree[0]->Fill();
    }
  }

  // **************************************************
  //
  //  LOOP OVER particles in the given particle lists
  //
  // **************************************************

  for (int iList = 0; iList < nParticleList; iList++) {
    // make sure the list exists and is not empty
    StoreObjPtr<ParticleList> particlelist(m_strParticleList[iList]);
    if (!particlelist or particlelist->getListSize(true) == 0) {
      B2WARNING("ParticleList " << m_strParticleList[iList] << " not found or empty, skipping");
      continue;
    }

    // loop over the particles in the list and follow the links to the
    // dE/dx information (Particle -> PIDLikelihood -> Track -> CDCDedxTrack)
    for (unsigned int iPart = 0; iPart < particlelist->getListSize(true); iPart++) {
      Particle* part = particlelist->getParticle(iPart, true);
      if (!part) {
        B2WARNING("No particles...");
        continue;
      }
      PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) {
        B2WARNING("No related PID likelihood...");
        continue;
      }
      Track* track = pid->getRelatedFrom<Track>();
      if (!track) {
        B2WARNING("No related track...");
        continue;
      }
      CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
      if (!dedxTrack) {
        B2WARNING("No related CDCDedxTrack...");
        continue;
      }
      std::string ptype = m_strParticleList[iList].substr(0, m_strParticleList[iList].find(":"));
      const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
      if (ptype != "pi+") {
        if (ptype == "K+") fitResult = track->getTrackFitResultWithClosestMass(Const::kaon);
        else if (ptype == "p+") fitResult = track->getTrackFitResultWithClosestMass(Const::proton);
        else if (ptype == "deuteron") fitResult = track->getTrackFitResultWithClosestMass(Const::deuteron);
        else if (ptype == "mu+") fitResult = track->getTrackFitResultWithClosestMass(Const::muon);
        else if (ptype == "e+") fitResult = track->getTrackFitResultWithClosestMass(Const::electron);
      }
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

      // fill the E/P
      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster) {
        m_eop = (eclCluster->getEnergy()) / (fitResult->getMomentum().Mag());

        // fill the muon depth
        const KLMCluster* klmCluster = eclCluster->getRelated<KLMCluster>();
        if (klmCluster) m_klmLayers = klmCluster->getLayers();
      }

      // fill the TTree with the Track information
      fillTrack(fitResult);

      // fill the TTree with the CDCDedxTrack information
      fillDedx(dedxTrack);

      // fill the TTree
      m_tree[iList]->Fill();
    }
  }
}

void HitLevelInfoWriterModule::terminate()
{

  for (unsigned int i = 0; i < m_file.size(); i++) {
    B2INFO("Done writing out the hit level information...\t" << m_tree[i]->GetEntries() << " tracks");

    // write the ttree to a root file
    m_file[i]->cd();
    m_tree[i]->Write();
    m_file[i]->Close();
  }
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

  m_chie = dedxTrack->getChi(0);
  m_chimu = dedxTrack->getChi(1);
  m_chipi = dedxTrack->getChi(2);
  m_chik = dedxTrack->getChi(3);
  m_chip = dedxTrack->getChi(4);
  m_chid = dedxTrack->getChi(5);

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

void
HitLevelInfoWriterModule::bookOutput(std::string filename)
{
  // register output root file
  m_file.push_back(new TFile(filename.c_str(), "RECREATE"));
  m_tree.push_back(new TTree("track", "dE/dx information"));

  int i = m_tree.size() - 1;
  m_tree[i]->SetDirectory(0);

  // event level information (from emd)
  m_tree[i]->Branch("exp", &m_expID, "exp/I");
  m_tree[i]->Branch("run", &m_runID, "run/I");
  m_tree[i]->Branch("event", &m_eventID, "event/I");

  // track level information (from tfr)
  m_tree[i]->Branch("phi", &m_phi, "phi/D");
  m_tree[i]->Branch("vx0", &m_vx0, "vx0/D");
  m_tree[i]->Branch("vy0", &m_vy0, "vy0/D");
  m_tree[i]->Branch("vz0", &m_vz0, "vz0/D");
  m_tree[i]->Branch("db", &m_d0, "db/D");
  m_tree[i]->Branch("dz", &m_z0, "dz/D");
  m_tree[i]->Branch("chi2", &m_chi2, "chi2/D");

  // track level information (from cdt)
  m_tree[i]->Branch("track", &m_trackID, "track/I");
  m_tree[i]->Branch("length", &m_length, "length/D");
  m_tree[i]->Branch("charge", &m_charge, "charge/I");
  m_tree[i]->Branch("costh", &m_cosTheta, "costh/D");
  m_tree[i]->Branch("pF", &m_p, "pF/D");
  m_tree[i]->Branch("pdg", &m_PDG, "pdg/D");
  m_tree[i]->Branch("ioasym", &m_ioasym, "ioasym/D");

  // other track level information
  m_tree[i]->Branch("eop", &m_eop, "eop/D");
  m_tree[i]->Branch("klmLayers", &m_klmLayers, "klmLayers/I");

  // calibration constants
  m_tree[i]->Branch("scale", &m_scale, "scale/D");
  m_tree[i]->Branch("coscor", &m_cosCor, "coscor/D");
  m_tree[i]->Branch("rungain", &m_runGain, "rungain/D");

  // track level dE/dx measurements
  m_tree[i]->Branch("mean", &m_mean, "mean/D");
  m_tree[i]->Branch("dedx", &m_trunc, "dedx/D");
  m_tree[i]->Branch("dedxsat", &m_trunc, "dedxsat/D"); // placeholder for Widget
  m_tree[i]->Branch("dedxerr", &m_error, "dedxerr/D");

  // PID values
  m_tree[i]->Branch("chiE", &m_chie, "chiE/D");
  m_tree[i]->Branch("chiMu", &m_chimu, "chiMu/D");
  m_tree[i]->Branch("chiPi", &m_chipi, "chiPi/D");
  m_tree[i]->Branch("chiK", &m_chik, "chiK/D");
  m_tree[i]->Branch("chiP", &m_chip, "chiP/D");
  m_tree[i]->Branch("chiD", &m_chid, "chiD/D");

  // layer level information
  m_tree[i]->Branch("lNHits", &l_nhits, "lNHits/I");
  m_tree[i]->Branch("lNHitsUsed", &l_nhitsused, "lNHitsUsed/I");
  m_tree[i]->Branch("lNHitsCombined", l_nhitscombined, "lNHitsCombined[lNHits]/I");
  m_tree[i]->Branch("lWireLongestHit", l_wirelongesthit, "lWireLongestHit[lNHits]/I");
  m_tree[i]->Branch("lLayer", l_layer, "lLayer[lNHits]/I");
  m_tree[i]->Branch("lPath", l_path, "lPath[lNHits]/D");
  m_tree[i]->Branch("lDedx", l_dedx, "lDedx[lNHits]/D");

  // hit level information
  m_tree[i]->Branch("hNHits", &h_nhits, "hNHits/I");
  m_tree[i]->Branch("hLWire", h_lwire, "hLWire[hNHits]/I");
  m_tree[i]->Branch("hWire", h_wire, "hWire[hNHits]/I");
  m_tree[i]->Branch("hLayer", h_layer, "hLayer[hNHits]/I");
  m_tree[i]->Branch("hPath", h_path, "hPath[hNHits]/D");
  m_tree[i]->Branch("hDedx", h_dedx, "hDedx[hNHits]/D");
  m_tree[i]->Branch("hADCRaw", h_adcraw, "hADCRaw[hNHits]/D");
  m_tree[i]->Branch("hDoca", h_doca, "hDoca[hNHits]/D");
  m_tree[i]->Branch("hEnta", h_enta, "hEnta[hNHits]/D");
  m_tree[i]->Branch("hDriftT", h_driftT, "hDriftT[hNHits]/D");
  m_tree[i]->Branch("hWireGain", h_wireGain, "hWireGain[hNHits]/D");
  m_tree[i]->Branch("hTwodcor", h_twodCor, "hTwodcor[hNHits]/D");
  m_tree[i]->Branch("hOnedcor", h_onedCor, "hOnedcor[hNHits]/D");
}
