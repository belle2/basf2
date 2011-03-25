/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/karlsruhe/CDCMCMatchingModule.h>
//#include <tracking/modules/karlsruhe/include/CDCMCMatchingModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>
#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/cdcConformalTracking/CDCSegment.h>
#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <boost/foreach.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCMCMatching)

CDCMCMatchingModule::CDCMCMatchingModule() :
    Module()
{
  setDescription("Matches the CDCTrackCandidates with MCTruth to evaluate the performance of the pattern recognition (CDCTrackingModule).");

  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits (should be created during the simulation within CDCSensitiveDetector)", string("MCPartToCDCSimHits"));
  addParam("CDCSimHitToCDCHitColName", m_cdcSimHitToCDCHitCollectioName, "Name of collection holding the relations between CDCSimHits and CDCHits (CDCHit index = CDCRecoHit index) (should be created by CDCDigi module)", string("SimHitToCDCHits"));

  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  addParam("CDCTrackCandidatesColName", m_cdcTrackCandsCollectionName, "Name of collection holding the CDCTrackCandidates (output of the pattern recognition, should be created by CDCTrackingModule)", string("CDCTrackCandidates"));
  addParam("CDCTrackCandsToCDCRecoHitsColName", m_cdcTrackCandsToRecoHits, "Name of collection holding the relations between CDCTrackCandidates and CDCRecoHits (should be created by CDCTrackingModule)", string("CDCTrackCandidatesToCDCRecoHits"));

  addParam("CDCTrackCandsToMCParticlesColName", m_cdcTrackCandsToMCParticles, "Name of collection holding the relations between the CDCTrackCandidates and the matched MCParticles (output of this module)", string("CDCTrackCandidateToMCParticle"));

}

CDCMCMatchingModule::~CDCMCMatchingModule()
{
}

void CDCMCMatchingModule::initialize()
{

}

void CDCMCMatchingModule::beginRun()
{

}

void CDCMCMatchingModule::event()
{

  B2INFO("**********   CDCMCMatchingModule  ************");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  //B2INFO("CDCMCMatching: total Number of MCParticles: "<<mcParticles.GetEntries());
  if (mcParticles.GetEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesCollection is empty!");

  StoreArray<Relation>   mcPartToSimHits(m_mcPartToCDCSimHitsCollectionName);
  //B2INFO("CDCMCMatching: Number of relations between MCParticles and SimHits: "<<mcPartToSimHits.GetEntries());
  if (mcPartToSimHits.GetEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesToSimHitsCollection is empty!");

  StoreArray<Relation>  cdcSimHitToCDCHits(m_cdcSimHitToCDCHitCollectioName);
  //B2INFO("CDCMCMatching: Number of relations between SimHits and CDCHits: "<<cdcSimHitToCDCHits.GetEntries());
  if (cdcSimHitToCDCHits.GetEntries() == 0) B2WARNING("CDCMCMatching: SimHitsToCDCHitsCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  //B2INFO("CDCMCMatching: Number of CDCRecoHits: "<<cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("CDCMCMatching: CDCRecoHitsCollection is empty!");

  StoreArray<CDCTrackCandidate> cdcTrackCandidates(m_cdcTrackCandsCollectionName);
  //B2INFO("CDCMCMatching: total Number of Track Candidates: "<<cdcTrackCandidates.GetEntries());
  if (cdcTrackCandidates.GetEntries() == 0) B2WARNING("CDCMCMatching: CDCTrackCandidatesCollection is empty!");

  StoreArray<Relation> cdcTrackCandsToRecoHits(m_cdcTrackCandsToRecoHits);
  //B2INFO("CDCMCMatching: Number of relations between CDCTrackCandidates and CDCRecoHits: "<<cdcTrackCandsToRecoHits.GetEntries());
  if (cdcTrackCandsToRecoHits.GetEntries() == 0) B2WARNING("CDCMCMatching: CDCTRackCandidatesToCDCRecoHitsCollection is empty!");


  //Create a relation between the track candidate and their most probable 'mother' MC particle
  StoreArray<Relation> cdcTrackCandToMCPart(m_cdcTrackCandsToMCParticles);

  for (int i = 0; i < cdcTrackCandidates->GetEntries(); i++) { //loop over all TrackCandidates
    list<short unsigned int> RecoHitsList;
    //create a list with RecoHits for this TrackCandidate
    for (int j = 0; j < cdcTrackCandsToRecoHits->GetEntries(); j++) {

      if (cdcTrackCandsToRecoHits[j]->getFromIndex() == i) {
        BOOST_FOREACH(int index, cdcTrackCandsToRecoHits[j]->getToIndices()) {
          RecoHitsList.push_back(index);
        }
      }
    }
    //B2INFO("Track "<<i<<"  "<<RecoHitsList.size()<<" RecoHits assigned");

    list<short unsigned int> SimHitsList;
    //use the RecoHitList to create a List with SimHit for this TrackCandidate
    for (int k = 0; k < cdcSimHitToCDCHits->GetEntries(); k++) {
      BOOST_FOREACH(int hit, RecoHitsList) {
        if (cdcSimHitToCDCHits[k]->getToIndex() == hit) {
          SimHitsList.push_back(cdcSimHitToCDCHits[k]->getFromIndex());
        }

      }
    }

    //B2INFO("Track "<<i<<"  "<<SimHitsList.size()<<" SimHits assigned");
    //finally look for MCParticles which created these SimHits
    for (int l = 0; l < mcPartToSimHits->GetEntries(); l++) {
      BOOST_FOREACH(int simhit, SimHitsList) {
        BOOST_FOREACH(int mchit, mcPartToSimHits[l]->getToIndices()) {
          if (simhit == mchit) {
            //B2INFO("This SimHit was created by MCParticle "<<mcPartToSimHits[l]->getFromIndex());
            //assign possible MCParticles to the TrackCandidate
            cdcTrackCandidates[i]->addMCParticle(mcPartToSimHits[l]->getFromIndex());
          }
        }

      }
    }
    for (unsigned int test = 0 ; test < cdcTrackCandidates[i]->getMCParticles().size(); test++) {
      //B2INFO("This Track has "<<cdcTrackCandidates[i]->getMCParticles().at(test).second<<" Hits from MCP "<<cdcTrackCandidates[i]->getMCParticles().at(test).first);
    }
    //Evaluate the MCParticle with the largest contribution
    cdcTrackCandidates[i]->evaluateMC();
    B2INFO("-------> Track " << i << " has " << cdcTrackCandidates[i]->getPurity() << "% of Hits from MCParticle " << cdcTrackCandidates[i]->getMCId());
    //Create Relation
    new(cdcTrackCandToMCPart->AddrAt(i)) Relation(cdcTrackCandidates, mcParticles, i, cdcTrackCandidates[i]->getMCId());

  }//end loop over all TrackCandidates

  //Now the evaltuation is performed from the other side

  /*
    B2INFO("Transform MCParticle to MCMatchParticles");
    StoreArray<MCMatchParticle> mcMatchParticles("MCMatchParticles");
    for (int i = 0; i < mcParticles->GetEntries(); i++) {
      MCMatchParticle matchparticle(mcParticles[i]);
      new (mcMatchParticles->AddrAt(i)) MCMatchParticle(matchparticle);
    }
  //  B2INFO("Loop over MCMatch particles "<<mcMatchParticles->GetEntries())
    int hitCounter = 0;
    for (int i = 0; i < mcMatchParticles->GetEntries(); i++) {
      list<short unsigned int> SimList;
      hitCounter = 0;

      if (mcMatchParticles[i]->getMother()) mcMatchParticles[i]->setPrimary(false);
      else mcMatchParticles[i]->setPrimary(true);


   //       B2INFO("Loop over relations SimHit->MC "<<cdcSimHitToMC->GetEntries());
      for (int j = 0; j < cdcSimHitToMC->GetEntries(); j++) {

        if (cdcSimHitToMC[j]->getToIndex() == i) {
          SimList.push_back(cdcSimHitToMC[j]->getFromIndex());
        }
      }

    //  B2INFO("Particle "<<i<<"  created SimHits: ");
      list<short unsigned int>::iterator iter;

      for (iter = SimList.begin(); iter != SimList.end(); iter++) {
    //    B2INFO(" iter: "<<*iter);

        for (int hit = 0; hit < cdcSimRelation->GetEntries(); hit ++){
        if (cdcSimRelation[hit]->getFromIndex() == *iter){
        //if (cdcSimRelation[*iter]) {  //!!!!!!!!!!!!!!!!
          int trackHitIndex = cdcSimRelation[hit]->getToIndex();
        //  B2INFO("And this SimHit created Track Hit "<<trackHitIndex);
          hitCounter++;
          //if (cdcTrackHitArray[trackHitIndex]->getTrackIndices().size()> 0) {
            for (int k = 0; k < cdcTrackHitArray[trackHitIndex]->getTrackIndices().size(); k++) {
              int trackIndex =
                      cdcTrackHitArray[trackHitIndex]->getTrackIndices().at(
                          k);
            //  B2INFO("+++ And this Track Hit belongs to Track "<<trackIndex);
              mcMatchParticles[i]->addTrack(trackIndex);
            }
        }
        //else B2INFO("----------------No relation to TrackHit for this SimHit");

        //}
      }
      }
      SimList.clear();
      mcMatchParticles[i]->setNHits(hitCounter);
      if (mcMatchParticles[i]->getNHits() > 0){
  //    B2INFO("particle "<<i<<" produced "<<mcMatchParticles[i]->getNHits()<<"  TrackHits");
      if (mcMatchParticles[i]->getTracks().size()>0){
  //    for (int k = 0; k < mcMatchParticles[i]->getTracks().size(); k++) {
  //      B2INFO("**** This particle has "<<mcMatchParticles[i]->getTracks().at(k).Y()<<" Hits in Track: "<<mcMatchParticles[i]->getTracks().at(k).X());
  //    }

      mcMatchParticles[i]->evaluateMC();
      mcMatchParticles[i]->evaluatePRes(cdcTracksArray[mcMatchParticles[i]->getTrackId()]->getMomentumValue());


      mcMatchParticles[i]->setPurity(cdcTracksArray[mcMatchParticles[i]->getTrackId()]->evaluateMC(i));
      if (mcMatchParticles[i]->getPrimary()== true){
      B2INFO("MC Particle "<<i<<" ("<<mcMatchParticles[i]->getPrimary()<<") :  "<<mcMatchParticles[i]->getCorrectMC()<<" %  of its hits are in the track "<<mcMatchParticles[i]->getTrackId() );
      B2INFO("---------------- and it corresponds to "<<mcMatchParticles[i]->getPurity()<<" % of the hits in this track");
      B2INFO("MC Particle PDG: "<<mcMatchParticles[i]->getPDG()<<"  Track Charge: "<<cdcTracksArray[mcMatchParticles[i]->getTrackId()]->getChargeSign());
      if (mcMatchParticles[i]->getPDG()*mcMatchParticles[i]->getPDG()>0 )
      { mcMatchParticles[i]->setCorrectCharge(1);
        }
      else mcMatchParticles[i]->setCorrectCharge(0);
      }
      }

    }

    }

  */
}

void CDCMCMatchingModule::endRun()
{

}

void CDCMCMatchingModule::terminate()
{

}

