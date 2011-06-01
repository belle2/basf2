/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mctrackfinder/MCTrackFinderModule.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>
#include <svd/dataobjects/SVDRecoHit.h>
#include "GFTrackCand.h"

#include <boost/foreach.hpp>

#include <utility>
#include <list>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCTrackFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCTrackFinderModule::MCTrackFinderModule() : Module()
{
  //Set module properties
  setDescription("Uses the MC information to create GFTrackCandidates from MCParticles and Relations between them.  Creates als a Relation between GFTrackCandidates and CDCRecoHits.");

  //Parameter definition

  // names of input containers
  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  // cdc spesific
  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits (should be created during the simulation within CDCSensitiveDetector)", string("MCPartToCDCSimHits"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "Name of collection holding the CDCRecoHits (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));
  addParam("CDCSimHitToCDCHitColName", m_cdcSimHitToCDCHitCollectioName, "Name of collection holding the relations between CDCSimHits and CDCHits (CDCHit index = CDCRecoHit index) (should be created by CDCDigi module)", string("SimHitToCDCHits"));
  //pxd spesific
  addParam("PXDRecoHitsColName", m_pxdRecoHitColName, "Name of collection holding the PXDRecoHits", string(DEFAULT_PXDRECOHITS));

  // svd spesific
  addParam("SVDRecoHitsColName", m_svdRecoHitColName, "Name of collection holding the SVDRecoHits", string(DEFAULT_SVDRECOHITS));
  //addParam("MCParticleToSVDSimHitsColName", m_mcPartToSvdSimHitsColName, "Name of collection holding the relations MCParticles->SVDSimHits", string(DEFAULT_SVDSIMHITSREL));
  // names of output containers
  addParam("GFTrackCandidatesColName", m_gfTrackCandsCollectionName, "Name of collection holding the GFTrackCandidates (output)", string("GFTrackCandidates"));
  addParam("GFTrackCandToMCParticleColName", m_gfTrackCandToMCParticleCollectionName, "Name of collection holding the relations between GFTrackCandidates and MCParticles (output)", string("GFTrackCandidateToMCParticle"));
  addParam("GFTrackCandToCDCRecoHitsColName", m_gfTrackCandToCDCRecoHitsCollectionName, "Name of collection holding the relations between GFTrackCandidates and CDCRecoHits (output)", string("GFTrackCandidateToCDCRecoHits"));
  addParam("GFTrackCandToPXDRecoHitsColName", m_gfTrackCandToPxdRecoHitsColName, "Name of collection holding the relations between GFTrackCandidates and PXDRecoHits (output)", string("GFTrackCandidateToPXDRecoHits"));
  addParam("GFTrackCandToSVDRecoHitsColName", m_gfTrackCandToSvdRecoHitsColName, "Name of collection holding the relations between GFTrackCandidates and SVDRecoHits (output)", string("GFTrackCandidateToSVDRecoHits"));
}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{
}



void MCTrackFinderModule::event()
{
  B2INFO("*******   MCTrackFinderModule  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(149, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) B2WARNING("MCTrackFinder: MCParticlesCollection is empty!");

  //get stuff for the CDC
  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  int nCdcRecoHits = cdcRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of CDCRecoHits: " << nCdcRecoHits);
  if (nCdcRecoHits == 0) B2WARNING("MCTrackFinder: CDCRecoHitsCollection is empty!");

  StoreArray<Relation>   mcPartToCdcSimHits(m_mcPartToCDCSimHitsCollectionName);
  int nMcPartToCdcSimHits = mcPartToCdcSimHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SimHits: " << nMcPartToCdcSimHits);
  if (nMcPartToCdcSimHits == 0) B2WARNING("MCTrackFinder: MCParticlesToSimHitsCollection is empty!");

  StoreArray<Relation>  cdcSimHitToCDCHits(m_cdcSimHitToCDCHitCollectioName);
  int nCdcSimHitToCDCHits = cdcSimHitToCDCHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between SimHits and CDCHits: " << nCdcSimHitToCDCHits);
  if (nCdcSimHitToCDCHits == 0) B2WARNING("MCTrackFinder: SimHitsToCDCHitsCollection is empty!");



  //get stuff for pxd
  StoreArray<PXDRecoHit> pxdRecoHits(m_pxdRecoHitColName);
  int nPxdRecoHits = pxdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of PXDRecoHits: " << nPxdRecoHits);
  if (nPxdRecoHits == 0) B2WARNING("MCTrackFinder: PXDRecoHitsCollection is empty!");

  StoreArray<Relation>   mcPartToPxdRecoHits(DEFAULT_PXDRECOHITSREL);
  int nMcPartToPxdRecoHits = mcPartToPxdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and PXDRecoHits: " << nMcPartToPxdRecoHits);
  if (nMcPartToPxdRecoHits == 0) B2WARNING("MCTrackFinder: MCParticlesToPXDRecoHitsCollection is empty!");

  //get stuff for svd
  StoreArray<SVDRecoHit> svdRecoHits(m_svdRecoHitColName);
  int nSvdRecoHits = svdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of SVDRecoHits: " << nSvdRecoHits);
  if (nSvdRecoHits == 0) B2WARNING("MCTrackFinder: SVDRecoHitsCollection is empty!");

  StoreArray<Relation>   mcPartToSvdRecoHits(DEFAULT_SVDRECOHITSREL);
  int nMcPartToSvdRecoHits = mcPartToSvdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SVDRecoHits: " << nMcPartToSvdRecoHits);
  if (nMcPartToSvdRecoHits == 0) B2WARNING("MCTrackFinder: MCParticlesToSVDRecoHitsCollection is empty!");

  //B2INFO("-> Create relations for primary particles");
  // loop over MCParticles.
  for (int iPart = 0; iPart < nMcParticles; iPart++) {

    //make links only for interesting MCParticles, for the moment take only primary particle
    if (mcParticles[iPart]->getMother() == NULL) {
      B2DEBUG(149, "iPart: " << iPart);
      //B2INFO("Primary particle!");
      //Get indices of the SimHits from this MCParticle

      // create a list containing the indices to the CDCRecoHits that belong to one track
      // mybe it would be better to have direct relations from mcParticles to CdcRecoHits
      // the the code for pxd svd and cdc would look the same
      list<short unsigned int> myList;
      for (int i = 0; i < nMcPartToCdcSimHits; i++) {
        if (mcPartToCdcSimHits[i]->getFromIndex() == iPart) {
          myList.push_back(mcPartToCdcSimHits[i]->getToIndex()); //this was different before, as the relation in CDCSensitiveCetector was created other way round
        }
      }
      //B2INFO("Nr of SimHits: "<<myList.size());
      //Now get all digitized Hits for this MCParticle
      list<int> indicesOfGoodCdcHits;
      // Not all SimHits have corresponding real hit. Find real CDCHits associated with the MCParticles:
      // Therefore I try to change the numbers in the list associated with each hit to the ones of the real CDCHits.
      for (list<short unsigned int>::iterator iter = myList.begin(), iterEnd = myList.end(); iter != iterEnd; iter++) {
        int cdcHit = getToForFrom(m_cdcSimHitToCDCHitCollectioName, (*iter));
        if (cdcHit >= 0) {
          B2DEBUG(149, cdcHit);
          indicesOfGoodCdcHits.push_back(cdcHit);
        }
      }
      //B2INFO("Nr of Hits: "<<indicesOfGoodCdcHits.size());
      //every CDCHit will have a CDCRecoHit.
      //Therefore I should now be able to create a Relation, that points to the RecoHits, that really belong to the same track.

      // create a list containing the indices to the PXDRecoHits that belong to one track
      // because there are direct relations between mcParticles and the recoHits this is a bit shorter
      list<int> indicesOfGoodPxdHits;
      int nMcPartToPxdRecoHits =  mcPartToPxdRecoHits.getEntries();
      //dataOutPxd << "nRecoRel " << nMcPartToPxdRecoHits << "\n";
      for (int i = 0; i not_eq nMcPartToPxdRecoHits; i++) {
        if (mcPartToPxdRecoHits[i]->getFromIndex() == iPart) {
          int pxdRecoHitIndex = mcPartToPxdRecoHits[i]->getToIndex();
          indicesOfGoodPxdHits.push_back(pxdRecoHitIndex);
        }
      }
      // create a list containing the indices to the SVDRecoHits that belong to one track
      list<int> indicesOfGoodSvdHits;
      int nMcPartToSvdRecoHits =  mcPartToSvdRecoHits.getEntries();
      //dataOutSvd << "nRecoRel " << nMcPartToSvdRecoHits << "\n";
      for (int i = 0; i not_eq nMcPartToSvdRecoHits; i++) {
        if (mcPartToSvdRecoHits[i]->getFromIndex() == iPart) {
          int svdRecoHitIndex = mcPartToSvdRecoHits[i]->getToIndex();
          indicesOfGoodSvdHits.push_back(svdRecoHitIndex);
        }
      }
      //Now create Tracks and the relations
      StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsCollectionName);
      StoreArray<Relation> trackCandsToCDCRecoHits(m_gfTrackCandToCDCRecoHitsCollectionName);
      StoreArray<Relation> trackCandsToMCParticles(m_gfTrackCandToMCParticleCollectionName);

      StoreArray<Relation> trackCandsToPxdRecoHits(m_gfTrackCandToPxdRecoHitsColName);
      StoreArray<Relation> trackCandsToSvdRecoHits(m_gfTrackCandToSvdRecoHitsColName);

      int counter = trackCandidates->GetLast() + 1;
      B2DEBUG(100, counter);

      //create TrackCandidate
      new(trackCandidates->AddrAt(counter)) GFTrackCand();

      //set the values needed as start values for the fit in the GFTrackCandidate from the MCParticle information
      //variables stored in the GFTrackCandidates are: vertex position, track direction, charge/total momentum, indices for the RecoHits

      TVector3 position = mcParticles[iPart]->getProductionVertex();
      TVector3 direction;
      direction.SetX(mcParticles[iPart]->getMomentum().x() / mcParticles[iPart]->getMomentum().Mag());
      direction.SetY(mcParticles[iPart]->getMomentum().y() / mcParticles[iPart]->getMomentum().Mag());
      direction.SetZ(mcParticles[iPart]->getMomentum().z() / mcParticles[iPart]->getMomentum().Mag());
      double chargeOverP = double(mcParticles[iPart]->getCharge()) / mcParticles[iPart]->getMomentum().Mag();
      //B2INFO("Momentum: "<<mcParticles[iPart]->getMomentum().x()<<"  "<<mcParticles[iPart]->getMomentum().y()<<"  "<<mcParticles[iPart]->getMomentum().z());
      //B2INFO("Absolut momentum: "<<mcParticles[iPart]->getMomentum().Mag());
      //B2INFO("Position: "<<position.x()<<"  "<<position.y()<<"  "<<position.z());
      //B2INFO("Direction: "<<direction.x()<<"  "<<direction.y()<<"  "<<direction.z());
      //B2INFO("Charge over P: "<<chargeOverP);

      trackCandidates[counter]->setTrackSeed(position, direction, chargeOverP);

      BOOST_FOREACH(int hitID, indicesOfGoodPxdHits) {
        trackCandidates[counter]->addHit(0, hitID);
      }
      int hitCounterOffset = nPxdRecoHits;
      BOOST_FOREACH(int hitID, indicesOfGoodSvdHits) {
        trackCandidates[counter]->addHit(1, hitCounterOffset + hitID);
      }
      hitCounterOffset = nPxdRecoHits + nSvdRecoHits;
      BOOST_FOREACH(int hitID, indicesOfGoodCdcHits) {
        trackCandidates[counter]->addHit(2, hitCounterOffset + hitID);
      }
      //B2INFO("Total NR of assigned RecoHits: "<<trackCandidates[counter]->getNHits());


      //create relation between the track candidate and cdcRecoHits
      new(trackCandsToCDCRecoHits->AddrAt(counter)) Relation(trackCandidates, cdcRecoHits, counter, indicesOfGoodCdcHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodCdcHits.size() << "  RecoHits");
      new(trackCandsToPxdRecoHits->AddrAt(counter)) Relation(trackCandidates, pxdRecoHits, counter, indicesOfGoodPxdHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodPxdHits.size() << "  RecoHits");
      new(trackCandsToSvdRecoHits->AddrAt(counter)) Relation(trackCandidates, svdRecoHits, counter, indicesOfGoodSvdHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodSvdHits.size() << "  RecoHits");
      //create relation between the track candidates and the mcParticle
      new(trackCandsToMCParticles->AddrAt(counter)) Relation(trackCandidates, mcParticles, counter, iPart);
      //B2INFO(" --- Create relation between Track Candidate"<<counter<<"  and MCParticle "<<iPart);


    } //endif

  }//end loop over MCParticles



}


void MCTrackFinderModule::terminate()
{
}


list<int> MCTrackFinderModule::getFromForTo(const std::string& relationName, const unsigned short int& toIndex)
{
  StoreArray<Relation> relations(relationName);
  list<int> myList;
  for (int ii = 0; ii < relations->GetEntriesFast(); ii++) {
    if (relations[ii]->getToIndex() == toIndex) {
      myList.push_back(relations[ii]->getFromIndex());
      B2DEBUG(250, relations[ii]->getFromIndex());
    }
  }
  return (myList);
}
