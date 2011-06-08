/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mctrackfinder/MCTrackFinderModule.h>

#include <generators/dataobjects/MCParticle.h>

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
  setDescription("Uses the MC information to create GFTrackCandidates from MCParticles and Relations between them.  Creates also a Relation between GFTrackCandidates and CDCRecoHits.");

  //Parameter definition

  // names of input containers
  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles", string("MCParticles"));
  // cdc specific
  addParam("CDCRecoHitsColName", m_cdcRecoHitColName, "Name of collection holding the CDCRecoHits", string("CDCRecoHits"));
  addParam("MCParticlesToCDCRecoHitsColName", m_mcParticleToCdcRecoHits, "Name of collection holding the Relations  MCParticles->CDCRecoHits..", string("MCParticleToCDCRecoHits"));
  //pxd specific
  addParam("PXDRecoHitsColName", m_pxdRecoHitColName, "Name of collection holding the PXDRecoHits", string(DEFAULT_PXDRECOHITS));
  addParam("MCParticlesToPXDRecoHitsColName", m_mcParticleToPxdRecoHits, "Name of collection holding the Relations  MCParticles->PXDRecoHits.", string(DEFAULT_PXDRECOHITSREL));
  // svd specific
  addParam("SVDRecoHitsColName", m_svdRecoHitColName, "Name of collection holding the SVDRecoHits", string(DEFAULT_SVDRECOHITS));
  addParam("MCParticlesToSVDRecoHitsColName", m_mcParticleToSvdRecoHits, "Name of collection holding the Relations  MCParticles->SVDRecoHits.", string(DEFAULT_SVDRECOHITSREL));

  // names of output containers
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Name of collection holding the GFTrackCandidates (output)", string("GFTrackCandidates"));
  addParam("GFTrackCandToMCParticleColName", m_gfTrackCandToMCParticleColName, "Name of collection holding the relations between GFTrackCandidates and MCParticles (output)", string("GFTrackCandidateToMCParticle"));
  addParam("GFTrackCandToCDCRecoHitsColName", m_gfTrackCandToCdcRecoHitsColName, "Name of collection holding the relations between GFTrackCandidates and CDCRecoHits (output)", string("GFTrackCandidateToCDCRecoHits"));
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
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(149, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) B2WARNING("MCTrackFinder: MCParticlesCollection is empty!");

  //get stuff for the CDC
  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitColName);
  int nCdcRecoHits = cdcRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of CDCRecoHits: " << nCdcRecoHits);
  if (nCdcRecoHits == 0) B2WARNING("MCTrackFinder: CDCRecoHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToCdcRecoHits(m_mcParticleToCdcRecoHits);
  int nMcPartToCdcRecoHits = mcParticleToCdcRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SimHits: " << nMcPartToCdcRecoHits);
  if (nMcPartToCdcRecoHits == 0) B2WARNING("MCTrackFinder: MCParticlesToCDCRecoHitsCollection is empty!");

  //get stuff for pxd
  StoreArray<PXDRecoHit> pxdRecoHits(m_pxdRecoHitColName);
  int nPxdRecoHits = pxdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of PXDRecoHits: " << nPxdRecoHits);
  if (nPxdRecoHits == 0) B2WARNING("MCTrackFinder: PXDRecoHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToPxdRecoHits(m_mcParticleToPxdRecoHits);
  int nMcPartToPxdRecoHits = mcParticleToPxdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and PXDRecoHits: " << nMcPartToPxdRecoHits);
  if (nMcPartToPxdRecoHits == 0) B2WARNING("MCTrackFinder: MCParticlesToPXDRecoHitsCollection is empty!");

  //get stuff for svd
  StoreArray<SVDRecoHit> svdRecoHits(m_svdRecoHitColName);
  int nSvdRecoHits = svdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of SVDRecoHits: " << nSvdRecoHits);
  if (nSvdRecoHits == 0) B2WARNING("MCTrackFinder: SVDRecoHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToSvdRecoHits(m_mcParticleToSvdRecoHits);
  int nMcPartToSvdRecoHits = mcParticleToSvdRecoHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SVDRecoHits: " << nMcPartToSvdRecoHits);
  if (nMcPartToSvdRecoHits == 0) B2WARNING("MCTrackFinder: MCParticlesToSVDRecoHitsCollection is empty!");

  // loop over MCParticles.
  // it would be nice to optimize this, because there are actually ~1000 secondary MCParticles for each primary MCParticle
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {

    //make links only for interesting MCParticles, for the moment take only primary particle
    //this method seems to be quite time consuming, maybe there is a better way to find out if it was a primary particle...
    if (mcParticles[iPart]->getMother() == NULL) {
      B2DEBUG(149, "Search a  track for the MCParticle with index: " << iPart << " (PDG: " << mcParticles[iPart]->getPDG() << ")");

      // create a list containing the indices to the CDCRecoHits that belong to one track
      list<int> indicesOfGoodCdcHits;
      int nMcPartToCdcRecoHits =  mcParticleToCdcRecoHits.getEntries();
      for (int i = 0; i < nMcPartToCdcRecoHits; ++i) {
        if (mcParticleToCdcRecoHits[i]->getFromIndex() == iPart) {
          int cdcRecoHitIndex = mcParticleToCdcRecoHits[i]->getToIndex();
          indicesOfGoodCdcHits.push_back(cdcRecoHitIndex);
        }
      }

      // create a list containing the indices to the PXDRecoHits that belong to one track
      list<int> indicesOfGoodPxdHits;
      int nMcPartToPxdRecoHits =  mcParticleToPxdRecoHits.getEntries();
      //dataOutPxd << "nRecoRel " << nMcPartToPxdRecoHits << "\n";
      for (int i = 0; i < nMcPartToPxdRecoHits; ++i) {
        if (mcParticleToPxdRecoHits[i]->getFromIndex() == iPart) {
          int pxdRecoHitIndex = mcParticleToPxdRecoHits[i]->getToIndex();
          indicesOfGoodPxdHits.push_back(pxdRecoHitIndex);
        }
      }
      // create a list containing the indices to the SVDRecoHits that belong to one track
      list<int> indicesOfGoodSvdHits;
      int nMcPartToSvdRecoHits =  mcParticleToSvdRecoHits.getEntries();
      //dataOutSvd << "nRecoRel " << nMcPartToSvdRecoHits << "\n";
      for (int i = 0; i < nMcPartToSvdRecoHits; ++i) {
        if (mcParticleToSvdRecoHits[i]->getFromIndex() == iPart) {
          int svdRecoHitIndex = mcParticleToSvdRecoHits[i]->getToIndex();
          indicesOfGoodSvdHits.push_back(svdRecoHitIndex);
        }
      }

      //Now create TrackCandidate and the relations
      //If the indices of the RecoHits are assigned to TrackCandidates, there may be no need for these Relations, but for the moment it is nice to have several options to do studies
      StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
      StoreArray<Relation> trackCandsToCdcRecoHits(m_gfTrackCandToCdcRecoHitsColName);
      StoreArray<Relation> trackCandsToPxdRecoHits(m_gfTrackCandToPxdRecoHitsColName);
      StoreArray<Relation> trackCandsToSvdRecoHits(m_gfTrackCandToSvdRecoHitsColName);

      StoreArray<Relation> trackCandsToMCParticles(m_gfTrackCandToMCParticleColName);

      int counter = trackCandidates->GetLast() + 1;
      B2DEBUG(100, "Create TrackCandidate  " << counter);

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

      trackCandidates[counter]->setTrackSeed(position, direction, chargeOverP);

      //assign indices of the RecoHits from all detectors, their are distinguishable by their DetID:
      // pxd 0
      //   svd 1
      //     cdc 2
      BOOST_FOREACH(int hitID, indicesOfGoodPxdHits) {
        trackCandidates[counter]->addHit(0, hitID);
      }
      //int hitCounterOffset = nPxdRecoHits;
      BOOST_FOREACH(int hitID, indicesOfGoodSvdHits) {
        trackCandidates[counter]->addHit(1, hitID);
      }
      //hitCounterOffset = nPxdRecoHits + nSvdRecoHits;
      BOOST_FOREACH(int hitID, indicesOfGoodCdcHits) {
        trackCandidates[counter]->addHit(2, hitID);
      }

      //Save the MCParticleID in the TrackCandidate
      trackCandidates[counter]->setMcTrackId(iPart);

      //create relation between the track candidate and cdcRecoHits
      new(trackCandsToCdcRecoHits->AddrAt(counter)) Relation(trackCandidates, cdcRecoHits, counter, indicesOfGoodCdcHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodCdcHits.size() << "  CDCRecoHits");
      new(trackCandsToPxdRecoHits->AddrAt(counter)) Relation(trackCandidates, pxdRecoHits, counter, indicesOfGoodPxdHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodPxdHits.size() << "  PXDRecoHits");
      new(trackCandsToSvdRecoHits->AddrAt(counter)) Relation(trackCandidates, svdRecoHits, counter, indicesOfGoodSvdHits);
      B2INFO("Create relation between Track Candidate " << counter << "  and  " << indicesOfGoodSvdHits.size() << "  SVDRecoHits");
      //create relation between the track candidates and the mcParticle
      new(trackCandsToMCParticles->AddrAt(counter)) Relation(trackCandidates, mcParticles, counter, iPart);
      B2DEBUG(149, " --- Create relation between Track Candidate" << counter << "  and MCParticle " << iPart);


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
  for (int ii = 0; ii < relations->GetEntriesFast(); ++ii) {
    if (relations[ii]->getToIndex() == toIndex) {
      myList.push_back(relations[ii]->getFromIndex());
      B2DEBUG(250, relations[ii]->getFromIndex());
    }
  }
  return (myList);
}
