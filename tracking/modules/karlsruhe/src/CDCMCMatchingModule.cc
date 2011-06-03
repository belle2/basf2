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

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>

#include "GFTrackCand.h"

#include <tracking/karlsruhe/MCMatchParticle.h>

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
  setDescription("Matches the CDCTrackCandidates with MCTruth to evaluate the performance of the pattern recognition (CDCTrackingModule). Creates as Output a relation between MCParticles and matched TrackCandidates as well as a MCMatchParticles Collection.");

  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits (should be created during the simulation within CDCSensitiveDetector)", string("MCPartToCDCSimHits"));
  addParam("CDCSimHitToCDCHitColName", m_cdcSimHitToCDCHitCollectioName, "Name of collection holding the relations between CDCSimHits and CDCHits (CDCHit index = CDCRecoHit index) (should be created by CDCDigi module)", string("SimHitToCDCHits"));

  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  addParam("CDCTrackCandidatesColName", m_cdcTrackCandsCollectionName, "Name of collection holding the CDCTrackCandidates (output of the pattern recognition, should be created by CDCTrackingModule)", string("CDCTrackCandidates"));
  addParam("CDCTrackCandToCDCRecoHitsColName", m_cdcTrackCandToRecoHits, "Name of collection holding the relations between CDCTrackCandidates and CDCRecoHits (should be created by CDCTrackingModule)", string("CDCTrackCandidateToCDCRecoHits"));

  addParam("CDCTrackCandsToMCParticlesColName", m_cdcTrackCandsToMCParticles, "Name of collection holding the relations between the CDCTrackCandidates and the matched MCParticles (output of this module)", string("CDCTrackCandidateToMCParticle"));
  addParam("MCMatchParticlesColName", m_mcMatchParticlesCollectionName, "Name of collection holding the MCMatchParticles (output of this module)", string("MCMatchParticles"));

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

  StoreArray<Relation> cdcTrackCandsToRecoHits(m_cdcTrackCandToRecoHits);
  //B2INFO("CDCMCMatching: Number of relations between CDCTrackCandidates and CDCRecoHits: "<<cdcTrackCandsToRecoHits.GetEntries());
  if (cdcTrackCandsToRecoHits.GetEntries() == 0) B2WARNING("CDCMCMatching: CDCTrackCandidatesToCDCRecoHitsCollection is empty!");

  StoreArray<GFTrackCand> gfTrackCandidates("GFTrackCandidates_conformal");


  //Create a relation between the track candidate and their most probable 'mother' MC particle
  StoreArray<Relation> cdcTrackCandToMCPart(m_cdcTrackCandsToMCParticles);

  if (cdcTrackCandidates.GetEntries() != 0) {
    for (int i = 0; i < cdcTrackCandidates.GetEntries(); i++) { //loop over all TrackCandidates
      list<short unsigned int> RecoHitsList;
      //create a list with RecoHits for this TrackCandidate
      for (int j = 0; j < cdcTrackCandsToRecoHits.GetEntries(); j++) {

        if (cdcTrackCandsToRecoHits[j]->getFromIndex() == i) {
          BOOST_FOREACH(int index, cdcTrackCandsToRecoHits[j]->getToIndices()) {
            RecoHitsList.push_back(index);
          }
        }
      }
      //B2INFO("Track "<<i<<"  "<<RecoHitsList.size()<<" RecoHits assigned");

      list<short unsigned int> SimHitsList;
      //use the RecoHitList to create a List with SimHits for this TrackCandidate
      for (int k = 0; k < cdcSimHitToCDCHits.GetEntries(); k++) {
        BOOST_FOREACH(int hit, RecoHitsList) {
          if (cdcSimHitToCDCHits[k]->getToIndex() == hit) {
            SimHitsList.push_back(cdcSimHitToCDCHits[k]->getFromIndex());
          }

        }
      }

      //B2INFO("Track "<<i<<"  "<<SimHitsList.size()<<" SimHits assigned");
      //finally look for MCParticles which created these SimHits
      for (int l = 0; l < mcPartToSimHits.GetEntries(); l++) {
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

      //Evaluate the MCParticle with the largest contribution
      cdcTrackCandidates[i]->evaluateMC();
      //Create Relation
      new(cdcTrackCandToMCPart->AddrAt(i)) Relation(cdcTrackCandidates, mcParticles, i, cdcTrackCandidates[i]->getMCId());

      //Some prompt info output
      B2INFO("-------> TrackCandidate " << i << " has " << cdcTrackCandidates[i]->getPurity() << "% of Hits from MCParticle " << cdcTrackCandidates[i]->getMCId());
      //B2INFO("        (TrackCandidate charge: "<<cdcTrackCandidates[i]->getChargeSign()<<"  : MCParticle pdg: "<<mcParticles[cdcTrackCandidates[i]->getMCId()]->getPDG()<<" )");

      gfTrackCandidates[i]->setMcTrackId(cdcTrackCandidates[i]->getMCId());
      gfTrackCandidates[i]->setCurv(cdcTrackCandidates[i]->getPurity());

      //B2INFO("Check GFTrackCandidates: "<<gfTrackCandidates[i]->getMcTrackId()<<"  "<<gfTrackCandidates[i]->getCurv());

      //Now find out if all Hits created by this MCParticle were collected in the TrackCandidate
      list<short unsigned int> MCSimHitsList;
      int hitCounter = 0;

      //fill the SimHitsList
      for (int j = 0; j < mcPartToSimHits.GetEntries(); j++) {
        if (mcPartToSimHits[j]->getFromIndex() == cdcTrackCandidates[i]->getMCId()) {
          BOOST_FOREACH(int index, mcPartToSimHits[j]->getToIndices()) {
            MCSimHitsList.push_back(index);
          }
        }
      }

      //create a list with RecoHits from the SimHitsList
      list<short unsigned int> MCRecoHitsList;

      //fill the RecoHitsList
      for (int k = 0; k < cdcSimHitToCDCHits.GetEntries(); k++) {
        BOOST_FOREACH(int simhit, MCSimHitsList) {
          if (cdcSimHitToCDCHits[k]->getFromIndex() == simhit) {
            MCRecoHitsList.push_back(cdcSimHitToCDCHits[k]->getToIndex());
            hitCounter ++;
          }
        }
      }

      B2INFO("Efficiency: " << cdcTrackCandidates[i]->getEfficiency(hitCounter));
      gfTrackCandidates[i]->setDip(cdcTrackCandidates[i]->getEfficiency(hitCounter));




    }//end loop over all TrackCandidates
  }

  //Now the evaluation is performed 'from the other side'
  //Create a StoreArray with MCMatchParticles, where the output can be stored
  //I think this part wont be needed in the future, but it is convenient for the moment...

  /*
  StoreArray<MCMatchParticle> mcMatchParticles(m_mcMatchParticlesCollectionName);
  //Create MCMatchParticle for each MCParticle (-> keep same StoreArray indices)
  for (int i = 0; i < mcParticles->GetEntries(); i++) {
   new (mcMatchParticles->AddrAt(i)) MCMatchParticle(mcParticles[i]);
  }

  //B2INFO("Nr of created MCMatchParticles: "<<mcMatchParticles->GetEntries())
  if (mcMatchParticles->GetEntries() == 0) B2WARNING("CDCMCMatching: No MCMatchParticles were created!");

  int hitCounter = 0;
  for (int i = 0; i < mcMatchParticles->GetEntries(); i++) {
       //create a list with SimHits created by this MCParticle
   list<short unsigned int> SimHitsList;
   hitCounter = 0;

   //fill the SimHitsList
   for (int j = 0; j < mcPartToSimHits->GetEntries(); j++){
          if (mcPartToSimHits[j]->getFromIndex()== i){
       BOOST_FOREACH(int index, mcPartToSimHits[j]->getToIndices()){
           SimHitsList.push_back(index);
         }
      }
   }

   //create a list with RecoHits from the SimHitsList
   list<short unsigned int> RecoHitsList;

   //fill the RecoHitsList
   for (int k = 0; k < cdcSimHitToCDCHits->GetEntries(); k++){
       BOOST_FOREACH(int simhit, SimHitsList){
         if (cdcSimHitToCDCHits[k]->getFromIndex()== simhit){
          RecoHitsList.push_back(cdcSimHitToCDCHits[k]->getToIndex());
          hitCounter ++;
         }
       }
   }

       mcMatchParticles[i]->setNHits(hitCounter); //Assign number of RecoHits produced by this particle

       //find out to which TrackCandidate the RecoHits were assigned
       for (int l = 0; l < cdcTrackCandsToRecoHits->GetEntries(); l++){
         BOOST_FOREACH(int hit, RecoHitsList){
           BOOST_FOREACH (int toIndex, cdcTrackCandsToRecoHits[l]->getToIndices()){
             if (hit == toIndex){
               //add the index of TrackCandidate to the particle
               mcMatchParticles[i]->addTrackCandidate(cdcTrackCandsToRecoHits[l]->getFromIndex());
             }
           }
         }
       }
       //Evaluate which Track has the most RecoHits from this particle
       mcMatchParticles[i]->evaluateMC();

       //Assign the momentum of the matched Track to the MCMatchParticle
       if (cdcTrackCandidates->GetEntries() != 0 ){
       mcMatchParticles[i]->setRecoMomentum(cdcTrackCandidates[mcMatchParticles[i]->getTrackCandId()]->getMomentumVector());
       }

       //Some prompt info output
   if (SimHitsList.size() != 0){
        // B2INFO("MCParticle "<<i<<" created "<<SimHitsList.size()<<"  SimHits and "<<RecoHitsList.size()<<" RecoHits ");
         if (RecoHitsList.size() != 0){
          // B2INFO("-----> "<<mcMatchParticles[i]->getEfficiency()<<" %  in TrackCandidate "<<mcMatchParticles[i]->getTrackCandId());
         }
   }

  }//end loop over MCMatchParticles
  */
}

void CDCMCMatchingModule::endRun()
{

}

void CDCMCMatchingModule::terminate()
{

}

