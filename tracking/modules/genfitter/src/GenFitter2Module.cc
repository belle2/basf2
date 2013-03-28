/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
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
#include <framework/dataobjects/SimpleVec.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>

#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VXDTrueHit.h>
#include <vxd/dataobjects/VXDSimpleDigiHit.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/trackCandidateHits/CDCTrackCandHit.h>
//genfit stuff
#include <GFTrack.h>
#include <GFTrackCand.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>
#include <GFConstField.h>
#include <GFFieldManager.h>
#include <GFMaterialEffects.h>
#include <GFDetPlane.h>
#include <GFTools.h>
#include <GFTGeoMaterialInterface.h>
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
#include <TGeoManager.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::CDC;
using namespace Belle2::Tracking;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
  Module()
{
  setDescription("Simplified trackfit module for debugging and testing new features before they go into the official GenFitter");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("hitsPerTrack", m_nLayerWithHit, "if this option is set only tracks with given number of hits will be fitted. Negative number deactivate this option", -1);
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
  addParam("hitType", m_hitType, "select what kind of hits are feeded to Genfit. Current Options \"TrueHit\", \"Cluster\" or \"VXDSimpleDigiHit\"", string("TrueHit"));
  addParam("smoothing", m_smoothing, "select smoothing type in Kalman filter: 0 = non; 1 = normal; 2 = fast", 2);
  addParam("dafTemperatures", m_dafTemperatures, "set the annealing scheme (temperatures) for the DAF. Length of vector will determine DAF iterations", vector<double>(1, -999.0));
  addParam("leftRightAmbiInfo", m_uselrAmbiInfo, "set true if the information to resolve the left right ambiguity from a track finder should be used", false);
}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{

  StoreArray<GFTrackCand>::required();
  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }
  // activate / deactivate material effects in genfit
  if (m_noEffects == true) {
    GFMaterialEffects::getInstance()->setNoEffects(true);
  } else {
    GFMaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    GFMaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    GFMaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
  }
  GFMaterialEffects::getInstance()->setMscModel(m_mscModel);
  //register output storeArray
  StoreArray<GFTrack>::registerPersistent();
  //set options for fitting algorithms
  m_kalmanFilter.setNumIterations(m_nGFIter);
  m_kalmanFilter.setBlowUpFactor(m_blowUpFactor);
  m_daf.setProbCut(m_probCut);
  int nDafTemps = m_dafTemperatures.size();
  if (nDafTemps == 1 && m_dafTemperatures[0] < 0.0) { // user did not set an annealing scheme. Set the default one.
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
  } else if (nDafTemps <= 10 && nDafTemps >= 1) {
    m_dafTemperatures.resize(10, -1.0);
    m_daf.setBetas(m_dafTemperatures[0], m_dafTemperatures[1], m_dafTemperatures[2], m_dafTemperatures[3], m_dafTemperatures[4], m_dafTemperatures[5], m_dafTemperatures[6], m_dafTemperatures[7], m_dafTemperatures[8], m_dafTemperatures[9]);
//    cout << "m_dafTemperatures: ";
//    for ( int i = 0; i not_eq m_dafTemperatures.size(); ++i){
//      cout << m_dafTemperatures[i] << " | ";
//    }
//    cout << endl;
  } else {
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
    B2ERROR("You either set 0 DAF temperatures or more than 10. This is not supported. The default scheme (81,8,4,1,1,1) was selected instead.");
  }

  //interpred the hittype option
  if (m_hitType == "TrueHit") {
    m_hitTypeId = 0;
  } else if (m_hitType == "VXDSimpleDigiHit") {
    m_hitTypeId = 1;
  } else if (m_hitType == "Cluster") {
    m_hitTypeId = 2;
  } else {
    B2FATAL("Hit type unknown to GenFitter2. The only options are: \"TrueHit\", \"VXDSimpleDigiHit\" or \"Cluster\" but you set \"" << m_hitType << "\"");
  }

  // Create new Translators and give them to the CDCRecoHits.
  // The way, I'm going to do it here will produce some small resource leak, but this will stop, once we go to ROOT 6 and have the possibility to use sharead_ptr
  CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(), new IdealCDCGeometryTranslator(), new SimpleTDCCountTranslator());

}

void GenFitter2Module::beginRun()
{
  m_notPerfectCounter = 0;
  m_failedFitCounter = 0;
  m_fitCounter = 0;
  m_largeAngleCounter = 0;
}

void GenFitter2Module::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "**********   GenFitter2Module processing event number: " << eventCounter << " ************");
  StoreArray<GFTrackCand> trackCandidates("");
  int nTrackCandidates = trackCandidates.getEntries();
  if (nTrackCandidates == 0) {
    B2DEBUG(100, "GenFitter2: StoreArray<GFTrackCand> is empty!");
  }

  StoreArray<GFTrack> fittedTracks; //holds the output of this module in the form of Genfit track objects
  fittedTracks.create();

  StoreArray<SVDTrueHit> svdTrueHits("");
  const int nSvdTrueHits = svdTrueHits.getEntries();
  if (nSvdTrueHits == 0) {
    B2DEBUG(100, "GenFitter2: StoreArray<SVDTrueHit> is empty!");
  }
  StoreArray<PXDTrueHit> pxdTrueHits("");
  const int nPxdTrueHits = pxdTrueHits.getEntries();
  if (nPxdTrueHits == 0) {
    B2DEBUG(100, "GenFitter2: GFTrackCandidatesCollection is empty!");
  }
  StoreArray<CDCHit> cdcHits("");
  const int nCdcHits = cdcHits.getEntries();
  if (nCdcHits == 0) {
    B2DEBUG(100, "GenFitter2: StoreArray<CDCHit> is empty!");
  }

  StoreArray<VXDSimpleDigiHit> pxdSimpleDigiHits("pxdSimpleDigiHits");
  StoreArray<VXDSimpleDigiHit> svdSimpleDigiHits("svdSimpleDigiHits");
  B2DEBUG(100, "pxdSimpleDigiHits.getEntries() " << pxdSimpleDigiHits.getEntries());
  B2DEBUG(100, "svdSimpleDigiHits.getEntries() " << svdSimpleDigiHits.getEntries());

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters("");
  const int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(149, "GenFitter2: Number of PXDClusters: " << nPXDClusters);
  if (nPXDClusters == 0) {B2DEBUG(100, "GenFitter2: PXDClustersCollection is empty!");}

  //SVD clusters
  StoreArray<SVDCluster> svdClusters("");
  const int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(149, "GenFitter2: Number of SVDClusters: " << nSVDClusters);
  if (nSVDClusters == 0) {B2DEBUG(100, "GenFitter2: SVDClustersCollection is empty!");}

  for (int iTrackCand = 0; iTrackCand not_eq nTrackCandidates; ++iTrackCand) {

    GFTrackCand* aTrackCandPointer = trackCandidates[iTrackCand];
    const int nTrackCandHits = aTrackCandPointer->getNHits();
    B2DEBUG(100, "nTrackCandHits " << nTrackCandHits);
    // if option is set ignore every track that does not have exactly 1 hit in every Si layer
    bool filterTrack = false;
    if (m_nLayerWithHit > 0) {
      if (nTrackCandHits not_eq m_nLayerWithHit) {
        filterTrack = true;
        B2DEBUG(100, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
        ++m_notPerfectCounter;
      } else {
        vector<int> layerIds(nTrackCandHits);
        for (int i = 0; i not_eq nTrackCandHits; ++i) {
          int detId = -1;
          int hitId = -1;
          aTrackCandPointer->getHit(i, detId, hitId);
          int layerId = -1;
          if (detId == Const::PXD) {
            if (m_hitTypeId == 1) {
              layerId = pxdSimpleDigiHits[hitId]->getSensorID().getLayerNumber();
            } else if (m_hitTypeId == 0) {
              layerId = pxdTrueHits[hitId]->getSensorID().getLayerNumber();
            }
          }
          if (detId == Const::SVD) {
            if (m_hitTypeId == 1) {
              layerId = svdSimpleDigiHits[hitId]->getSensorID().getLayerNumber();
            } else if (m_hitTypeId == 0) {
              layerId = svdTrueHits[hitId]->getSensorID().getLayerNumber();
            }
          }
          layerIds[i] = layerId;
        }
        sort(layerIds.begin(), layerIds.end());
        int first = layerIds[0];
        for (int l = 1; l not_eq nTrackCandHits; ++l) {
          if (first + l  not_eq layerIds[l]) {
            filterTrack = true;
            B2DEBUG(100, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
            ++m_notPerfectCounter;
            break;
          }
        }

      }
    }
    //find out if a track has a too large scattering angel and if yes ignore it ( only check if event was not filter out by the six hit only filter)
    if (m_angleCut > 0.0 and filterTrack == false) {
      // class to convert global and local coordinates into each other
      //VXD::GeoCache& aGeoCach = VXD::GeoCache::getInstance();
      for (int i = 0; i not_eq nTrackCandHits; ++i) {
        int detId = -1;
        int hitId = -1;
        aTrackCandPointer->getHit(i, detId, hitId);
        VXDTrueHit const* aVxdTrueHitPtr = NULL;
        if (detId == Const::PXD) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(pxdTrueHits[hitId]);
        }
        if (detId == Const::SVD) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(svdTrueHits[hitId]);
        }
        TVector3 pTrueIn = aVxdTrueHitPtr->getEntryMomentum();
        TVector3 pTrueOut = aVxdTrueHitPtr->getExitMomentum();
        //        const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aVxdTrueHitPtr->getSensorID());
        //        TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
        //        TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
        if (abs(pTrueIn.Angle(pTrueOut)) > m_angleCut) {
          filterTrack = true;
          ++m_largeAngleCounter;
          B2INFO("Scattering angle larger than "  << m_angleCut << ". Track " << eventCounter << " will not be reconstructed");
          break;
        }

      }

    }


    if (filterTrack == false) { // fit the track

      //get fit starting values from the from the track candidate
      TVector3 posSeed = aTrackCandPointer->getPosSeed();
      TVector3 momentumSeed = aTrackCandPointer->getMomSeed();
      TMatrixDSym covSeed = aTrackCandPointer->getCovSeed();

      B2DEBUG(100, "Start values: momentum (x,y,z,abs): " << momentumSeed.x() << "  " << momentumSeed.y() << "  " << momentumSeed.z() << " " << momentumSeed.Mag());
      B2DEBUG(100, "Start values: momentum std: " << sqrt(covSeed[3][3]) << "  " << sqrt(covSeed[4][4]) << "  " << sqrt(covSeed[5][5]));
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      B2DEBUG(100, "Start values: pos std:   " << sqrt(covSeed[0][0]) << "  " << sqrt(covSeed[1][1]) << "  " << sqrt(covSeed[2][2]));
      B2DEBUG(100, "Start values: pdg:      " << aTrackCandPointer->getPdgCode());

      //Now create a GenFit track with this representation
      RKTrackRep* trackRep = new RKTrackRep(aTrackCandPointer);
      trackRep->setPropDir(1); // setting the prop dir disables the automatic selection of the direction. This is necessary for curling tracks! (but it still will automatically change when switching between forward and backward filter)
      GFTrack track(trackRep); // the track now has ownership of the trackRep do not try do manually delete it
      if (m_smoothing == 1) {
        track.setSmoothing(true); // uhm 2 times same shit??? do something about it!
      } else if (m_smoothing == 2) {
        track.setSmoothing(true);
      }


      GFRecoHitFactory factory;

      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer =  NULL;
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer =  NULL;
      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer =  NULL;

      GFRecoHitProducer <VXDSimpleDigiHit, PXDRecoHit> * pxdSimpleDigiHitProducer =   NULL;
      GFRecoHitProducer <VXDSimpleDigiHit, SVDRecoHit2D> * svdSimpleDigiHitProducer =   NULL;

      GFRecoHitProducer <PXDCluster, PXDRecoHit> * pxdClusterProducer = NULL;
      GFRecoHitProducer <SVDCluster, SVDRecoHit> * svdClusterProducer = NULL;
      //create RecoHitProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_hitTypeId == 0) { // use the trueHits
        if (nPxdTrueHits not_eq 0) {
          PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
        if (nSvdTrueHits not_eq 0) {
          SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }
      } else if (m_hitTypeId == 1) {
        pxdSimpleDigiHitProducer =  new GFRecoHitProducer <VXDSimpleDigiHit, PXDRecoHit> (pxdSimpleDigiHits.getPtr());
        svdSimpleDigiHitProducer =  new GFRecoHitProducer <VXDSimpleDigiHit, SVDRecoHit2D> (svdSimpleDigiHits.getPtr());
        factory.addProducer(Const::PXD, pxdSimpleDigiHitProducer);
        factory.addProducer(Const::SVD, svdSimpleDigiHitProducer);
      } else if (m_hitTypeId == 2) {
        if (nPXDClusters not_eq 0) {
          pxdClusterProducer =  new GFRecoHitProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
        if (nSVDClusters not_eq 0) {
          svdClusterProducer =  new GFRecoHitProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      }
      if (nCdcHits not_eq 0) {
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
        factory.addProducer(Const::CDC, CDCProducer);
      }

      //use the factory to create RecoHits for all Hits stored in the track candidate
      vector <GFAbsRecoHit*> factoryHits = factory.createMany(*aTrackCandPointer);

      //add created hits to the track
      track.addHitVector(factoryHits);
      track.setCandidate(*aTrackCandPointer);

      //tell the CDCRecoHits how the left/right ambiguity of wire hits should be resolved (this info should come from the track finder)
      const int nHitsInTrack = track.getNumHits();
      if (m_uselrAmbiInfo == true) {
        for (int i = 0; i not_eq nHitsInTrack; ++i) {
          CDCTrackCandHit* aTrackCandHitPtr = dynamic_cast<CDCTrackCandHit*>(aTrackCandPointer->getHit(i));
          if (aTrackCandHitPtr not_eq NULL) {
            CDCRecoHit* aCdcRecoHit = static_cast<CDCRecoHit*>(track.getHit(i)); //this now has to be a CDCRecoHit because the oder of hits in GFTrack and GFTrackCand must be the same
            char lrInfo = aTrackCandHitPtr->getLeftRightResolution();
            B2DEBUG(100, "l/r: for hit with index " <<  i << " is " << int(lrInfo));
            aCdcRecoHit->setLeftRightResolution(lrInfo);
          }
        }
      }

      B2DEBUG(100, "Total Nr of Hits assigned to the Track: " << nHitsInTrack);
      /*      for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
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

                cout << "aVxdTrueHitPtr->getU() " << aTrueHitPtr->getU() << endl;
                cout << "aVxdTrueHitPtr->getGlobalTime() " << aTrueHitPtr->getGlobalTime()<< endl;
                cout << "layerId " << aTrueHitPtr->getSensorID().getLayerNumber()<< endl;
            }*/
      //process track (fit them!)
      if (m_useDaf == false) {
        m_kalmanFilter.processTrack(&track);
      } else {
        m_daf.processTrack(&track);
      }

      int genfitStatusFlag = trackRep->getStatusFlag();
      B2DEBUG(100, "----> Status of fit: " << genfitStatusFlag);
      B2DEBUG(100, "-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
      B2DEBUG(100, "-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
      B2DEBUG(100, "----> Chi2 of the fit: " << track.getChiSqu());
      B2DEBUG(100, "----> NDF of the fit: " << track.getNDF());
      //          track.Print();
      //          for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
      //            track.getHit(iHit)->Print();
      //          }

      if (genfitStatusFlag == 0) {
        fittedTracks.appendNew(track);
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
