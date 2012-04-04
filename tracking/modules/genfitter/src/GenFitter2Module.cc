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


//framework stuff
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
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VXDTrueHit.h>

#include <tracking/gfbfield/GFGeant4Field.h>
//genfit stuff
#include <GFTrack.h>
//#include <GFKalman2.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>
#include <GFConstField.h>
#include <GFFieldManager.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFMaterialEffects.h>
#include <GFDetPlane.h>
#include <GFTools.h>
// c++ stl stuff
#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
//boost stuff
#include <boost/foreach.hpp>
//root stuff
#include <TVector3.h>


using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
  Module()
{
  setDescription("Simplified trackfit module for debugging and testing new features before they go into the official GenFitter");
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("filter", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer", false);
  addParam("filterIterations", m_nGFIter, "number of Genfit iterations", 1);
  addParam("probCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", 0.001);
  addParam("energyLossBetheBloch", m_energyLossBetheBloch, "activate the material effect: EnergyLossBetheBloch", true);
  addParam("noiseBetheBloch", m_noiseBetheBloch, "activate the material effect: NoiseBetheBloch", true);
  addParam("noiseCoulomb", m_noiseCoulomb, "activate the material effect: NoiseCoulomb", true);
  addParam("energyLossBrems", m_energyLossBrems, "activate the material effect: EnergyLossBrems", true);
  addParam("noiseBrems", m_noiseBrems, "activate the material effect: NoiseBrems", true);
  addParam("noEffects", m_noEffects, "switch off all material effects in Genfit. This overwrites all individual material effects switches", false);
  addParam("angleCut", m_angleCut, "only process tracks with scattering angles smaller then angleCut (The angles are calculated from TrueHits). If negative value given no selection will take place", -1.0);
  addParam("mscModel", m_mscModel, "select the MSC model in Genfit", string("Highland"));
}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{
  // convert the geant4 geometry to a TGeo geometry
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
  //pass the magnetic field to genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());
  // activate / deactivate material effects in genfit
  if (m_noEffects == true) {
    GFMaterialEffects::getInstance()->setNoEffects(true); //not yet possible in current basd2 genfit version (but already possible upstream)
  } else {
    GFMaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    GFMaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    GFMaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
  }
  //GFMaterialEffects::getInstance()->setMscModel(m_mscModel);
  StoreArray<GFTrack> fittedTracks(""); //initialization of the the output container of this module

  //set options for fitting algorithms
  m_kalmanFilter.setNumIterations(m_nGFIter);
  m_kalmanFilter.setBlowUpFactor(m_blowUpFactor);
  m_daf.setProbCut(m_probCut);

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
  B2DEBUG(99, "**********   GenFitter2Module  processing event number: " << eventCounter << " ************");
  StoreArray<GFTrackCand> trackCandidates("");
  int nTrackCandidates = trackCandidates.getEntries();
  if (nTrackCandidates not_eq 0) {  // only try to access a track candidate if there is one

    if (nTrackCandidates == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<GFTrackCand> is empty!");
    }
    StoreArray<SVDTrueHit> svdTrueHits("");
    int nSvdTrueHits = svdTrueHits.getEntries();
    if (nSvdTrueHits == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<SVDTrueHit> is empty!");
    }
    StoreArray<PXDTrueHit> pxdTrueHits("");
    int nPxdTrueHits = pxdTrueHits.getEntries();
    if (nPxdTrueHits == 0) {
      B2DEBUG(100, "GenFitter2: GFTrackCandidatesCollection is empty!");
    }
    StoreArray<CDCHit> cdcHits("");
    int nCdcHits = cdcHits.getEntries();
    if (nCdcHits == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<CDCHit> is empty!");
    }

    GFTrackCand* aTrackCandPointer = trackCandidates[0];
    int nTrackCandHits = aTrackCandPointer->getNHits();

    // if option is set ignore every track that does not have exactly 1 hit in every Si layer
    bool filterEvent = false;
    if (m_filter == true) {
      if (nTrackCandHits not_eq 6) {
        filterEvent = true;
        B2DEBUG(99, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
        ++m_notPerfectCounter;
      } else {
        vector<int> layerIds(nTrackCandHits);
        for (int i = 0; i not_eq nTrackCandHits; ++i) {
          unsigned int detId = -1;
          unsigned int hitId = -1;
          aTrackCandPointer->getHit(i, detId, hitId);
          int layerId = -1;
          if (detId == 0) {
            layerId = pxdTrueHits[hitId]->getSensorID().getLayer();
          }
          if (detId == 1) {
            layerId = svdTrueHits[hitId]->getSensorID().getLayer();
          }
          layerIds[i] = layerId;
        }
        sort(layerIds.begin(), layerIds.end());
        for (int l = 0; l not_eq nTrackCandHits; ++l) {
          if (l + 1 not_eq layerIds[l]) {
            filterEvent = true;
            B2DEBUG(99, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
            ++m_notPerfectCounter;
            break;
          }
        }
      }
    }
    //find out if a track has a too large scattering angel and if yes ignore it ( only check if event was not filter out by the six hit only filter)
    if (m_angleCut > 0.0 and filterEvent == false) {
      // class to convert global and local coordinates into each other
      //VXD::GeoCache& aGeoCach = VXD::GeoCache::getInstance();
      for (int i = 0; i not_eq nTrackCandHits; ++i) {
        unsigned int detId = -1;
        unsigned int hitId = -1;
        aTrackCandPointer->getHit(i, detId, hitId);
        VXDTrueHit const* aVxdTrueHitPtr = NULL;
        if (detId == 0) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(pxdTrueHits[hitId]);
        }
        if (detId == 1) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(svdTrueHits[hitId]);
        }
        TVector3 pTrueIn = aVxdTrueHitPtr->getEntryMomentum();
        TVector3 pTrueOut = aVxdTrueHitPtr->getExitMomentum();
//        const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aVxdTrueHitPtr->getSensorID());
//        TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
//        TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
        if (abs(pTrueIn.Angle(pTrueOut)) > m_angleCut) {
          filterEvent = true;
          ++m_largeAngleCounter;
          B2INFO("Scattering angle larger than "  << m_angleCut << ". Track " << eventCounter << " will not be reconstructed");
          break;
        }

      }

    }


    if (filterEvent == false) { // fit the track

      StoreArray<GFTrack> fittedTracks(""); //holds the output of this module in the form of Genfit track objects


      //get fit starting values from the MCParticle
      TVector3 vertex = aTrackCandPointer->getPosSeed();
      TVector3 vertexSigma = aTrackCandPointer->getPosError();
      TVector3 momentum = aTrackCandPointer->getDirSeed() * abs(1.0 / aTrackCandPointer->getQoverPseed());
      TVector3 dirSigma = aTrackCandPointer->getDirError();


      //B2DEBUG(99,"MCIndex: "<<mcindex);
      B2DEBUG(99, "Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
      B2DEBUG(99, "Start values: direction std: " << dirSigma.x() << "  " << dirSigma.y() << "  " << dirSigma.z());
      B2DEBUG(99, "Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
      B2DEBUG(99, "Start values: vertex std:   " << vertexSigma.x() << "  " << vertexSigma.y() << "  " << vertexSigma.z());
      B2DEBUG(99, "Start values: pdg:      " << aTrackCandPointer->getPdgCode());
      RKTrackRep* trackRep;
      //Now create a GenFit track with this representation

      trackRep = new RKTrackRep(aTrackCandPointer);
      //trackRep->setPropDir(1); // setting the prop dir disables the automatic selection of the direction (but it still will automatically change when switching between forward and backward filter
      GFTrack track(trackRep, true);

      GFRecoHitFactory factory;

      //create RecoHitProducers for PXD, SVD and CDC
      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
      PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;
      SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdTrueHits);
      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;
      CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);

      //add producers to the factory with correct detector Id
      factory.addProducer(0, PXDProducer);
      factory.addProducer(1, SVDProducer);
      factory.addProducer(2, CDCProducer);

      vector <GFAbsRecoHit*> factoryHits;
      //use the factory to create RecoHits for all Hits stored in the track candidate
      factoryHits = factory.createMany(*aTrackCandPointer);

      //add created hits to the track
      track.addHitVector(factoryHits);
      track.setCandidate(*aTrackCandPointer);

      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << track.getNumHits());
      /*for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
        GFAbsRecoHit* const aGFAbsRecoHitPtr = track.getHit(iHit);

        PXDRecoHit const* const aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const * const>(aGFAbsRecoHitPtr);
        SVDRecoHit2D const* const aSvdRecoHitPtr =  dynamic_cast<SVDRecoHit2D const * const>(aGFAbsRecoHitPtr);
        VXDTrueHit const*  aTrueHitPtr = NULL;

          if (aPxdRecoHitPtr not_eq NULL) {
            cout << "this is a pxd hit" << endl;
            aTrueHitPtr = static_cast<VXDTrueHit const*>(aPxdRecoHitPtr->getTrueHit());
          } else {
            cout << "this is a svd hit" << endl;
            aTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHitPtr->getTrueHit());
          }

          //cout << "aVxdTrueHitPtr->getU() " << aTrueHitPtr->getU() << endl;
          //cout << "aVxdTrueHitPtr->getGlobalTime() " << aTrueHitPtr->getGlobalTime()<< endl;
          cout << "layerId " << aTrueHitPtr->getSensorID().getLayer()<< endl;
      }*/
      //process track (fit them!)
      if (m_useDaf == false) {
        m_kalmanFilter.processTrack(&track);
      } else {
        m_daf.processTrack(&track);
      }

      int genfitStatusFlag = trackRep->getStatusFlag();
      B2DEBUG(99, "----> Status of fit: " << genfitStatusFlag);
      B2DEBUG(99, "-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
      B2DEBUG(99, "-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
      B2DEBUG(99, "----> Chi2 of the fit: " << track.getChiSqu());
      B2DEBUG(99, "----> NDF of the fit: " << track.getNDF());
//          track.Print();
//          for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
//            track.getHit(iHit)->Print();
//          }

      if (genfitStatusFlag == 0) {
        new(fittedTracks->AddrAt(0)) GFTrack(track);
        ++m_fitCounter;
      } else {
        B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit of one track in event " << eventCounter);
        ++m_failedFitCounter;
      }

    }
  }
}

void GenFitter2Module::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2INFO(m_notPerfectCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks had not exactly on hit in every layer and were not fitted");
  }
  if (m_largeAngleCounter != 0) {
    B2WARNING(m_largeAngleCounter << " of " << m_fitCounter + m_failedFitCounter + m_largeAngleCounter << " had a scattering larger than " << m_angleCut <<  " rad and were not fitted");
  }
  if (m_failedFitCounter != 0) {
    B2WARNING(m_failedFitCounter << " of " << m_fitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }

}

void GenFitter2Module::terminate()
{

}
