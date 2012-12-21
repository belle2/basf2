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

#include <tracking/dataobjects/TrackingOutput.h>

#include <framework/datastore/StoreArray.h>


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
  setDescription("This is an auxiliary output module for tracking. It can be used after pattern recognition and fitting and uses the MC information, the Tracks from fitting the MC Information and the Tracks from fitting the pattern recognition tracks. The results are stored in a TrackingOutput objects, the collection can be written out using SimpleOutputModule.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition

  // names of input containers
  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles", string(""));

  addParam("MCTracksColName", m_tracksMCColName, "Name of collection holding the Tracks from MCTracking", string(""));

  addParam("PRTracksColName", m_tracksPRColName, "Name of collection holding the Tracks from MCTracking", string("Tracks_PatternReco"));

  addParam("MCGFTracksColName", m_gfTracksMCColName, "Name of collection holding the GFTracks from MCTracking", string(""));

  addParam("PRGFTracksColName", m_gfTracksPRColName, "Name of collection holding the GFTracks from MCTracking", string("GFTracks_PatternReco"));

  //choose for which particles the output should be created (you should use the same particles as in MCTrackFinder to get a fair comparison)
  addParam("WhichParticles", m_whichParticles, "Select for which particles output should be created: 0 for all primaries, 1 for tracks which created hits in the PXD, 2 for tracks which created hits in the SVD, 3 for tracks which created hits in the CDC", int(0));
  addParam("EnergyCut", m_energyCut, "Track Candidates are only created for MCParticles with energy larger than this cut ", double(0.1));
  addParam("Neutrals", m_neutrals, "Set true if track candidates should be created also for neutral particles", bool(true));

  //name of output container
  addParam("OutputCollectionName" , m_outputCollectionName, "Name of the created TrackingOutput collection", string(""));

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

  StoreArray<TrackingOutput> output(m_outputCollectionName);

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

  //output storeArray
  StoreArray<TrackingOutput> output(m_outputCollectionName);

  //set the proper status
  int status = 0;
  if (m_whichParticles == 0) status = MCParticle::c_PrimaryParticle;   //primaries
  if (m_whichParticles == 1) status = MCParticle::c_SeenInPXD;  //seen in PXD
  if (m_whichParticles == 2) status = MCParticle::c_SeenInSVD;  //seen in SVD
  if (m_whichParticles == 3) status = MCParticle::c_SeenInCDC;  //seen in CDC
  if (m_whichParticles > 3 || m_whichParticles < 0) {
    B2WARNING("Invalid parameter! Track Candidates for primary particles will be created.")
    status = 1;
  }

  //an auxiliary variable to discard neutrals if necessary (assume that no particles with charge -999 exist)
  float forbiddenCharge = -999;
  if (m_neutrals == false) forbiddenCharge = 0;

  m_nMCPart = 0 ;
  for (int i = 0; i < nMcParticles; i++) {
    if (mcParticles[i]->getEnergy() > m_energyCut && mcParticles[i]->hasStatus(status) == true && mcParticles[i]->getPDG() < 100000000 && mcParticles[i]->getCharge() != forbiddenCharge) {
      ++m_nMCPart;
    }
  }

  B2INFO("Tracking output: number of interesting MCParticles: " << m_nMCPart);
  double alpha = 1 / (1.5 * 0.00299792458); //assume constant 1.5 T magnetic field to recalculate the momentum

  int counter = -1;

  for (int i = 0; i < nMcParticles; i++) {
    if (mcParticles[i]->getEnergy() > m_energyCut && mcParticles[i]->hasStatus(status) == true && mcParticles[i]->getPDG() < 100000000 && mcParticles[i]->getCharge() != forbiddenCharge) {
      counter++;

      m_mcSuccessCounter = 0;
      m_prSuccessCounter = 0;

      //create an output object to store the information
      new(output->AddrAt(counter)) TrackingOutput();

      output[counter]->setMCMomentumX(mcParticles[i]->getMomentum().x());
      output[counter]->setMCMomentumY(mcParticles[i]->getMomentum().y());
      output[counter]->setMCMomentumZ(mcParticles[i]->getMomentum().z());
      output[counter]->setMCMomentum(mcParticles[i]->getMomentum().Mag());
      output[counter]->setMCCurv(1 / sqrt(mcParticles[i]->getMomentum().x()*mcParticles[i]->getMomentum().x() + mcParticles[i]->getMomentum().y()*mcParticles[i]->getMomentum().y()));
      output[counter]->setMCPhi(atan2(output[counter]->getMCMomentumY(), output[counter]->getMCMomentumX()));
      output[counter]->setMCCotTheta(output[counter]->getMCMomentumZ() * output[counter]->getMCCurv());
      output[counter]->setMCTheta(atan2(1 / (output[counter]->getMCCurv()), output[counter]->getMCMomentumZ()));

      output[counter]->setMCPositionX(mcParticles[i]->getVertex().x());
      output[counter]->setMCPositionY(mcParticles[i]->getVertex().y());
      output[counter]->setMCPositionZ(mcParticles[i]->getVertex().z());

      output[counter]->setMCPDG(mcParticles[i]->getPDG());


      m_mcFitTracks = getTrackIdsForMCId(m_tracksMCColName, i);

      m_prFitTracks = getTrackIdsForMCId(m_tracksPRColName, i);

      output[counter]->setMCFitTracks(m_mcFitTracks);
      output[counter]->setPRFitTracks(m_prFitTracks);

      m_nMCFitTracks = m_mcFitTracks.size();
      m_nPRFitTracks = m_prFitTracks.size();

      output[counter]->setNMCFitTracks(m_mcFitTracks.size());
      output[counter]->setNPRFitTracks(m_prFitTracks.size());

      //there should only be one mc track for an mc particle, but maybe one wants to fit with different pdg hypothesises

      //B2INFO("Collect Info for MCTrack ( "<<m_nMCFitTracks<<" tracks found )");
      for (int j = 0; j < m_nMCFitTracks; j ++) {
        int trackId = m_mcFitTracks.at(j);
        GFTrackCand candidateMC = gfTracksMC[trackId]->getCand();

        output[counter]->setMCStartMomentumX(candidateMC.getMomSeed().x());
        output[counter]->setMCStartMomentumY(candidateMC.getMomSeed().y());
        output[counter]->setMCStartMomentumZ(candidateMC.getMomSeed().z());
        output[counter]->setMCStartMomentum(candidateMC.getMomSeed().Mag());
        output[counter]->setMCStartCurv(1 / sqrt(candidateMC.getMomSeed().x()*candidateMC.getMomSeed().x() + candidateMC.getMomSeed().y()*candidateMC.getMomSeed().y()));
        output[counter]->setMCStartPhi(atan2(output[counter]->getMCStartMomentumY().at(j), output[counter]->getMCStartMomentumX().at(j)));
        output[counter]->setMCStartCotTheta(output[counter]->getMCStartMomentumZ().at(j) * output[counter]->getMCStartCurv().at(j));
        output[counter]->setMCStartTheta(atan2((1 / (output[counter]->getMCStartCurv().at(j))), output[counter]->getMCStartMomentumZ().at(j)));

        output[counter]->setMCFitMomentumX(tracksMC[trackId]->getMomentum().x());
        output[counter]->setMCFitMomentumY(tracksMC[trackId]->getMomentum().y());
        output[counter]->setMCFitMomentumZ(tracksMC[trackId]->getMomentum().z());
        output[counter]->setMCFitMomentum(tracksMC[trackId]->getMomentum().Mag());

        output[counter]->setMCFitPositionX(tracksMC[trackId]->getD0()*sin(tracksMC[trackId]->getPhi()));
        output[counter]->setMCFitPositionY(-tracksMC[trackId]->getD0()*cos(tracksMC[trackId]->getPhi()));
        output[counter]->setMCFitPositionZ(tracksMC[trackId]->getZ0());

        output[counter]->setMCFitCurv(abs(tracksMC[trackId]->getOmega()*alpha));
        output[counter]->setMCFitPhi(atan2(output[counter]->getMCFitMomentumY().at(j), output[counter]->getMCFitMomentumX().at(j)));
        output[counter]->setMCFitCotTheta(output[counter]->getMCFitMomentumZ().at(j) * output[counter]->getMCFitCurv().at(j));
        output[counter]->setMCFitTheta(atan2(1 / (output[counter]->getMCFitCurv().at(j)), output[counter]->getMCFitMomentumZ().at(j)));

        output[counter]->setNMCHits(tracksMC[trackId]->getNHits());
        output[counter]->setMCFitPDG(tracksMC[trackId]->getPDG());
        output[counter]->setMCFitChi2(tracksMC[trackId]->getChi2());
        output[counter]->setMCFitPValue(tracksMC[trackId]->getPValue());

        output[counter]->setMCFitMomentumXErr(tracksMC[trackId]->getPErrors().x());
        output[counter]->setMCFitMomentumYErr(tracksMC[trackId]->getPErrors().y());
        output[counter]->setMCFitMomentumZErr(tracksMC[trackId]->getPErrors().z());
        output[counter]->setMCFitPositionXErr(tracksMC[trackId]->getVertexErrors().x());
        output[counter]->setMCFitPositionYErr(tracksMC[trackId]->getVertexErrors().y());
        output[counter]->setMCFitPositionZErr(tracksMC[trackId]->getVertexErrors().z());


        if (tracksMC[trackId]->getFitFailed() == false) {
          output[counter]->setMCSuccessFit(1);

        } else output[counter]->setMCSuccessFit(0);

        if (tracksMC[trackId]->getExtrapFailed() == false) {
          output[counter]->setMCSuccessExtrap(1);
        }

        else output[counter]->setMCSuccessExtrap(0);
      }


      //B2INFO("Collect Info for PRTrack( "<<m_nPRFitTracks<<" tracks found )");
      if (m_nPRFitTracks > 0) {
        int trackId = -999;
        for (int j = 0; j < m_nPRFitTracks; j ++) {
          trackId = m_prFitTracks.at(j);

          GFTrackCand candidatePR = gfTracksPR[trackId]->getCand();


          output[counter]->setPRStartMomentumX(candidatePR.getMomSeed().x());
          output[counter]->setPRStartMomentumY(candidatePR.getMomSeed().y());
          output[counter]->setPRStartMomentumZ(candidatePR.getMomSeed().z());
          output[counter]->setPRStartMomentum(candidatePR.getMomSeed().Mag());
          output[counter]->setPRStartCurv(1 / sqrt(candidatePR.getMomSeed().x()*candidatePR.getMomSeed().x() + candidatePR.getMomSeed().y()*candidatePR.getMomSeed().y()));
          output[counter]->setPRStartPhi(atan2(output[counter]->getPRStartMomentumY().at(j), output[counter]->getPRStartMomentumX().at(j)));
          output[counter]->setPRStartCotTheta(output[counter]->getPRStartMomentumZ().at(j) * output[counter]->getPRStartCurv().at(j));
          output[counter]->setPRStartTheta(atan2(1 / (output[counter]->getPRStartCurv().at(j)), output[counter]->getPRStartMomentumZ().at(j)));


          output[counter]->setPRFitMomentumX(tracksPR[trackId]->getMomentum().x());
          output[counter]->setPRFitMomentumY(tracksPR[trackId]->getMomentum().y());
          output[counter]->setPRFitMomentumZ(tracksPR[trackId]->getMomentum().z());
          output[counter]->setPRFitMomentum(tracksPR[trackId]->getMomentum().Mag());
          output[counter]->setPRFitCurv(abs(tracksPR[trackId]->getOmega()*alpha));
          output[counter]->setPRFitPhi(atan2(output[counter]->getPRFitMomentumY().at(j), output[counter]->getPRFitMomentumX().at(j)));
          output[counter]->setPRFitCotTheta(output[counter]->getPRFitMomentumZ().at(j) * output[counter]->getPRFitCurv().at(j));
          output[counter]->setPRFitTheta(atan2(1 / output[counter]->getPRFitCurv().at(j), output[counter]->getPRFitMomentumZ().at(j)));

          output[counter]->setPRFitPositionX(tracksPR[trackId]->getD0()*sin(tracksPR[trackId]->getPhi()));
          output[counter]->setPRFitPositionY(-tracksPR[trackId]->getD0()*cos(tracksPR[trackId]->getPhi()));
          output[counter]->setPRFitPositionZ(tracksPR[trackId]->getZ0());

          output[counter]->setNPRHits(tracksPR[trackId]->getNHits());
          output[counter]->setPRFitPDG(tracksPR[trackId]->getPDG());
          output[counter]->setPRFitChi2(tracksPR[trackId]->getChi2());
          output[counter]->setPRFitPValue(tracksPR[trackId]->getPValue());

          output[counter]->setPRPurity(tracksPR[trackId]->getPurity());

          output[counter]->setPRFitMomentumXErr(tracksPR[trackId]->getPErrors().x());
          output[counter]->setPRFitMomentumYErr(tracksPR[trackId]->getPErrors().y());
          output[counter]->setPRFitMomentumZErr(tracksPR[trackId]->getPErrors().z());
          output[counter]->setPRFitPositionXErr(tracksPR[trackId]->getVertexErrors().x());
          output[counter]->setPRFitPositionYErr(tracksPR[trackId]->getVertexErrors().y());
          output[counter]->setPRFitPositionZErr(tracksPR[trackId]->getVertexErrors().z());


          if (tracksPR[trackId]->getFitFailed() == false) {
            output[counter]->setPRSuccessFit(1);
          } else output[counter]->setPRSuccessFit(0);

          if (tracksPR[trackId]->getExtrapFailed() == false) {
            output[counter]->setPRSuccessExtrap(1);
          } else output[counter]->setPRSuccessExtrap(0);
        }
      } else {
        //B2INFO("TrackingOutput: no PatternRecoTrack found for this particle...");
      }

    }


  }

}

void TrackingOutputModule::endRun()
{

}

void TrackingOutputModule::terminate()
{


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


