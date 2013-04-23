/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mcTrackCandCombiner/MCTrackCandCombinerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <GFTrackCand.h>
#include <tracking/trackCandidateHits/CDCTrackCandHit.h>

#include <boost/foreach.hpp>
#include <boost/math/special_functions/sign.hpp>

#include <utility>
#include <sstream>
#include <cmath>


using namespace std;
using namespace Belle2;
using namespace Tracking;
using boost::math::sign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCTrackCandCombiner)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCTrackCandCombinerModule::MCTrackCandCombinerModule() : Module()
{
  //Set module properties
  setDescription("Uses the MC information to create GFTrackCandidates for primary MCParticles and Relations between them.  Fills the created GFTrackCandidates with all information (start values, hit indices) needed for the fitting.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition
  //addParam("UseClusters", m_useClusters, "Set true if you want to use PXD/SVD clusters instead of PXD/SVD trueHits", bool(false));
  addParam("CDCTrackCandidatesColName", m_cdcTrackCandColName, "Name of collection holding the GFTrackCandidates from CDC track finding (input)", string(""));
  addParam("VXDTrackCandidatesColName", m_vxdTrackCandColName, "Name of collection holding the GFTrackCandidates from VXD track finding(input)", string(""));
  addParam("OutputTrackCandidatesColName", m_combinedTrackCandColName, "Name of collection holding the combined GFTrackCandidates (output)", string(""));
  addParam("HitsRatio", m_hitsRatio, "minimal ratio of hits belonging to one MCParticle to declare a track candidate coming from this MCParticle", 0.6);
  addParam("InsertCorrectPDGCode", m_addMcInfo, "set the correct PDG code from the MCParticle in the output track candidates", true);
//  addParam("UseClusters", m_useClusters, "Set true if you want to use PXD/SVD clusters instead of PXD/SVD trueHits", bool(false));
}


MCTrackCandCombinerModule::~MCTrackCandCombinerModule()
{

}


void MCTrackCandCombinerModule::initialize()
{
  StoreArray<MCParticle>::required();
  // at least one of the two store arrays has to be present
  if (StoreArray<GFTrackCand>::optional(m_vxdTrackCandColName) == false) {
    StoreArray<GFTrackCand>::required(m_cdcTrackCandColName);
  }

  //output store arrays have to be registered in initialize()
  StoreArray<GFTrackCand>::registerPersistent(m_combinedTrackCandColName);

  RelationArray::registerPersistent<GFTrackCand, MCParticle>(m_combinedTrackCandColName, "");



}

void MCTrackCandCombinerModule::beginRun()
{
  m_nIgnoredVxdTcs = 0;
  m_nIgnoredCdcTcs = 0;

  m_nCdcTcs = 0;
  m_nVxdTcs = 0;
  m_nOutputTcs = 0;

  m_nCdcTcsWithoutPartner = 0;
  m_nVxdTcsWithoutPartner = 0;
}


void MCTrackCandCombinerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "*******   MCTrackCandCombinerModule processing event number: " << eventCounter << " *******");

  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles;
  const int nMcParticles = mcParticles.getEntries();
  B2DEBUG(100, "Number of MCParticles in this Event: " << nMcParticles);

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters;
  const int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(100, "Number of PXDClusters in this Event: " << nPXDClusters);
  //SVD clusters
  StoreArray<SVDCluster> svdClusters;
  const int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(100, "Number of SVDClusters in this Event: " << nSVDClusters);

  //CDC
  StoreArray<CDCHit> cdcHits;
  const int nCDCHits = cdcHits.getEntries();
  B2DEBUG(100, "Number of CDCHits in this Event: " << nCDCHits);



  //input store arrays
  StoreArray<GFTrackCand> vxdCands(m_vxdTrackCandColName);
  const int nVxdTCs = vxdCands.getEntries();
  B2DEBUG(100, "Number of vxdCands in this Event: " << nVxdTCs);
  m_nVxdTcs += nVxdTCs;
  StoreArray<GFTrackCand> cdcCands(m_cdcTrackCandColName);
  const int nCdcTCs = cdcCands.getEntries();
  B2DEBUG(100, "Number of cdcCands in this Event: " << nCdcTCs);
  m_nCdcTcs += nCdcTCs;
  //register StoreArray which will be filled by this module
  StoreArray<GFTrackCand> outCands(m_combinedTrackCandColName);
  outCands.create();
  //RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

  // loop over MCParticles.
  for (int iPart = 0; iPart not_eq nMcParticles; ++iPart) {
    MCParticle* aMcParticle = mcParticles[iPart];

    RelationVector<CDCHit> cdcRelations = aMcParticle->getRelationsTo<CDCHit>();
    const int nCdcHits = cdcRelations.size();

    RelationVector<PXDCluster> pxdRelations = aMcParticle->getRelationsFrom<PXDCluster>(); //yeah these are in the opposite direction :-P

    const int nPxdHits = pxdRelations.size();

    RelationVector<SVDCluster> svdRelations = aMcParticle->getRelationsFrom<SVDCluster>();

    const int nSvdHits = svdRelations.size();

    if (nCdcHits + nPxdHits + nSvdHits < 3) {
      continue; //2 hits are not enough to produce a TC no matter what track finder was used
    }

    vector<GFTrackCand*> goodCdcCands; //should have a maximum of 1 in most cases
    vector<int> matchingHitsInGoodCdcCands;
//    int nMostMatchingHits = 0;
    for (int iTC = 0; iTC not_eq nCdcTCs; ++iTC) {
      int nMatchingHits = 0;
      GFTrackCand* aTC = cdcCands[iTC];
      vector<int> hitIDsFromTC = aTC->getHitIDs(Const::CDC);
      const int nHitIDs = hitIDsFromTC.size();
      const int nNeededHits = int(m_hitsRatio * double(nHitIDs));
      vector<CDCHit*> hitsFromTC;
      for (int i = 0; i not_eq nHitIDs; ++i) {
        hitsFromTC.push_back(cdcHits[hitIDsFromTC[i]]);
      }
      for (int i = 0; i not_eq nHitIDs; ++i) {
        for (int j = 0; j not_eq nCdcHits; ++j) {
          if (hitsFromTC[i] == cdcRelations[j]) {
            ++nMatchingHits;
          }
        }
      }
      if (nMatchingHits >= nNeededHits) {
        goodCdcCands.push_back(aTC);
        matchingHitsInGoodCdcCands.push_back(nMatchingHits);
        B2DEBUG(100, "found a CDC TC with index " << iTC << " and (" << nMatchingHits << " | " << aTC->getNHits() << ") number of (matching|total) hits. The TC belongs the MCParticle with index " << iPart);
      }
    }


    vector<GFTrackCand*> goodVxdCands; //should have a maximum of 1 in most cases
    vector<int> matchingHitsInGoodVxdCands;
//    nMostMatchingHits = 0;
    for (int iTC = 0; iTC not_eq nVxdTCs; ++iTC) {
      int nMatchingHits = 0;
      GFTrackCand* aTC = vxdCands[iTC];
      vector<int> pxdHitIDsFromTC = aTC->getHitIDs(Const::PXD);
      const int nPxdHitsFromTC = pxdHitIDsFromTC.size();
      vector<int> svdHitIDsFromTC = aTC->getHitIDs(Const::SVD);
      const int nSvdHitsFromTC = svdHitIDsFromTC.size();
      const int nHitIDsFromTC = nPxdHitsFromTC + nSvdHitsFromTC;
      const int nNeededHits = int(m_hitsRatio * double(nHitIDsFromTC));
      vector<PXDCluster*> pxdHitsFromTC;
      for (int i = 0; i not_eq nPxdHitsFromTC; ++i) {
        pxdHitsFromTC.push_back(pxdClusters[pxdHitIDsFromTC[i]]);
      }
      vector<SVDCluster*> svdHitsFromTC;
      for (int i = 0; i not_eq nSvdHitsFromTC; ++i) {
        svdHitsFromTC.push_back(svdClusters[svdHitIDsFromTC[i]]);
      }
      for (int i = 0; i not_eq nPxdHitsFromTC; ++i) {
        for (int j = 0; j not_eq nPxdHits; ++j) {
          if (pxdHitsFromTC[i] == pxdRelations[j]) {
            ++nMatchingHits;
          }
        }
      }
      for (int i = 0; i not_eq nSvdHitsFromTC; ++i) {
        for (int j = 0; j not_eq nSvdHits; ++j) {
          if (svdHitsFromTC[i] == svdRelations[j]) {
            ++nMatchingHits;
          }
        }
      }
      if (nMatchingHits >= nNeededHits) {
        goodVxdCands.push_back(aTC);
        matchingHitsInGoodVxdCands.push_back(nMatchingHits);
        B2DEBUG(100, "found a VXD TC with index " << iTC << " and (" << nMatchingHits << " | " << aTC->getNHits() << ") number of (matching|total) hits. The TC belongs the MCParticle with index " << iPart);
      }
    }

    if (goodVxdCands.size() + goodCdcCands.size() == 0) { // no matching TC goto next MParticle
      continue;
    }

    //throw out all TC that have less good hits than the best of them (of course the makes curler finding impossible
    vector<int>::iterator it;
    int index = -1;
    GFTrackCand* bestTC = NULL;
    if (goodVxdCands.size() > 1) {
      m_nIgnoredVxdTcs += goodVxdCands.size() - 1;
      it = max_element(matchingHitsInGoodVxdCands.begin(), matchingHitsInGoodVxdCands.end());
      index = it - matchingHitsInGoodVxdCands.begin();
      bestTC = goodVxdCands[index];
      goodVxdCands.clear();
      goodVxdCands.push_back(bestTC);

    }

    if (goodCdcCands.size() > 1) {
      m_nIgnoredCdcTcs += goodCdcCands.size() - 1;
      it = max_element(matchingHitsInGoodCdcCands.begin(), matchingHitsInGoodCdcCands.end());
      index = it - matchingHitsInGoodCdcCands.begin();
      bestTC = goodCdcCands[index];
      goodCdcCands.clear();
      goodCdcCands.push_back(bestTC);
    }
    //easiest case; no curler finding yet
    const int truePdgCode = aMcParticle->getPDG();
    B2DEBUG(100, "The track caused by MCParticle with index " << iPart << " and PDG code " << truePdgCode << " that has p = " << aMcParticle->getMomentum().Mag() << " GeV and θ = " << aMcParticle->getMomentum().Theta() * 180 / TMath::Pi() << "° was found by at least one track finder.");
    B2DEBUG(100, "goodVxdCands.size() " << goodVxdCands.size() << " goodCdcCands.size() " << goodCdcCands.size());
    if (goodVxdCands.size() == 1 and goodCdcCands.size() == 1) { //from one mcparticle we have one tc in vxd and one in cdc
      GFTrackCand* combinedTrackCand = outCands.appendNew();
      (*combinedTrackCand) = *(goodVxdCands[0]);
      if (m_addMcInfo) {
        combinedTrackCand->setMcTrackId(iPart);
        combinedTrackCand->setPdgCode(truePdgCode);
      }

      //combinedTrackCand.append(*(goodCdcCands[0])); // append does not work with the store array because it does not make a deep copy of the trackCandHtis (this is fixed in genfit version 978

      int nCdcCandHits = goodCdcCands[0]->getNHits();
      int detId = -1;
      int hitId = -1;
      int planeId = -1;
      double rho = -1;
      for (int i = 0 ; i not_eq nCdcCandHits; ++i) {
        goodCdcCands[0]->getHit(i, detId, hitId, rho);
        combinedTrackCand->addHit(detId, hitId, planeId, rho);
      }
      B2DEBUG(100, "combined 2 TCs to one")
    }

    if (goodVxdCands.size() == 1 and goodCdcCands.size() == 0) {
      GFTrackCand* combinedTrackCand = outCands.appendNew();
      (*combinedTrackCand) = *(goodVxdCands[0]);
      if (m_addMcInfo) {
        combinedTrackCand->setMcTrackId(iPart);
        combinedTrackCand->setPdgCode(truePdgCode);
      }
      B2DEBUG(100, "copy one TC to the output that had only hits in the VXD but non in the CDC")
      ++m_nVxdTcsWithoutPartner;
    }

    if (goodVxdCands.size() == 0 and goodCdcCands.size() == 1) {
      GFTrackCand* combinedTrackCand = outCands.appendNew();
      (*combinedTrackCand) = *(goodCdcCands[0]);
      if (m_addMcInfo) {
        combinedTrackCand->setMcTrackId(iPart);
        combinedTrackCand->setPdgCode(truePdgCode);
      }
      B2DEBUG(100, "copy one TC to the output that had only hits in the CDC but non in the VXD")
      ++m_nCdcTcsWithoutPartner;
    }

  }//end loop over MCParticles

  m_nOutputTcs += outCands.getEntries();
  B2DEBUG(100, "created " << outCands.getEntries() << " track candidates")
}

void MCTrackCandCombinerModule::endRun()
{
  B2INFO("Total number of input VXD track candidates: " << m_nVxdTcs);
  B2INFO("Total number of input CDC track candidates: " << m_nCdcTcs);
  B2INFO("Total number of output track candidates: " << m_nOutputTcs << " . " <<  m_nVxdTcsWithoutPartner << " have only VXD hits. " << m_nCdcTcsWithoutPartner << " have only CDC hits and " <<  m_nOutputTcs - m_nVxdTcsWithoutPartner - m_nCdcTcsWithoutPartner << " have VXD and CDC hits");
  B2WARNING("Total number of ignored VXD track candidates: " << m_nIgnoredVxdTcs);
  B2WARNING("Total number of ignored CDC track candidates: " << m_nIgnoredCdcTcs);
}

void MCTrackCandCombinerModule::terminate()
{
}
