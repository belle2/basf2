/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitter2Module.h>



#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
//#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <framework/logging/Logger.h>


#include <GFTrack.h>
//#include <GFKalman2.h>
#include <GFKalman.h>
#include <GFDaf.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>
#include <GFConstField.h>
#include <GFFieldManager.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

#include <GFDetPlane.h>
#include <GFTools.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
    Module()
{

  setDescription("Uses GenFit to fit tracks within the CDC. Can fit the MCTracks or Tracks provided by the CDCTrackingModule.");
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("noFilter", m_noFilter, "do not throw away tracks with do not have exactly 1 hit in every Si layer", false);
}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{

}

void GenFitter2Module::beginRun()
{
  m_notPerfectCounter = 0;
  m_failedFitCounter = 0;
  m_fitCounter = 0;
}

void GenFitter2Module::event()
{

  //StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  //if (eventMetaDataPtr->getEvent() == 4821){
  B2INFO("**********   GenFitter2Module  ************");
  StoreArray<GFTrackCand> trackCandidates("");
  int nTrackCandidates = trackCandidates.getEntries();
  if (nTrackCandidates == 0) {
    B2DEBUG(100, "GenFitter: GFTrackCandidatesCollection is empty!");
  }
  StoreArray<SVDTrueHit> svdTrueHits("");
  int nSvdTrueHits = svdTrueHits.getEntries();
  if (nSvdTrueHits == 0) {
    B2DEBUG(100, "GenFitter: GFTrackCandidatesCollection is empty!");
  }
  StoreArray<PXDTrueHit> pxdTrueHits("");
  int nPxdTrueHits = pxdTrueHits.getEntries();
  if (nPxdTrueHits == 0) {
    B2DEBUG(100, "GenFitter: GFTrackCandidatesCollection is empty!");
  }
  StoreArray<GFTrack> fittedTracks(""); //holds the output of this module in the form of Genfit track objects

  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

  GFTrackCand* aTrackCandPointer = trackCandidates[0];
  //get fit starting values from the MCParticle
  TVector3 vertex = aTrackCandPointer->getPosSeed();
  TVector3 momentum = aTrackCandPointer->getDirSeed() * abs(1.0 / aTrackCandPointer->getQoverPseed());
  //int pdg = aMcParticleArray[0]->getPDG();

  // for the filter function to get only tracks that hits specific layers
  int nHitsL1 = 0; int nHitsL2 = 0; int nHitsL3 = 0; int nHitsL4 = 0; int nHitsL5 = 0; int nHitsL6 = 0;

  //B2INFO("MCIndex: "<<mcindex);
  B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
  B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
  B2INFO("Start values: pdg:      " << aTrackCandPointer->getPdgCode());
  GFAbsTrackRep* trackRep;
  //Now create a GenFit track with this representation

  //trackRep = new RKTrackRep(vertex, momentum, poserr, momerr, pdg);
  trackRep = new RKTrackRep(aTrackCandPointer);
  GFTrack track(trackRep, true);


//  vector<unsigned int> pxdIndexList = aTrackCandPointer->GetHitIDs(0);
//  B2DEBUG(100, "pxd hitids");
//  int nPxdIndexList = pxdIndexList.size();
//  for (int i = 0; i not_eq nPxdIndexList; ++i) {
//    B2DEBUG(100, pxdIndexList[i] << " ");
//  }
//  B2DEBUG(100, endl);
//  //Collect all RecoHits for this track
//  //Caution: for the moment RecoHits are not sorted, but GenFit needs them in a certain order.
//  //It seems that that order is by chance correct for simple single tracks, but the fit may fail for more complicated events.
//  int hitCounter = -1;
//  //list<short unsigned int> pxdIndexList = trackToPxdRecoHits[i]->getToIndices();
//  vector<unsigned int>::const_iterator iter = pxdIndexList.begin();
//  vector<unsigned int>::const_iterator iterMax = pxdIndexList.end();
//  while (iter not_eq iterMax) {
//    hitCounter++;
//    int aSensorUniID = pxdRecoHits[*iter]->getSensorUniID();
//    SensorUniIDManager aIdConverter(aSensorUniID);
//    int layerId = aIdConverter.getLayerID();
//    int ladderId = aIdConverter.getLadderID();
//    int sensorId = aIdConverter.getSensorID();
//    track.addHit(pxdRecoHits[*iter], 0, hitCounter, 0, hitCounter);
//    double varU = pxdRecoHits[*iter]->getUError();
//    double varV = pxdRecoHits[*iter]->getVError();
//    double U = pxdRecoHits[*iter]->getU();
//    double V = pxdRecoHits[*iter]->getV();
//    B2DEBUG(100,"====== NEXT HIT pxd " << *iter << " " << layerId << " " << ladderId << " " << sensorId);
//    B2DEBUG(100,"vars: " << varU << " " << varV << " " << sqrt(varU) << " " << sqrt(varV));
//    B2DEBUG(100,"m: " << U << " " << V);
//    //GFDetPlane aGFDetPlane = pxdRecoHits[j]->getDetPlane(trackRep);
//    //cout << "print pxd plane" << flush;
//    //aGFDetPlane.Print();
//    ++iter;
//    if(layerId == 1){
//      ++nHitsL1;
//    }
//    if(layerId == 2){
//      ++nHitsL2;
//    }
//  }//end loop over pxd RecoHits
//
//  //track.addHit(pxdRecoHits[0], 0, 0);
//  //track.addHit(pxdRecoHits[1], 0, 1);
//  //track.addHit(pxdRecoHits[2], 0, 2);
//  //SVD
//  //pxdRecoHits[0]->Print();
//  //track.addHit(pxdRecoHits[0], 1, 0);
//  vector<unsigned int> svdIndexList = aTrackCandPointer->GetHitIDs(1);
//
//  B2DEBUG(100, "svd hitids");
//  for (int i = 0; i not_eq svdIndexList.size(); ++i) {
//    B2DEBUG(100, svdIndexList[i] << " ");
//  }
//  B2DEBUG(100, endl);
//  iter = svdIndexList.begin();
//  iterMax = svdIndexList.end();
//  while (iter not_eq iterMax) {
//    hitCounter++;
//    //cerr << *iter << " ";
//    SVDRecoHit* aSVDRecoHit = svdRecoHits[*iter];
//    //aSVDRecoHit->Print();
//    double varU = aSVDRecoHit->getUError();
//    double varV = aSVDRecoHit->getVError();
//    double U = aSVDRecoHit->getU();
//    double V = aSVDRecoHit->getV();
//
//    //cerr << "after addHit" << endl;
//    int aSensorUniID = svdRecoHits[*iter]->getSensorUniID();
//    SensorUniIDManager aIdConverter(aSensorUniID);
//    int layerId = aIdConverter.getLayerID();
//    int ladderId = aIdConverter.getLadderID();
//    int sensorId = aIdConverter.getSensorID();
//    track.addHit(aSVDRecoHit, 1, hitCounter, 0, hitCounter);
//    B2DEBUG(100,"====== NEXT HIT svd " << *iter << " " << layerId << " " << ladderId << " " << sensorId);
//    B2DEBUG(100,"vars: " << varU << " " << varV << " " << sqrt(varU) << " " << sqrt(varV));
//    B2DEBUG(100,"m: " << U << " " << V);
//    //GFDetPlane aGFDetPlane = svdRecoHits[j]->getDetPlane(trackRep);
//    //cout << "print svd plane" << flush;
//    // aGFDetPlane.Print();
//    ++iter;
//    if(layerId == 3){
//      ++nHitsL3;
//    }
//    if(layerId == 4){
//      ++nHitsL4;
//    }
//    if(layerId == 5){
//      ++nHitsL5;
//    }
//    if(layerId == 6){
//      ++nHitsL6;
//    }
//  }//end loop over SVD RecoHits
//
//
//  vector<unsigned int> indexList = aTrackCandPointer->GetHitIDs(2);
//  B2DEBUG(100, "cdc hitids");
//  for (int i = 0; i not_eq indexList.size(); ++i) {
//    B2DEBUG(100, indexList[i] << " ");
//  }
//  B2DEBUG(100, endl);
//  //hitCounter = 1;
//  //list<short unsigned int> indexList = trackToCDCRecoHits[i]->getToIndices();
//  int nCdcRecoHits = cdcRecoHits->getEntries();
//  for (int j = 0; j not_eq nCdcRecoHits; j++) {
//
//    // list<short unsigned int>::iterator iter;
//    // for (iter = indexList.begin(); iter != indexList.end(); iter++) {
//    BOOST_FOREACH(int hit, indexList) {
//      if (j == hit) {
//        //B2INFO("Add CDC Reco Hit");
//        hitCounter++;
//        track.addHit(cdcRecoHits[j], 2, hitCounter);
//        //GFDetPlane aGFDetPlane = cdcRecoHits[j]->getDetPlane(trackRep);
//        //cout << "print cdc plane" << flush;
//        //aGFDetPlane.Print();
//      }
//    }
//  }//end loop over cdc RecoHits

  GFRecoHitFactory factory;

  //create RecoHitProducers for PXD, SVD and CDC
  GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
  PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);

  //GFRecoHitProducer <SVDTrueHit, SVDRecoHit> * SVDProducer;
  //SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit> (&*svdHits);

  //GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;
  //CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);

  //add producers to the factory with correct detector Id
  factory.addProducer(0, PXDProducer);
  //factory.addProducer (1, SVDProducer);
  // factory.addProducer(2, CDCProducer);

  vector <GFAbsRecoHit *> factoryHits;
  //use the factory to create RecoHits for all Hits stored in the track candidate
  factoryHits = factory.createMany(*trackCandidates[0]);
  //add created hits to the track
  track.addHitVector(factoryHits);
  track.setCandidate(*trackCandidates[0]);

  B2INFO("Total Nr of Hits assigned to the Track: " << track.getNumHits());


  /* TVector3 magField = GFFieldManager::getFieldVal(vertex);
   B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());
   TVector3 testPoint(1,1,1);
   magField = GFFieldManager::getFieldVal(testPoint);
   B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());
   testPoint.SetXYZ(-0.5,0.5,-0.5);
   magField = GFFieldManager::getFieldVal(testPoint);
   B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());*/
  //Initialize fitting algorithm and process track
//  if((nHitsL1 == 1 and nHitsL2 == 1 and nHitsL3 == 1 and nHitsL4 == 1 and nHitsL5 == 1 and nHitsL6 == 1) or m_noFilter == true){
//    if (m_useDaf == false){
//      GFKalman2 kalmanFilter(m_randomGenPtr);
//      kalmanFilter.setNumIterations(1);
//      kalmanFilter.setBlowUpFactor(m_blowUpFactor);
//      kalmanFilter.processTrack(&track);
//    } else {
//      GFDaf daf;
//      daf.processTrack(&track);
//    }
//    //track.Print();
//    //cerr << "1";
//    B2INFO("----> Status of fit: " << trackRep->getStatusFlag());
//    B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
//    B2INFO("-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
//    B2INFO("----> Chi2 of the fit: " << track.getChiSqu());
//    B2INFO("----> NDF of the fit: " << track.getNDF());
//    int genfitStatusFlag = trackRep->getStatusFlag();
//
//    if (genfitStatusFlag == 0) {
//      new(fittedTracks->AddrAt(0)) GFTrack(track);
//      ++m_fitCounter;
//    } else {
//      B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit of one track");
//      ++m_failedFitCounter;
//    }
//  } else {
//    ++m_notPerfectCounter;
//    B2INFO("Not exacly one hit in very Si layer. Track "  << m_fitCounter + m_failedFitCounter + m_notPerfectCounter<< " will not be reconstructed" );
//  }

}

void GenFitter2Module::endRun()
{
  if (m_failedFitCounter != 0) {
    B2WARNING(m_failedFitCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks could not be fitted in this run");
  }
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks had not exaclty on hit in every layer and were not fitted");
  }
}

void GenFitter2Module::terminate()
{

}



