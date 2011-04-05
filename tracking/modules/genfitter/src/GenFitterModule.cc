/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitterModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>
#include <tracking/dataobjects/Track.h>

#include <tracking/karlsruhe/MCMatchParticle.h>

#include "GFTrack.h"
#include "GFKalman.h"
//#include "GFDaf.h"

#include "GFAbsTrackRep.h"
#include "RKTrackRep.h"

#include "GFConstField.h"
#include "GFFieldManager.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter)

GenFitterModule::GenFitterModule() :
    Module()
{

  setDescription("Uses GenFit to fit tracks within the CDC. Can fit the MCTracks or Tracks provided by the CDCTrackingModule.");

  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  addParam("TracksColName", m_tracksCollectionName, "Name of collection holding the Tracks (for MCTracks: should be created by MCTrackFinderModule and filled with 'content' after the fit, for recoTracks: will be created after the fit)", string("Tracks"));
  addParam("TrackToCDCRecoHitColName",  m_trackToCDCRecoHitCollectionName, "Name of collection holding the relations between Tracks and CDCRecoHits (should be created by MCTrackFinderModule)", string("TrackToCDCRecoHits"));
  addParam("TrackToMCParticleColName", m_trackToMCParticleCollectionName, "Name of collection holding the relations between Tracks and MCParticles (should be created by MCTrackFinderModule)", string("TrackToMCParticle"));

  addParam("CDCTrackCandidatesColName", m_cdcTrackCandsCollectionName, "Name of collection holding the CDCTrackCandidates (output of the pattern recognition, should be created by CDCTrackingModule)", string("CDCTrackCandidates"));
  addParam("CDCTrackCandsToCDCRecoHitsColName", m_cdcTrackCandToRecoHitsCollectionName, "Name of collection holding the relations between CDCTrackCandidates and CDCRecoHits (should be created by CDCTrackingModule)", string("CDCTrackCandidatesToCDCRecoHits"));
  addParam("CDCTrackCandsToMCParticlesColName", m_cdcTrackCandToMCParticleCollectionName, "Relation of CDCTrackCandidates to MCParticles (should be created by CDCMCMatchingModule)", string("CDCTrackCandidateToMCParticle"));

  addParam("MCMatchParticlesColName", m_mcMatchParticlesCollectionName, "Name of collection holding the MCMatchParticles (should be created by CDCMCMatchingModule)", string("MCMatchParticles"));

  addParam("FitMCTracks", m_fitMCTracks, "True if MC tracks should be fitted", true);
  addParam("FitRecoTracks", m_fitRecoTracks, "True if track candidates from pattern recognition should be fitted", false);
}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{

}

void GenFitterModule::beginRun()
{

}

void GenFitterModule::event()
{
  B2INFO("**********   GenFitterModule  ************");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  B2INFO("GenFitter: total Number of MCParticles: " << mcParticles.GetEntries());
  if (mcParticles.GetEntries() == 0) B2WARNING("GenFitter: MCParticlesCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  B2INFO("GenFitter: Number of CDCRecoHits: " << cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("GenFitter: CDCRecoHitsCollection is empty!");

  //The use of MCMatchParticles is optional, the fitting of MCtruth should work without CDCMCMatchingModule
  StoreArray<MCMatchParticle> mcMatchParticles(m_mcMatchParticlesCollectionName);

  //Give Genfit the magnetic field, should come from the common database later...
  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

  if (m_fitMCTracks == true) {

    B2INFO("-------   Fitting using true MCparticle tracks  ---------------");

    StoreArray<Track> tracks(m_tracksCollectionName);
    B2INFO("GenFitter: Number of Tracks: " << tracks.GetEntries());
    if (tracks.GetEntries() == 0) B2WARNING("GenFitter: TracksCollection is empty!");

    StoreArray<Relation> trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
    //B2INFO("GenFitter: Number of relations between Tracks and CDCRecoHits: "<<trackToCDCRecoHits.GetEntries());
    if (trackToCDCRecoHits.GetEntries() == 0) B2WARNING("GenFitter: TrackToCDCRecoHitsCollection is empty!");

    StoreArray<Relation> trackToMCPart(m_trackToMCParticleCollectionName);
    //B2INFO("GenFitter: Number of relations between Tracks and MCParticles: "<<trackToMCPart.GetEntries());
    if (trackToMCPart.GetEntries() == 0) B2WARNING("GenFitter: TrackToMCParticlesCollection is empty!");

    //loop over all tracks
    for (int i = 0; i < tracks->GetEntries(); i++) {
      B2INFO("#### Fit track Nr. : " << i);
      //get the index of the MCParticle which produced this track
      int mcindex = trackToMCPart[i]->getToIndex();

      //get fit starting values from the MCParticle
      TVector3 vertex = mcParticles[mcindex]->getProductionVertex();
      TVector3 momentum = mcParticles[mcindex]->getMomentum();
      int pdg = mcParticles[mcindex]->getPDG();

      //B2INFO("MCIndex: "<<mcindex);
      B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z());
      B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
      B2INFO("Start values: pdg:      " << pdg);

      //Now create a GenFit track with this representation
      GFAbsTrackRep* trackRep = new RKTrackRep(vertex, momentum, pdg);
      GFTrack track(trackRep);

      //Collect all RecoHits for this track
      //Caution: for the moment RecoHits are not sorted, but GenFit needs them in a certain order.
      //It seems that that order is by chance correct for simple single tracks, but the fit may fail for more complicated events.
      list<short unsigned int> indexList = trackToCDCRecoHits[i]->getToIndices();
      for (int j = 0; j < cdcRecoHits->GetEntries(); j++) {
        int counter = -1;
        // list<short unsigned int>::iterator iter;
        // for (iter = indexList.begin(); iter != indexList.end(); iter++) {
        BOOST_FOREACH(int hit, indexList) {
          if (j == hit) {
            //B2INFO("Add Reco Hit");
            counter++;
            track.addHit(cdcRecoHits[j], 0, counter);
          }
        }
      }//end loop over RecoHits

      B2INFO("Number of hits assigned to the track to be fitted: " << track.getNumHits());

      //Initialize fitting algorithm and process track
      GFKalman k;
      //GFDaf daf;
      //k.setNumIterations(1);
      k.processTrack(&track);
      //track.Print();
      B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z());
      B2INFO("----> Chi2 of the fit: " << track.getChiSqu());

      //if the MCMatchParticles were created, assign the fitted momentum
      if (mcMatchParticles->GetEntries() != 0) {
        mcMatchParticles[mcindex]->setFitMCMomentum(track.getMom());
        mcMatchParticles[mcindex]->setChi2(track.getChiSqu());
      }

      track.releaseHits();

    }//end loop over tracks

  }//endif m_fitMCTracks


  if (m_fitRecoTracks == true) {

    B2INFO("-------   Fitting using CDCTrackCandidates from pattern recogition  ---------------");

    StoreArray<CDCTrackCandidate> trackCandidates(m_cdcTrackCandsCollectionName);
    B2INFO("GenFitter: Number of TrackCandidates: " << trackCandidates.GetEntries());
    if (trackCandidates.GetEntries() == 0) B2WARNING("GenFitter: CDCTrackCandidatesCollection is empty!");

    StoreArray<Relation> cdcTrackCandToCDCRecoHits(m_cdcTrackCandToRecoHitsCollectionName);
    //B2INFO("GenFitter: Number of relations between Tracks and CDCRecoHits: "<<cdcTrackCandToCDCRecoHits.GetEntries());
    if (cdcTrackCandToCDCRecoHits.GetEntries() == 0) B2WARNING("GenFitter: CDCTrackCandToCDCRecoHitsCollection is empty!");

    StoreArray<Relation> cdcTrackCandToMCPart(m_cdcTrackCandToMCParticleCollectionName);
    //B2INFO("GenFitter: Number of relations between Tracks and MCParticles: "<<cdcTrackCandToMCPart.GetEntries());
    if (cdcTrackCandToMCPart.GetEntries() == 0) B2WARNING("GenFitter: CDCTrackCandToMCParticleCollection is empty!");

    //loop over all tracks
    for (int i = 0; i < trackCandidates->GetEntries(); i++) {
      B2INFO("#### Fit track Nr. : " << i);
      //get start values for the fit
      TVector3 vertex(0., 0., 0.);
      TVector3 momentum = trackCandidates[i]->getMomentumVector();
      int pdg = trackCandidates[i]->getChargeSign() * 211;

      //get the index of the MCParticle which was matched to produce this TrackCandidate
      int mcindex = cdcTrackCandToMCPart[i]->getToIndex();

      //get values from true mcparticle for comparison
      TVector3 trueVertex = mcParticles[mcindex]->getProductionVertex();
      TVector3 trueMomentum = mcParticles[mcindex]->getMomentum();
      int truePdg = mcParticles[mcindex]->getPDG();

      B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " (true: " << trueMomentum.x() << "  " << trueMomentum.y() << "  " << trueMomentum.z() << " )");
      B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z() << " (true: " << trueVertex.x() << "  " << trueVertex.y() << "  " << trueVertex.z() << " )");
      B2INFO("Start values: pdg:      " << pdg << " (true: " << truePdg << " )");

      //Now create a GenFit track with this representation
      GFAbsTrackRep* trackRep = new RKTrackRep(vertex, momentum, pdg);
      GFTrack track(trackRep);

      //Collect all RecoHits for this track
      //Caution: for the moment RecoHits are not sorted, but GenFit needs them in a certain order.
      //It seems that that order is by chance correct for simple single tracks, but the fit may fail for more complicated events.
      list<short unsigned int> indexList = cdcTrackCandToCDCRecoHits[i]->getToIndices();
      for (int j = 0; j < cdcRecoHits->GetEntries(); j++) {
        int counter = -1;
        BOOST_FOREACH(int hit, indexList) {
          if (j == hit) {
            counter++;
            track.addHit(cdcRecoHits[j], 0, counter);
          }
        }
      }//end loop over RecoHits

      B2INFO("Number of hits assigned to the track to be fitted: " << track.getNumHits());

      //Initialize fitting algorithm and process track
      GFKalman k;
      k.processTrack(&track);
      B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z());
      B2INFO("----> Chi2 of the fit: " << track.getChiSqu());

      //if the MCMatchParticles were created, assign the fitted momentum
      if (mcMatchParticles->GetEntries() != 0) {
        mcMatchParticles[mcindex]->setFitRecoMomentum(track.getMom());
      }

      track.releaseHits();

    }//end loop over tracks

  }//endif m_recoMCTracks


}

void GenFitterModule::endRun()
{

}

void GenFitterModule::terminate()
{

}



