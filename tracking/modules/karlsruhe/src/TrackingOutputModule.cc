/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/karlsruhe/TrackingOutputModule.h>

#include <generators/dataobjects/MCParticle.h>

#include <tracking/dataobjects/Track.h>
#include "GFTrack.h"
#include "GFTrackCand.h"

#include <framework/datastore/StoreArray.h>

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackingOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingOutputModule::TrackingOutputModule() : Module()
{
  //Set module properties
  setDescription("This is an auxiliary output module for tracking. It can be used after pattern recognition and fitting and uses the MC information, the Tracks from fitting the MC Information and the Tracks from fitting the pattern recognition tracks. However, it produces very large files. And I am also not very familiar with root trees, so that module may be far from optimal ...");

  //Parameter definition

  // names of input containers
  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles", string(""));

  addParam("MCTracksColName", m_tracksMCColName, "Name of collection holding the Tracks from MCTracking", string(""));

  addParam("PRTracksColName", m_tracksPRColName, "Name of collection holding the Tracks from MCTracking", string("Tracks_PatternReco"));

  addParam("MCGFTracksColName", m_gfTracksMCColName, "Name of collection holding the GFTracks from MCTracking", string(""));

  addParam("PRGFTracksColName", m_gfTracksPRColName, "Name of collection holding the GFTracks from MCTracking", string("GFTracks_PatternReco"));

  addParam("OutputFileName" , m_outputFileName, "Name of the created TFile name", string("TrackingOutput.root"));

}


TrackingOutputModule::~TrackingOutputModule()
{

}


void TrackingOutputModule::initialize()
{
  //initialize store arrays
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  StoreArray<Track> tracksMC(m_tracksMCColName);
  StoreArray<GFTrack> gfTracksMC(m_gfTracksMCColName);
  StoreArray<Track> tracksPR(m_tracksPRColName);
  StoreArray<GFTrack> gfTracksPR(m_gfTracksPRColName);


  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "TrackingOutput");
  m_file->SetCompressionLevel(5);         //I have no real idea what value is suitable

  m_tree = new TTree("tree", "A ROOT tree with tracking output", 1000000);  //I have also no real idea to what value the buffersize should be set

  //initialize branches

  //information about the original MCParticle
  m_tree->Branch("nMCPrimary", &m_nMCPrimary, "nMCPrimary/I");
  m_tree->Branch("mcMomentumX", &m_mcMomentumX, "mcMomentumX/F");
  m_tree->Branch("mcMomentumY", &m_mcMomentumY, "mcMomentumY/F");
  m_tree->Branch("mcMomentumZ", &m_mcMomentumZ, "mcMomentumZ/F");
  m_tree->Branch("mcMomentum", &m_mcMomentum, "mcMomentum/F");
  m_tree->Branch("mcCurv", &m_mcCurv, "mcCurv/F");
  m_tree->Branch("mcCotTheta", &m_mcCotTheta, "mcCotTheta/F");
  m_tree->Branch("mcPositionX", &m_mcPositionX, "mcPositionX/F");
  m_tree->Branch("mcPositionY", &m_mcPositionY, "mcPositionY/F");
  m_tree->Branch("mcPositionZ", &m_mcPositionZ, "mcPositionZ/F");
  m_tree->Branch("mcPDG", &m_mcPDG, "mcPDG/I");

  //information about the fitted MCTracks
  m_tree->Branch("nMCFitTracks", &m_nMCFitTracks, "nMCFitTracks/I");
  m_tree->Branch("mcStartMomentumX", &m_mcStartMomentumX, "mcStartMomentumX[nMCFitTracks]/F");
  m_tree->Branch("mcStartMomentumY", &m_mcStartMomentumY, "mcStartMomentumY[nMCFitTracks]/F");
  m_tree->Branch("mcStartMomentumZ", &m_mcStartMomentumZ, "mcStartMomentumZ[nMCFitTracks]/F");
  m_tree->Branch("mcStartMomentum", &m_mcStartMomentum, "mcStartMomentum[nMCFitTracks]/F");
  m_tree->Branch("mcStartCurv", &m_mcStartCurv, "mcStartCurv[nMCFitTracks]/F");
  m_tree->Branch("mcStartCotTheta", &m_mcStartCotTheta, "mcStartCotTheta[nMCFitTracks]/F");
  m_tree->Branch("mcStartPositionX", &m_mcStartPositionX, "mcStartPositionX[nMCFitTracks]/F");
  m_tree->Branch("mcStartPositionY", &m_mcStartPositionY, "mcStartPositionY[nMCFitTracks]/F");
  m_tree->Branch("mcStartPositionZ", &m_mcStartPositionZ, "mcStartPositionZ[nMCFitTracks]/F");

  m_tree->Branch("mcFitMomentumX", &m_mcFitMomentumX, "mcFitMomentumX[nMCFitTracks]/F");
  m_tree->Branch("mcFitMomentumY", &m_mcFitMomentumY, "mcFitMomentumY[nMCFitTracks]/F");
  m_tree->Branch("mcFitMomentumZ", &m_mcFitMomentumZ, "mcFitMomentumZ[nMCFitTracks]/F");
  m_tree->Branch("mcFitMomentum", &m_mcFitMomentum, "mcFitMomentum[nMCFitTracks]/F");
  m_tree->Branch("mcFitCurv", &m_mcFitCurv, "mcFitCurv[nMCFitTracks]/F");
  m_tree->Branch("mcFitCotTheta", &m_mcFitCotTheta, "mcFitCotTheta[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionX", &m_mcFitPositionX, "mcFitPositionX[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionY", &m_mcFitPositionY, "mcFitPositionY[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionZ", &m_mcFitPositionZ, "mcFitPositionZ[nMCFitTracks]/F");

  m_tree->Branch("nMCHits", &m_nMCHits, "nMCHits[nMCFitTracks]/I");
  m_tree->Branch("mcFitPDG", &m_mcFitPDG, "mcFitPDG[nMCFitTracks]/I");
  m_tree->Branch("mcFitChi2", &m_mcFitChi2, "mcFitChi2[nMCFitTracks]/F");
  m_tree->Branch("mcFitPValue", &m_mcFitPValue, "mcFitPvalue[nMCFitTracks]/F");
  m_tree->Branch("mcSuccessFit", &m_mcSuccessFit, "mcSuccessFit[nMCFitTracks]/I");
  m_tree->Branch("mcSuccessExtrap", &m_mcSuccessExtrap, "mcSuccessExtrap[nMCFitTracks]/I");

  m_tree->Branch("mcFitMomentumXErr", &m_mcFitMomentumXErr, "mcFitMomentumXErr[nMCFitTracks]/F");
  m_tree->Branch("mcFitMomentumYErr", &m_mcFitMomentumYErr, "mcFitMomentumYErr[nMCFitTracks]/F");
  m_tree->Branch("mcFitMomentumZErr", &m_mcFitMomentumZErr, "mcFitMomentumZErr[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionXErr", &m_mcFitPositionXErr, "mcFitPositionXErr[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionYErr", &m_mcFitPositionYErr, "mcFitPositionYErr[nMCFitTracks]/F");
  m_tree->Branch("mcFitPositionZErr", &m_mcFitPositionZErr, "mcFitPositionZErr[nMCFitTracks]/F");

  //information about the fitted PatternReco Tracks
  m_tree->Branch("nPRFitTracks", &m_nPRFitTracks, "nPRFitTracks/I");
  m_tree->Branch("prStartMomentumX", &m_prStartMomentumX, "prStartMomentumX[nPRFitTracks]/F");
  m_tree->Branch("prStartMomentumY", &m_prStartMomentumY, "prStartMomentumY[nPRFitTracks]/F");
  m_tree->Branch("prStartMomentumZ", &m_prStartMomentumZ, "prStartMomentumZ[nPRFitTracks]/F");
  m_tree->Branch("prStartMomentum", &m_prStartMomentum, "prStartMomentum[nPRFitTracks]/F");
  m_tree->Branch("prStartCurv", &m_prStartCurv, "prStartCurv[nPRFitTracks]/F");
  m_tree->Branch("prStartCotTheta", &m_prStartCotTheta, "prStartCotTheta[nPRFitTracks]/F");
  m_tree->Branch("prStartPositionX", &m_prStartPositionX, "prStartPositionX[nPRFitTracks]/F");
  m_tree->Branch("prStartPositionY", &m_prStartPositionY, "prStartPositionY[nPRFitTracks]/F");
  m_tree->Branch("prStartPositionZ", &m_prStartPositionZ, "prStartPositionZ[nPRFitTracks]/F");

  m_tree->Branch("prFitMomentumX", &m_prFitMomentumX, "prFitMomentumX[nPRFitTracks]/F");
  m_tree->Branch("prFitMomentumY", &m_prFitMomentumY, "prFitMomentumY[nPRFitTracks]/F");
  m_tree->Branch("prFitMomentumZ", &m_prFitMomentumZ, "prFitMomentumZ[nPRFitTracks]/F");
  m_tree->Branch("prFitMomentum", &m_prFitMomentum, "prFitMomentum[nPRFitTracks]/F");
  m_tree->Branch("prFitCurv", &m_prFitCurv, "prFitCurv[nPRFitTracks]/F");
  m_tree->Branch("prFitCotTheta", &m_prFitCotTheta, "prFitCotTheta[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionX", &m_prFitPositionX, "prFitPositionX[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionY", &m_prFitPositionY, "prFitPositionY[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionZ", &m_prFitPositionZ, "prFitPositionZ[nPRFitTracks]/F");

  m_tree->Branch("nPRHits", &m_nPRHits, "nPRHits[nPRFitTracks]/I");
  m_tree->Branch("prFitPDG", &m_prFitPDG, "prFitPDG[nPRFitTracks]/I");
  m_tree->Branch("prPurity", &m_prPurity, "prPurity[nPRFitTracks]/F");
  m_tree->Branch("prFitChi2", &m_prFitChi2, "prFitChi2[nPRFitTracks]/F");
  m_tree->Branch("prFitPValue", &m_prFitPValue, "prFitPValue[nPRFitTracks]/F");
  m_tree->Branch("prSuccessFit", &m_prSuccessFit, "prSuccessFit[nPRFitTracks]/I");
  m_tree->Branch("prSuccessExtrap", &m_prSuccessExtrap, "prSuccessExtrap[nPRFitTracks]/I");

  m_tree->Branch("prFitMomentumXErr", &m_prFitMomentumXErr, "prFitMomentumXErr[nPRFitTracks]/F");
  m_tree->Branch("prFitMomentumYErr", &m_prFitMomentumYErr, "prFitMomentumYErr[nPRFitTracks]/F");
  m_tree->Branch("prFitMomentumZErr", &m_prFitMomentumZErr, "prFitMomentumZErr[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionXErr", &m_prFitPositionXErr, "prFitPositionXErr[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionYErr", &m_prFitPositionYErr, "prFitPositionYErr[nPRFitTracks]/F");
  m_tree->Branch("prFitPositionZErr", &m_prFitPositionZErr, "prFitPositionZErr[nPRFitTracks]/F");

}

void TrackingOutputModule::beginRun()
{


}

void TrackingOutputModule::event()
{

  B2INFO("*******   TrackingOutputModule  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(149, "TrackingOutput: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) B2WARNING("TrackingOutput: MCParticlesCollection is empty!");

  StoreArray<Track> tracksMC(m_tracksMCColName);
  int nMcTracks = tracksMC.getEntries();
  B2DEBUG(149, "TrackingOutput: total Number of McTracks: " << nMcTracks);
  if (nMcTracks == 0) B2WARNING("TrackingOutput: TracksMCCollection is empty!");

  StoreArray<GFTrack> gfTracksMC(m_gfTracksMCColName);
  int nMcGFTracks = gfTracksMC.getEntries();
  B2DEBUG(149, "TrackingOutput: total Number of McGFTracks: " << nMcGFTracks);
  if (nMcGFTracks == 0) B2WARNING("TrackingOutput: GFTracksMCCollection is empty!");
  if (nMcTracks != nMcGFTracks) B2WARNING("Different number of Tracks and GFTracks!");

  StoreArray<Track> tracksPR(m_tracksPRColName);
  int nPrTracks = tracksPR.getEntries();
  B2DEBUG(149, "TrackingOutput: total Number of PrTracks: " << nPrTracks);
  if (nPrTracks == 0) B2WARNING("TrackingOutput: TracksPRCollection is empty!");

  StoreArray<GFTrack> gfTracksPR(m_gfTracksPRColName);
  int nPrGFTracks = gfTracksPR.getEntries();
  B2DEBUG(149, "TrackingOutput: total Number of PrGFTracks: " << nPrGFTracks);
  if (nPrGFTracks == 0) B2WARNING("TrackingOutput: GFTracksPRCollection is empty!");
  if (nPrTracks != nPrGFTracks) B2WARNING("Different number of Tracks and GFTracks!");

  m_nMCPrimary = 0 ;
  for (int i = 0; i < nMcParticles; i++) {
    if (mcParticles[i]->hasStatus(1) == true) {
      ++m_nMCPrimary;
    }
  }

  B2INFO("Tracking output: number of  primary MCParticles: " << m_nMCPrimary);

  for (int i = 0; i < nMcParticles; i++) {
    if (mcParticles[i]->hasStatus(1) == true) {
      m_mcSuccessCounter = 0;
      m_prSuccessCounter = 0;

      m_mcMomentumX = mcParticles[i]->getMomentum().x();
      m_mcMomentumY = mcParticles[i]->getMomentum().y();
      m_mcMomentumZ = mcParticles[i]->getMomentum().z();
      m_mcMomentum = sqrt(m_mcMomentumX * m_mcMomentumX + m_mcMomentumY * m_mcMomentumY + m_mcMomentumZ * m_mcMomentumZ);
      m_mcCurv = 1 / (sqrt(m_mcMomentumX * m_mcMomentumX + m_mcMomentumY * m_mcMomentumY));
      m_mcCotTheta = m_mcMomentumZ * m_mcCurv;

      m_mcPositionX = mcParticles[i]->getVertex().x();
      m_mcPositionY = mcParticles[i]->getVertex().y();
      m_mcPositionZ = mcParticles[i]->getVertex().z();

      m_mcPDG = mcParticles[i]->getPDG();

      m_mcFitTracks = getTrackIdsForMCId(m_tracksMCColName, i);
      m_prFitTracks = getTrackIdsForMCId(m_tracksPRColName, i);

      m_nMCFitTracks = m_mcFitTracks.size();
      m_nPRFitTracks = m_prFitTracks.size();

      //there should only be one mc track for an mc particle, but maybe one wants to fit with different pdg hypothesises

      //B2INFO("Collect Info for MCTrack");
      for (int j = 0; j < m_nMCFitTracks; j ++) {
        int trackId = m_mcFitTracks.at(j);
        GFTrackCand candidateMC = gfTracksMC[trackId]->getCand();

        m_mcStartMomentumX[j] = candidateMC.getDirSeed().x();
        m_mcStartMomentumY[j] = candidateMC.getDirSeed().y();
        m_mcStartMomentumZ[j] = candidateMC.getDirSeed().z();
        m_mcStartMomentum[j] = candidateMC.getDirSeed().Mag();
        m_mcStartCurv[j] = 1 / (sqrt(m_mcStartMomentumX[j] * m_mcStartMomentumX[j] + m_mcStartMomentumY[j] * m_mcStartMomentumY[j]));
        m_mcStartCotTheta[j] = m_mcStartMomentumZ[j] * m_mcStartCurv[j];

        m_mcFitMomentumX[j] = abs(1 / (tracksMC[trackId]->getKappa())) * (-sin(tracksMC[trackId]->getPhi()));
        m_mcFitMomentumY[j] = abs(1 / (tracksMC[trackId]->getKappa())) * cos(tracksMC[trackId]->getPhi());
        m_mcFitMomentumZ[j] = abs(1 / (tracksMC[trackId]->getKappa())) * (tracksMC[trackId]->getTanLambda());
        m_mcFitMomentum[j] = sqrt(m_mcFitMomentumX[j] * m_mcFitMomentumX[j] + m_mcFitMomentumY[j] * m_mcFitMomentumY[j] + m_mcFitMomentumZ[j] * m_mcFitMomentumZ[j]);
        m_mcFitCurv[j] = tracksMC[trackId]->getKappa();
        m_mcFitCotTheta[j] = m_mcFitMomentumZ[j] * m_mcFitCurv[j];

        m_nMCHits[j] = tracksMC[trackId]->getNHits();
        m_mcFitPDG[j] = tracksMC[trackId]->getPDG();
        m_mcFitChi2[j] =  tracksMC[trackId]->getChi2();
        m_mcFitPValue[j] = tracksMC[trackId]->getPValue();

        m_mcFitMomentumXErr[j] = tracksMC[trackId]->getPErrors().x();
        m_mcFitMomentumYErr[j] = tracksMC[trackId]->getPErrors().y();
        m_mcFitMomentumZErr[j] = tracksMC[trackId]->getPErrors().z();
        m_mcFitPositionXErr[j] = tracksMC[trackId]->getVertexErrors().x();
        m_mcFitPositionYErr[j] = tracksMC[trackId]->getVertexErrors().y();
        m_mcFitPositionZErr[j] = tracksMC[trackId]->getVertexErrors().z();


        if (tracksMC[trackId]->getFitFailed() == false) {
          m_mcSuccessFit[j] = 1;
        } else m_mcSuccessFit[j] = 0;

        if (tracksMC[trackId]->getExtrapFailed() == false) {
          m_mcSuccessExtrap[j] = 1;
        }

        else m_mcSuccessExtrap[j] = 0;
      }


      //B2INFO("Collect Info for PRTrack");
      if (m_nPRFitTracks > 0) {
        int trackId = -999;
        for (int j = 0; j < m_nPRFitTracks; j ++) {
          trackId = m_prFitTracks.at(j);

          GFTrackCand candidatePR = gfTracksPR[trackId]->getCand();

          m_prStartMomentumX[j] = candidatePR.getDirSeed().x();
          m_prStartMomentumY[j] = candidatePR.getDirSeed().y();
          m_prStartMomentumZ[j] = candidatePR.getDirSeed().z();
          m_prStartMomentum[j] = candidatePR.getDirSeed().Mag();
          m_prStartCurv[j] = 1 / (sqrt(m_prStartMomentumX[j] * m_prStartMomentumX[j] + m_prStartMomentumY[j] * m_prStartMomentumY[j]));
          m_prStartCotTheta[j] = m_prStartMomentumZ[j] * m_prStartCurv[j];

          m_prFitMomentumX[j] = abs(1 / (tracksPR[trackId]->getKappa())) * (-sin(tracksPR[trackId]->getPhi()));
          m_prFitMomentumY[j] = abs(1 / (tracksPR[trackId]->getKappa())) * cos(tracksPR[trackId]->getPhi());
          m_prFitMomentumZ[j] = abs(1 / (tracksPR[trackId]->getKappa())) * (tracksPR[trackId]->getTanLambda());
          m_prFitMomentum[j] = sqrt(m_prFitMomentumX[j] * m_prFitMomentumX[j] + m_prFitMomentumY[j] * m_prFitMomentumY[j] + m_prFitMomentumZ[j] * m_prFitMomentumZ[j]);
          m_prFitCurv[j] = tracksPR[trackId]->getKappa();
          m_prFitCotTheta[j] = m_prFitMomentumZ[j] * m_prFitCurv[j];

          m_nPRHits[j] = tracksPR[trackId]->getNHits();
          m_prFitPDG[j] = tracksPR[trackId]->getPDG();
          m_prPurity[j] = tracksPR[trackId]->getPurity();
          m_prFitChi2[j] =  tracksPR[trackId]->getChi2();
          m_prFitPValue[j] = tracksPR[trackId]->getPValue();

          m_prFitMomentumXErr[j] = tracksPR[trackId]->getPErrors().x();
          m_prFitMomentumYErr[j] = tracksPR[trackId]->getPErrors().y();
          m_prFitMomentumZErr[j] = tracksPR[trackId]->getPErrors().z();
          m_prFitPositionXErr[j] = tracksPR[trackId]->getVertexErrors().x();
          m_prFitPositionYErr[j] = tracksPR[trackId]->getVertexErrors().y();
          m_prFitPositionZErr[j] = tracksPR[trackId]->getVertexErrors().z();


          if (tracksPR[trackId]->getFitFailed() == false) {
            m_prSuccessFit[j] = true;
          } else m_prSuccessFit[j] = false;

          if (tracksPR[trackId]->getExtrapFailed() == false) {
            m_prSuccessExtrap[j] = true;
          } else m_prSuccessExtrap[j] = false;
        }
      } else {
        B2INFO("TrackingOutput: no PatternRecoTrack found for this particle...");
      }

      m_tree->Fill();
    }

  }


  m_tree->Write();

}

void TrackingOutputModule::endRun()
{


}

void TrackingOutputModule::terminate()
{
  //m_tree->Write();
  m_file->Close();
  if (m_file) {
    delete m_file;
  }

}

vector<int> TrackingOutputModule::getTrackIdsForMCId(string Tracks, int MCId)
{

  StoreArray<Track> tracks(Tracks.c_str());
  vector <int> trackIds;

  for (int i = 0; i < tracks.getEntries(); i++) {

    if (tracks[i]->getMCId() == MCId) {
      trackIds.push_back(i);
    }

  }

  return trackIds;


}


