/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitterModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>

#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>

#include <tracking/dataobjects/Track.h>
#include <tracking/trackCandidateHits/CDCTrackCandHit.h>

#include <GFTrack.h>
#include <GFTrackCand.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <GFConstField.h>
#include <GFFieldManager.h>

#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <TMath.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace Tracking;

REG_MODULE(GenFitter)

GenFitterModule::GenFitterModule() :
  Module()
{

  setDescription(
    "Uses GenFit to fit tracks. Needs GFTrackCands as input and provides GFTracks and Tracks as output.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the GFTrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(""));

  addParam("MCParticlesColName", m_mcParticlesColName,
           "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)", string(""));
  //select the filter and set some parameters
  addParam("FilterId", m_filterId, "Set 0 to use Kalman Filter, 1 to use DAF", int(0));
  addParam("NIterations", m_nIter, "Number of iterations for the Kalman filter", int(1));
  addParam("ProbCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", double(0.001));
  addParam("StoreFailedTracks", m_storeFailed, "Set true if the tracks where the fit failed should also be stored in the output", bool(false));
  addParam("UseClusters", m_useClusters, "if set to true cluster hits (PXD/SVD clusters) will be used for fitting. If false Gaussian smeared trueHits will be used", false);
  addParam("PDGCodes", m_pdgCodes, "List of PDG codes used to set the mass hypothesis for the fit. All your codes will be tried with every track. The sign of your codes will be ignored and the charge will always come from the GFTrackCand. If you do not set any PDG code the code will be taken from the GFTrackCand. This is the default behavior)", vector<int>(0));
  //output
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the final GFTracks (will be created by this module)", string(""));
  addParam("TracksColName", m_tracksColName, "Name of collection holding the final Tracks (will be created by this module). NOT IMPLEMENTED!", string(""));

  addParam("HelixOutput", m_createTextFile, "Set true if you want to have a text file with perigee helix parameters of all tracks", bool(false));
  addParam("DAFTemperatures", m_dafTemperatures, "set the annealing scheme (temperatures) for the DAF. Length of vector will determine DAF iterations", vector<double>(1, -999.0));

}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{

  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;

  m_failedGFTrackCandFitCounter = 0;
  m_successfulGFTrackCandFitCounter = 0;

  StoreArray<GFTrackCand>::required(m_gfTrackCandsColName);

  StoreArray<Track>::registerPersistent();
  StoreArray<TrackFitResult>::registerPersistent();
  StoreArray < GFTrack >::registerPersistent(m_gfTracksColName);

  if (!m_tracksColName.empty() and m_tracksColName != "Tracks") {
    B2ERROR("Setting a collection name with TracksColName is not implemented.");
    //TODO: implementation might also need different name for TrackFitResults?
  }


  RelationArray::registerPersistent<GFTrack, MCParticle>(m_gfTracksColName, m_mcParticlesColName);
  RelationArray::registerPersistent<MCParticle, Track> ();
  RelationArray::registerPersistent<GFTrack, TrackFitResult>();

  if (m_createTextFile) {
    HelixParam.open("HelixParam.txt");
  }

  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }
  GFMaterialEffects::getInstance()->setMscModel("Highland");

  //read the pdgCode options and set attributes accordingly
  int nPdgCodes = m_pdgCodes.size();
  if (nPdgCodes == 0) {
    m_usePdgCodeFromTrackCand = true;
  } else {
    m_usePdgCodeFromTrackCand = false;
    for (int i = 0; i != nPdgCodes; ++i) {
      m_pdgCodes[i] = abs(m_pdgCodes[i]);
    }
  }

  //set parameters for the fitter algorithm objects
  m_kalmanFilter.setNumIterations(m_nIter);
  m_daf.setProbCut(m_probCut);
  int nDafTemps = m_dafTemperatures.size();
  if (nDafTemps == 1 && m_dafTemperatures[0] < 0.0) { // user did not set an annealing scheme. Set the default one.
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
  } else if (nDafTemps <= 10 && nDafTemps >= 1) {
    m_dafTemperatures.resize(10, -1.0);
    m_daf.setBetas(m_dafTemperatures[0], m_dafTemperatures[1], m_dafTemperatures[2], m_dafTemperatures[3], m_dafTemperatures[4], m_dafTemperatures[5], m_dafTemperatures[6], m_dafTemperatures[7], m_dafTemperatures[8], m_dafTemperatures[9]);
  } else {
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
    B2ERROR("You either set 0 DAF temperatures or more than 10. This is not supported. The default scheme (81,8,4,1,1,1) was selected instead.");
  }

  // Create new Translators and give them to the CDCRecoHits.
  // The way, I'm going to do it here will produce some small resource leak, but this will stop, once we go to ROOT 6 and have the possibility to use sharead_ptr
  CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(), new IdealCDCGeometryTranslator(), new SimpleTDCCountTranslator());
}

void GenFitterModule::beginRun()
{

}

void GenFitterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "**********   GenFitterModule processing event number: " << eventCounter << " ************");

  StoreArray < MCParticle > mcParticles(m_mcParticlesColName);
  B2DEBUG(149, "GenFitter: total Number of MCParticles: " << mcParticles.getEntries());
  if (mcParticles.getEntries() == 0) { B2DEBUG(100, "GenFitter: MCParticlesCollection is empty!"); }

  StoreArray < GFTrackCand > trackCandidates(m_gfTrackCandsColName);
  B2DEBUG(99, "GenFitter: Number of GFTrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2DEBUG(100, "GenFitter: GFTrackCandidatesCollection is empty!");

  StoreArray < CDCHit > cdcHits(m_cdcHitsColName);
  B2DEBUG(149, "GenFitter: Number of CDCHits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: CDCHitsCollection is empty!");

  StoreArray < SVDTrueHit > svdTrueHits(m_svdHitsColName);
  B2DEBUG(149, "GenFitter: Number of SVDHits: " << svdTrueHits.getEntries());
  if (svdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: SVDHitsCollection is empty!");

  StoreArray < PXDTrueHit > pxdTrueHits(m_pxdHitsColName);
  B2DEBUG(149, "GenFitter: Number of PXDHits: " << pxdTrueHits.getEntries());
  if (pxdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: PXDHitsCollection is empty!");

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters("");
  int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(149, "GenFitter: Number of PXDClusters: " << nPXDClusters);
  if (nPXDClusters == 0) {B2DEBUG(100, "GenFitter: PXDClustersCollection is empty!");}

  //SVD clusters
  StoreArray<SVDCluster> svdClusters("");
  int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(149, "GenFitter: Number of SVDClusters: " << nSVDClusters);
  if (nSVDClusters == 0) {B2DEBUG(100, "GenFitter: SVDClustersCollection is empty!");}

  if (m_filterId == 0) {
    B2DEBUG(99, "Kalman filter with " << m_nIter << " iterations will be used ");
  } else {
    B2DEBUG(99, "DAF will wit probability cut " << m_probCut << " will be used ");
  }


  //StoreArrays to store the fit results
  StoreArray < Track > tracks;
  tracks.create();
  StoreArray <TrackFitResult> trackFitResults;
  trackFitResults.create();
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);
  gfTracks.create();


  //Create a relation between the gftracks and their most probable 'mother' MC particle
  RelationArray gfTracksToMCPart(gfTracks, mcParticles);

  //Relations for Tracks
  RelationArray mcParticlesToTracks(mcParticles, tracks);
  RelationArray gfTracksToTrackFitResults(gfTracks, trackFitResults);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;
  int trackFitResultCounter = 0;

  for (int i = 0; i < trackCandidates.getEntries(); ++i) { //loop over all track candidates
    B2DEBUG(99, "#############  Fit track candidate Nr. : " << i << "  ################");
    GFTrackCand* aTrackCandPointer = trackCandidates[i];


    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown

    if (m_usePdgCodeFromTrackCand == true) {
      m_pdgCodes.clear(); //clear the pdg code from the last track
      m_pdgCodes.push_back(aTrackCandPointer->getPdgCode());
      B2DEBUG(100, "PDG code from track candidate will be used and it is: " << aTrackCandPointer->getPdgCode());
      if (m_pdgCodes[0] == 0) {
        B2FATAL("The current GFTrackCand has no valid PDG code (it is 0) AND you did not set any valid PDG Code in GenFitter module to use instead");
      }
    }

    const int nPdg = m_pdgCodes.size();  //number of pdg hypothesises
    bool candFitted = false;   //boolean to mark if the track candidates was fitted successfully with at least one PDG hypothesis

    for (int iPdg = 0; iPdg != nPdg; ++iPdg) {  // loop over all pdg hypothesises
      //make sure the track fit starts with the correct PDG code because the sign of the PDG code will also set the charge in the TrackRep
      TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdgCodes[iPdg]);
      int currentPdgCode = boost::math::sign(aTrackCandPointer->getChargeSeed()) * m_pdgCodes[iPdg];
      if (currentPdgCode == 0) {
        B2FATAL("Either the charge of the current GFTRackCand is 0 or you set 0 as a PDG code");
      }
      if (part->Charge() < 0.0) {
        currentPdgCode *= -1; //swap sign
      }

      //Find the particle with the correct PDG Code;
      Const::ChargedStable chargedStable = Const::pion;
      try {
        chargedStable = Const::ChargedStable(abs(currentPdgCode));
      } catch (...) {
        //use pion as default
      }


      RKTrackRep* trackRep = new RKTrackRep(aTrackCandPointer, currentPdgCode); //initialize track representation and give the seed helix parameters and cov and the pdg code to the track fitter

      B2DEBUG(99, "Fit track with start values: ");

      //get fit starting values from the from the track candidate
      TVector3 posSeed = aTrackCandPointer->getPosSeed();
      TVector3 momentumSeed = aTrackCandPointer->getMomSeed();
      TMatrixDSym covSeed = aTrackCandPointer->getCovSeed();
      B2DEBUG(100, "Start values: momentum (x,y,z,abs): " << momentumSeed.x() << "  " << momentumSeed.y() << "  " << momentumSeed.z() << " " << momentumSeed.Mag());
      B2DEBUG(100, "Start values: momentum std: " << sqrt(covSeed(3, 3)) << "  " << sqrt(covSeed(4, 4)) << "  " << sqrt(covSeed(5, 5)));
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      B2DEBUG(100, "Start values: pos std:   " << sqrt(covSeed(0, 0)) << "  " << sqrt(covSeed(1, 1)) << "  " << sqrt(covSeed(2, 2)));
      B2DEBUG(100, "Start values: pdg:      " << currentPdgCode);

      GFTrack gfTrack(trackRep);  //create the track with the corresponding track representation
      gfTrack.setSmoothing(true); // activate smoothing and the saving of the prediction state to avoid recalculation of prediction during smoothing
      //B2INFO("       Initial Covariance matrix: ");
      //gfTrack.getTrackRep(0)->getCov().Print();

      GFRecoHitFactory factory;

      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer =  NULL;
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer =  NULL;
      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer =  NULL;

      GFRecoHitProducer <PXDCluster, PXDRecoHit> * pxdClusterProducer = NULL;
      GFRecoHitProducer <SVDCluster, SVDRecoHit> * svdClusterProducer = NULL;
      //create RecoHitProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_useClusters == false) { // use the trueHits
        if (pxdTrueHits.getEntries() not_eq 0) {
          PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
        if (svdTrueHits.getEntries() not_eq 0) {
          SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }
      } else {
        if (nPXDClusters not_eq 0) {
          pxdClusterProducer =  new GFRecoHitProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
        if (nSVDClusters not_eq 0) {
          svdClusterProducer =  new GFRecoHitProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      }
      if (cdcHits.getEntries() not_eq 0) {
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
        factory.addProducer(Const::CDC, CDCProducer);
      }

      //use the factory to create RecoHits for all Hits stored in the track candidate
      vector <GFAbsRecoHit*> factoryHits = factory.createMany(*aTrackCandPointer);
      //add created hits to the track
      gfTrack.addHitVector(factoryHits);
      gfTrack.setCandidate(*aTrackCandPointer);
      const int nHitsInTrack = gfTrack.getNumHits();
      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << nHitsInTrack);
      //tell the CDCRecoHits how the left/right ambiguity of wire hits should be resolved (this info should come from the track finder)
      for (int j = 0; j not_eq nHitsInTrack; ++j) {
        CDCTrackCandHit* aTrackCandHitPtr =  dynamic_cast<CDCTrackCandHit*>(aTrackCandPointer->getHit(j));
        if (aTrackCandHitPtr not_eq NULL) {
          CDCRecoHit* aCdcRecoHit = static_cast<CDCRecoHit*>(gfTrack.getHit(j)); //this now has to be a CDCRecoHit because the oder of hits in GFTrack and GFTrackCand must be the same
          char lrInfo = aTrackCandHitPtr->getLeftRightResolution();
          B2DEBUG(100, "l/r: for hit with index " <<  j << " is " << int(lrInfo));
          aCdcRecoHit->setLeftRightResolution(lrInfo);
        }
      }

      //Check which hits are contributing to the track
      int nCDC = 0;
      int nSVD = 0;
      int nPXD = 0;

      for (unsigned int hit = 0; hit < gfTrack.getNumHits(); hit++) {
        int detId = 0;
        int hitId = 0;
        aTrackCandPointer->getHit(hit, detId, hitId);
        if (detId == Const::PXD) {
          nPXD++;
        } else if (detId == Const::SVD) {
          nSVD++;
        } else if (detId == Const::CDC) {
          nCDC++;
        } else {
          B2WARNING("Hit from unknown detectorID has contributed to this track! The unknown id is: " << detId);
        }
      }
      B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ")");

      if (gfTrack.getNumHits() < 3) { // this should not be nessesary because track finder should only produce track candidates with enough hits to calculate a momentum
        B2WARNING("GenFitter: only " << gfTrack.getNumHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
      } else {

        //now fit the track
        try {

          if (m_filterId == 0) {
            m_kalmanFilter.processTrack(&gfTrack);
          } else {
            m_daf.processTrack(&gfTrack);
          }

          //gfTrack.Print();
          int genfitStatusFlag = trackRep->getStatusFlag();
          //StatusFlag == 0 means fit was successful
          B2DEBUG(99, "-----> Fit results:");
          B2DEBUG(99, "       Status of fit: " << genfitStatusFlag);
          B2DEBUG(99, "       Chi2 of the fit: " << gfTrack.getChiSqu());
          //B2DEBUG(99,"       Forward Chi2: "<<gfTrack.getForwardChiSqu());
          B2DEBUG(99, "       NDF of the fit: " << gfTrack.getNDF());
          //Calculate probability
          double pValue = gfTrack.getPVal();
          B2DEBUG(99, "       pValue of the fit: " << pValue);
          //B2DEBUG(99,"       Covariance matrix: ");
          //gfTrack.getTrackRep(0)->getCov().Print();

          if (genfitStatusFlag != 0) {    //if fit failed
            B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit!");
            ++m_failedFitCounter;
            if (m_storeFailed == true) {
              ++trackCounter;

              //Create output tracks
              gfTracks.appendNew(gfTrack);  //GFTrack can be assigned directly
              tracks.appendNew(); //Track is created empty, helix parameters are not available because the fit failed, but other variables may give some hint on the reason for the failure

              //Create relation
              if (aTrackCandPointer->getMcTrackId() != -999) {
                gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
              }

              else B2WARNING("No MCParticle contributed to this track! No GFTrack<->MCParticle relation will be created!");

              //Set non-helix parameters
              tracks[trackCounter]->setTrackFitResultIndex(chargedStable, -999);
              /*                            tracks[trackCounter]->setFitFailed(true);
                                          tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
                                          tracks[trackCounter]->setNHits(gfTrack.getNumHits());
                                          tracks[trackCounter]->setNCDCHits(nCDC);
                                          tracks[trackCounter]->setNSVDHits(nSVD);
                                          tracks[trackCounter]->setNPXDHits(nPXD);
                                          tracks[trackCounter]->setMCId(aTrackCandPointer->getMcTrackId());
                                          tracks[trackCounter]->setPDG(aTrackCandPointer->getPdgCode());
                                          //tracks[trackCounter]->setPurity(aTrackCandPointer->getDip()); //setDip will be deleted soon. If purity is used it has to be passed differently to the Track class
                                          tracks[trackCounter]->setPValue(pValue);
                                          //Set helix parameters
                                          tracks[trackCounter]->setD0(-999);
                                          tracks[trackCounter]->setPhi(-999);
                                          tracks[trackCounter]->setOmega(gfTrack.getCharge());
                                          tracks[trackCounter]->setZ0(-999);
                                          tracks[trackCounter]->setCotTheta(-999);
                            */
            }
          } else {            //fit successful
            ++m_successfulFitCounter;
            ++trackCounter;

            candFitted = true;
            //Create output tracks
            gfTracks.appendNew(gfTrack);  //GFTrack can be assigned directly
            tracks.appendNew(); //Track is created empty, parameters are set later on

            //Create relation
            if (aTrackCandPointer->getMcTrackId() != -999) {
              gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
            }

            else B2WARNING("No MCParticle contributed to this track! No GFTrack<->MCParticle relation will be created!");

            //Set non-helix parameters
            /*            tracks[trackCounter]->setFitFailed(false);
                        tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
                        tracks[trackCounter]->setNHits(gfTrack.getNumHits());
                        tracks[trackCounter]->setNCDCHits(nCDC);
                        tracks[trackCounter]->setNSVDHits(nSVD);
                        tracks[trackCounter]->setNPXDHits(nPXD);
                        tracks[trackCounter]->setMCId(aTrackCandPointer->getMcTrackId());
                        tracks[trackCounter]->setPDG(aTrackCandPointer->getPdgCode());
                        //tracks[trackCounter]->setPurity(aTrackCandPointer->getDip()); //setDip will be deleted soon. If purity is used it has to be passed differently to the Track class
                        tracks[trackCounter]->setPValue(pValue);
                        tracks[trackCounter]->setExtrapFailed(false);
                        */

            //To calculate the correct starting helix parameters, one has to extrapolate the track to its 'start' (here: take point of closest approach to the origin)

            //Find the point of closest approach of the track to the origin
            TVector3 pos(0., 0., 0.); //origin
            TVector3 poca(0., 0., 0.); //point of closest approach
            TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

            try {
              //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
              gfTrack.getCardinalRep()->extrapolateToPoint(pos, poca, dirInPoca);

              B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
              B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

              //Now create a reference plane to get momentum and vertex position
              GFDetPlane plane(poca, dirInPoca);
              TVector3 testPos(10., 10., 1.);
              //GFDetPlane testPlane(testPos, dirInPoca);

              //get momentum, position and covariance matrix
              TVector3 resultPosition;
              TVector3 resultMomentum;
              TMatrixDSym resultCovariance;
              gfTrack.getPosMomCov(plane, resultPosition, resultMomentum, resultCovariance);
              TMatrixF newResultCovariance(6, 6);
              for (int ii = 0; ii < 6; ii++) {
                for (int jj = 0; jj < 6; jj++) {
                  newResultCovariance(ii, jj) = resultCovariance(ii, jj);
                }
              }

              //MH: this is new stuff...
              tracks[trackCounter]->setTrackFitResultIndex(chargedStable, trackFitResultCounter);
              //Create relations
              if (aTrackCandPointer->getMcTrackId() != -999) {
                mcParticlesToTracks.add(aTrackCandPointer->getMcTrackId(), trackCounter);
              }

              TrackFitResult* newTrackFitResult = trackFitResults.appendNew();
              newTrackFitResult->setCharge((int)gfTrack.getCharge());
              newTrackFitResult->setParticleType(chargedStable);
              newTrackFitResult->setMomentum(resultMomentum);
              newTrackFitResult->setPosition(resultPosition);
              newTrackFitResult->setCovariance6(newResultCovariance);
              newTrackFitResult->setPValue(gfTrack.getPVal());
              gfTracksToTrackFitResults.add(trackCounter, trackFitResultCounter);
              trackFitResultCounter++;



              // store position
//              tracks[trackCounter]->setPosition(resultPosition);
              // store covariance matrix
//              tracks[trackCounter]->setErrorMatrix(resultCovariance);

              //store position errors
              double xErr = sqrt(resultCovariance(0, 0));
              double yErr = sqrt(resultCovariance(1, 1));
              double zErr = sqrt(resultCovariance(2, 2));
              B2DEBUG(99, "Position standard deviation: " << xErr << "  " << yErr << "  " << zErr);
//              tracks[trackCounter]->setVertexErrors(xErr, yErr, zErr);

              //store momentum errors
              double pxErr = sqrt(resultCovariance(3, 3));
              double pyErr = sqrt(resultCovariance(4, 4));
              double pzErr = sqrt(resultCovariance(5, 5));
              B2DEBUG(99, "Momentum standard deviation: " << pxErr << "  " << pyErr << "  " << pzErr);
//              tracks[trackCounter]->setPErrors(pxErr, pyErr, pzErr);


              //Now calculate the parameters for helix parametrisation to fill the Track objects

              //calculate transverse momentum
              //double pt = sqrt(resultMomentum.x() * resultMomentum.x() + resultMomentum.y() * resultMomentum.y());

              //determine angle phi for perigee parametrisation, distributed from -pi to pi
              //double phi = atan2(dirInPoca.y() , dirInPoca.x());

              //determine d0 sign for perigee parametrization
              //double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x() + poca.y() * dirInPoca.y());

              //coefficient to illiminate the B field and get the 'pure' curvature
              //double alpha = 1 / (1.5 * 0.00299792458);

              //Now set the helix parameters for perigee parametrization
              /*              tracks[trackCounter]->setD0(d0Sign * sqrt(poca.x() * poca.x() + poca.y() * poca.y()));
                            tracks[trackCounter]->setPhi(phi);
                            tracks[trackCounter]->setOmega((gfTrack.getCharge() / (pt * alpha)));
                            tracks[trackCounter]->setZ0(poca.z());
                            tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));
              */
              //Print helix parameters
              /*              B2DEBUG(99, ">>>>>>> Helix Parameters <<<<<<<");
                            B2DEBUG(99, "D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
                            B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
                            //Additional check
                            B2DEBUG(99, "Recalculate momentum from perigee: px: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha)) * (cos(tracks[trackCounter]->getPhi())) << "  py: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*sin(tracks[trackCounter]->getPhi()) << "  pz: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*tracks[trackCounter]->getCotTheta());
                            B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
              */
              if (m_createTextFile) {
                //Additional code
                //print helix parameter to a file
                //useful if one like to quickly plot track trajectories
                //-------------------------------------
                /*                HelixParam << tracks[trackCounter]->getD0() << " \t"
                                           << tracks[trackCounter]->getPhi() << " \t"
                                           << tracks[trackCounter]->getOmega() << " \t"
                                           << tracks[trackCounter]->getZ0() << " \t"
                                           << tracks[trackCounter]->getCotTheta() << "\t" << poca.x()
                                           << "\t" << poca.y() << "\t" << poca.z() << endl;
                                //----------------------------------------
                  */              //end additional code
              }
            }

            catch (...) {
              B2WARNING("Something went wrong during the extrapolation of fit results!");
//              tracks[trackCounter]->setExtrapFailed(true);
            }

          }// end else for successful fits

        } catch (...) {
          B2WARNING("Something went wrong during the fit!");
          ++m_failedFitCounter;
        }
      }
      factory.clear();
    } //end loop over all pdg hypothesis

    if (candFitted == true) m_successfulGFTrackCandFitCounter++;
    else m_failedGFTrackCandFitCounter++;

  }//end loop over all track candidates
  B2DEBUG(99, "GenFitter event summary: " << trackCounter + 1 << " tracks were processed");

}

void GenFitterModule::endRun()
{
  B2INFO("----- GenFitter run summary")
  B2INFO("      " << m_successfulGFTrackCandFitCounter << " track candidates were fitted successfully");
  B2INFO("      in total " << m_successfulFitCounter << " tracks were fitted");
  if (m_failedFitCounter > 0) {
    B2WARNING("GenFitter: " << m_failedGFTrackCandFitCounter << " of " << m_successfulGFTrackCandFitCounter + m_failedGFTrackCandFitCounter << " track candidates could not be fitted in this run");
    B2WARNING("GenFitter: " << m_failedFitCounter << " of " << m_successfulFitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }
}

void GenFitterModule::terminate()
{
  if (m_createTextFile) {
    HelixParam.close();
  }
}

