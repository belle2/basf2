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

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <framework/logging/Logger.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/dataobjects/SVDRecoHit2D.h>
#include <pxd/dataobjects/PXDRecoHit.h>

#include <tracking/gfbfield/GFGeant4Field.h>

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
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>



#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

#include <GFDetPlane.h>
#include <GFTools.h>

//#include <TRandom.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
  Module()
{

  setDescription("Simplified trackfit module with for testing and debugging");
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("filter", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer", false);
  addParam("filterIterations", m_nGFIter, "number of Genfit iterations", 1);
//  addParam("seedForRecoHits", m_seedForRecoHits, "hack because there is no framwork wide random number seed at the moment", -1);

}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
//  if (m_seedForRecoHits >= 0) {
//    gRandom->SetSeed(m_seedForRecoHits); //this will set the seed for the global gRandom where the recoHits get there randomness for measurements from
//  }
}

void GenFitter2Module::beginRun()
{
  m_notPerfectCounter = 0;
  m_failedFitCounter = 0;
  m_fitCounter = 0;
}

void GenFitter2Module::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  B2INFO("**********   GenFitter2Module  processing event number: " << eventCounter << " ************");
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

  //fitler
  bool filterEvent = false;
  if (m_filter == true) {
    // for the filter function to get only tracks that hits specific layers
    int nHitsL1 = 0; int nHitsL2 = 0; int nHitsL3 = 0; int nHitsL4 = 0; int nHitsL5 = 0; int nHitsL6 = 0;
    for (int i = 0; i not_eq nPxdTrueHits; ++i) {
      int layerId = pxdTrueHits[i]->getSensorID().getLayer();
      if (layerId == 1) {
        ++nHitsL1;
      } else {
        ++nHitsL2;
      }
    }
    for (int i = 0; i not_eq nSvdTrueHits; ++i) {
      int layerId = svdTrueHits[i]->getSensorID().getLayer();
      if (layerId == 3) {
        ++nHitsL3;
      } else if (layerId == 4) {
        ++nHitsL4;
      } else if (layerId == 5) {
        ++nHitsL5;
      } else {
        ++nHitsL6;
      }
    }
    if (nHitsL1 not_eq 1 or nHitsL2 not_eq 1 or nHitsL3 not_eq 1 or nHitsL4 not_eq 1 or nHitsL5 not_eq 1 or nHitsL6 not_eq 1) {
      filterEvent = true;
      B2INFO("Not exacly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
      ++m_notPerfectCounter;
    }
  }

  if (filterEvent == false) { // fit the track

    StoreArray<GFTrack> fittedTracks(""); //holds the output of this module in the form of Genfit track objects

    GFFieldManager::getInstance()->init(new GFGeant4Field());

    GFTrackCand* aTrackCandPointer = trackCandidates[0];
    //get fit starting values from the MCParticle
    TVector3 vertex = aTrackCandPointer->getPosSeed();
    TVector3 vertexSigma = aTrackCandPointer->getPosError();
    TVector3 momentum = aTrackCandPointer->getDirSeed() * abs(1.0 / aTrackCandPointer->getQoverPseed());
    TVector3 dirSigma = aTrackCandPointer->getDirError();
    //int pdg = aMcParticleArray[0]->getPDG();



    //B2INFO("MCIndex: "<<mcindex);
    B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
    B2INFO("Start values: direction std: " << dirSigma.x() << "  " << dirSigma.y() << "  " << dirSigma.z());
    B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
    B2INFO("Start values: vertex std:   " << vertexSigma.x() << "  " << vertexSigma.y() << "  " << vertexSigma.z());
    B2INFO("Start values: pdg:      " << aTrackCandPointer->getPdgCode());
    GFAbsTrackRep* trackRep;
    //Now create a GenFit track with this representation

    //trackRep = new RKTrackRep(vertex, momentum, poserr, momerr, pdg);
    trackRep = new RKTrackRep(aTrackCandPointer);
    GFTrack track(trackRep, true);

    GFRecoHitFactory factory;

    //create RecoHitProducers for PXD, SVD and CDC
    GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
    PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);

    GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;
    SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdTrueHits);

    //GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;
//   CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);

    //add producers to the factory with correct detector Id
    factory.addProducer(0, PXDProducer);
    factory.addProducer(1, SVDProducer);

    vector <GFAbsRecoHit*> factoryHits;
    //use the factory to create RecoHits for all Hits stored in the track candidate
    factoryHits = factory.createMany(*trackCandidates[0]);
    /*cout << "sizeOffactoryHits " << factoryHits.size() << "\n";
    for ( int i = 0; i not_eq nPxdTrueHits; ++i){
    cout << "i= " << i << endl;
    factoryHits[i]->Print();
    cout <<pxdTrueHits[i]->getU() << " " << pxdTrueHits[i]->getV() << "\n";
    cout << (static_cast<PXDRecoHit*> (factoryHits[i]))->getUVariance() << " ";
    cout << (static_cast<PXDRecoHit*> (factoryHits[i]))->getVVariance() << " ";
    cout << (static_cast<PXDRecoHit*> (factoryHits[i]))->getUVCov() << "\n";
    factoryHits[i]->getHMatrix(trackRep).Print();
    }
      for ( int i = 0; i not_eq nSvdTrueHits; ++i){
      //pxdTrueHits[i]->
      cout << "i= " << i << endl;
    factoryHits[i+nPxdTrueHits]->Print();
    cout << svdTrueHits[i]->getU() << " " << svdTrueHits[i]->getV() << "\n";
    cout << (static_cast<SVDRecoHit2D*> (factoryHits[i+nPxdTrueHits]))->getUVariance() << " ";
    cout << (static_cast<SVDRecoHit2D*> (factoryHits[i+nPxdTrueHits]))->getVVariance() << " ";
    cout << (static_cast<SVDRecoHit2D*> (factoryHits[i+nPxdTrueHits]))->getUVCov() << "\n";
    factoryHits[i+nPxdTrueHits]->getHMatrix(trackRep).Print();
    }
    for ( int i = 0; i not_eq factoryHits.size(); ++i){
    cout << "i= " << i << endl;
    factoryHits[i]->Print();
    factoryHits[i]->getHMatrix(trackRep).Print();
    }  */

    //add created hits to the track
    track.addHitVector(factoryHits);
    track.setCandidate(*trackCandidates[0]);

    B2INFO("Total Nr of Hits assigned to the Track: " << track.getNumHits());

    //Initialize fitting algorithm and process track
    if (m_useDaf == false) {
      //GFKalman2 kalmanFilter;
      GFKalman kalmanFilter;
      kalmanFilter.setNumIterations(m_nGFIter);
      kalmanFilter.setBlowUpFactor(m_blowUpFactor);
      kalmanFilter.processTrack(&track);
    } else {
      GFDaf daf;
      daf.processTrack(&track);
    }
//    //track.Print();

    int genfitStatusFlag = trackRep->getStatusFlag();
    B2INFO("----> Status of fit: " << genfitStatusFlag);
    B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
    B2INFO("-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
    B2INFO("----> Chi2 of the fit: " << track.getChiSqu());
    B2INFO("----> NDF of the fit: " << track.getNDF());
    /*    track.Print();
        for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
          track.getHit(iHit)->Print();
        }
        */
    if (genfitStatusFlag == 0) {
      new(fittedTracks->AddrAt(0)) GFTrack(track);
      ++m_fitCounter;
    } else {
      B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit of one track in event " << eventCounter);
      ++m_failedFitCounter;
    }

  }

}

void GenFitter2Module::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks had not exactly on hit in every layer and were not fitted");
  }
  if (m_failedFitCounter != 0) {
    B2WARNING(m_failedFitCounter << " of " << m_fitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }

}

void GenFitter2Module::terminate()
{

}
