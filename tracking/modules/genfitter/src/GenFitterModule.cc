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

#include <framework/logging/Logger.h>

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

#include <GFTrack.h>
#include <GFTrackCand.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFMaterialEffects.h>

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

#include <TMath.h>


using namespace std;
using namespace Belle2;
using namespace CDC;

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

  //the tracks from MCTrackFinder are treated slightly different from tracks from pattern recognition (uknown pdg), so this parameter should be set true from mcTrack and false for pattern reco tracks
  addParam("mcTracks", m_mcTracks, "Set true if the track candidates are from MCTrackFinder, set false if they are coming from the pattern recognition", bool(true));
  //select the filter and set some parameters
  addParam("FilterId", m_filterId, "Set 0 to use Kalman Filter, 1 to use DAF", int(0));
  addParam("NIterations", m_nIter, "Number of iterations for the Kalman filter", int(1));
  addParam("ProbCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", double(0.001));
  addParam("StoreFailedTracks", m_storeFailed, "Set true if the tracks where the fit failed should also be stored in the output", bool(false));
  addParam("pdg", m_pdg, "Set the pdg hypothesis (positive charge) for the track (if set to -999, MC/default pdg will be used)", int(-999));
  addParam("allPDG", m_allPDG, "Set true if you want each track fitted 4 times with different pdg hypothesises (-11,-13, 211, 321), active only for pattern recognition tracks", bool(false));
  addParam("UseClusters", m_useClusters, "if set to true cluster hits (PXD/SVD clusters) will be used for fitting. If false Gaussian smeared trueHits will be used", false);
  //output
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the final GFTracks (will be created by this module)", string(""));
  addParam("TracksColName", m_tracksColName, "Name of collection holding the final Tracks (will be created by this module)", string(""));

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

  StoreArray<Track>::registerPersistent(m_tracksColName);
  StoreArray < GFTrack >::registerPersistent(m_gfTracksColName);


  RelationArray::registerPersistent<GFTrack, MCParticle>(m_gfTracksColName, m_mcParticlesColName);

  if (m_createTextFile) {
    HelixParam.open("HelixParam.txt");
  }

  //convert geant4 geometry to TGeo geometry
  //in the moment tesselated solids used for the glue within the PXD cannot be converted to TGeo, the general solution still has to be found, at the moment you can just comment out lines 6 and 13 in  pxd/data/PXD-Components.xml.
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();

  //get the magnetic field
  GFFieldManager::getInstance()->init(new GFGeant4Field());
  GFMaterialEffects::getInstance()->setMscModel("Highland");

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
  B2DEBUG(99, "**********   GenFitterModule  ************");

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
  if (nPXDClusters == 0) {B2DEBUG(100, "GenFitter2: PXDClustersCollection is empty!");}

  //SVD clusters
  StoreArray<SVDCluster> svdClusters("");
  int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(149, "GenFitter: Number of SVDClusters: " << nSVDClusters);
  if (nSVDClusters == 0) {B2DEBUG(100, "GenFitter2: SVDClustersCollection is empty!");}

  if (m_filterId == 0) {
    B2DEBUG(99, "Kalman filter with " << m_nIter << " iterations will be used ");
  }

  else {
    B2DEBUG(99, "DAF will wit probability cut " << m_probCut << " will be used ");
  }


  //StoreArrays to store the fit results
  StoreArray < Track > tracks(m_tracksColName);
  tracks.create();
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);
  gfTracks.create();


  //Create a relation between the gftracks and their most probable 'mother' MC particle
  RelationArray gfTracksToMCPart(gfTracks, mcParticles);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;

  for (int i = 0; i < trackCandidates.getEntries(); ++i) { //loop over all track candidates
    B2DEBUG(99, "#############  Fit track candidate Nr. : " << i << "  ################");

    GFAbsTrackRep* trackRep;  //initialize track representation

    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown

    vector<int> pdg;   //vector to store the pdg hypothesises
    if (m_mcTracks == true) {
      //for GFTrackCandidates from MCTrackFinder all information is already there
      //check for user chosen pdg, otherwise put the true pdg in the vector
      if (m_pdg != -999) {
        pdg.push_back(m_pdg);
      } else pdg.push_back(trackCandidates[i]->getPdgCode());
    }

    else {
      //the idea is to use different possible pdg values (with correct charge) and fit them all and only afterwards select the best hypothesis
      if (m_allPDG == true) {
        pdg.push_back(-11);
        pdg.push_back(-13);
        pdg.push_back(211);
        pdg.push_back(321);
      } else {
        if (m_pdg != -999) {
          pdg.push_back(m_pdg);
        } else pdg.push_back(-13);    //just choose some random common pdg, in this case muon
      }
    }

    int pdgCounter = pdg.size();   //number of pdg hypothesises

    bool candFitted = false;   //boolean to mark if the track candidates was fitted successfully with at least one PDG hypothesis

    while (pdgCounter > 0) {  //while loop over all pdg hypothesises
      if (m_mcTracks == true && m_pdg == -999)trackCandidates[i]->setPdgCode(pdg.at(pdgCounter - 1));
      else trackCandidates[i]->setPdgCode(int(TMath::Sign(1., trackCandidates[i]->getQoverPseed()) * pdg.at(pdgCounter - 1)));

      trackRep = new RKTrackRep(trackCandidates[i]);

      if (m_mcTracks) {
        B2DEBUG(99, "Fit MCTrack with start values: ");
      } else {
        B2DEBUG(99, "Fit pattern reco track with start values: ");
      }

      B2DEBUG(99, "            momentum: " << trackCandidates[i]->getDirSeed().x() << "  " << trackCandidates[i]->getDirSeed().y() << "  " << trackCandidates[i]->getDirSeed().z());
      B2DEBUG(99, "            vertex:   " << trackCandidates[i]->getPosSeed().x() << "  " << trackCandidates[i]->getPosSeed().y() << "  " << trackCandidates[i]->getPosSeed().z());
      B2DEBUG(99, "            pdg:      " << trackCandidates[i]->getPdgCode());

      GFTrack gfTrack(trackRep, true);  //create the track with the corresponding track representation

      //B2INFO("       Initial Covariance matrix: ");
      //gfTrack.getTrackRep(0)->getCov().Print();

      GFRecoHitFactory factory;

      //create RecoHitProducers for PXD, SVD and CDC
      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;



      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;


      GFRecoHitProducer <PXDCluster, PXDRecoHit> * pxdClusterProducer;
      GFRecoHitProducer <SVDCluster, SVDRecoHit> * svdClusterProducer;

      //create RecoHitProducers for PXD, SVD and CDC
      if (m_useClusters == false) { // use the trueHits
        PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);
        SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdTrueHits);
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);
      } else {
        pxdClusterProducer =  new GFRecoHitProducer <PXDCluster, PXDRecoHit> (&*pxdClusters);
        svdClusterProducer =  new GFRecoHitProducer <SVDCluster, SVDRecoHit> (&*svdClusters);
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);
      }

      if (m_useClusters == false) { // use the trueHits
        factory.addProducer(0, PXDProducer);
        factory.addProducer(1, SVDProducer);
        factory.addProducer(2, CDCProducer);
      } else { // use the cluster hits
        factory.addProducer(0, pxdClusterProducer);
        factory.addProducer(1, svdClusterProducer);
        factory.addProducer(2, CDCProducer);
      }

      vector <GFAbsRecoHit*> factoryHits;
      //use the factory to create RecoHits for all Hits stored in the track candidate
      factoryHits = factory.createMany(*trackCandidates[i]);
      //add created hits to the track
      gfTrack.addHitVector(factoryHits);
      gfTrack.setCandidate(*trackCandidates[i]);

      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << gfTrack.getNumHits());

      //Check which hits are contributing to the track
      int nCDC = 0;
      int nSVD = 0;
      int nPXD = 0;

      for (unsigned int hit = 0; hit < gfTrack.getNumHits(); hit++) {
        unsigned int detId = 0;
        unsigned int hitId = 0;
        trackCandidates[i]->getHit(hit, detId, hitId);
        if (detId == 0) nPXD++;
        if (detId == 1) nSVD++;
        if (detId == 2) nCDC++;
        if (detId != 0 && detId != 1 && detId != 2) B2WARNING("Hit from unknown detectorID has contributed to this track!");
      }

      B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ")");

      if (gfTrack.getNumHits() < 3) {
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
          double pValue = TMath::Prob(gfTrack.getChiSqu(), gfTrack.getNDF());
          B2DEBUG(99, "       pValue of the fit: " << pValue);
          //B2DEBUG(99,"       Covariance matrix: ");
          //gfTrack.getTrackRep(0)->getCov().Print();

          if (genfitStatusFlag != 0) {    //if fit failed
            B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit!");
            ++m_failedFitCounter;
            if (m_storeFailed == true) {
              ++trackCounter;

              //Create output tracks
              new(gfTracks->AddrAt(trackCounter)) GFTrack(gfTrack);  //GFTrack can be assigned directly
              new(tracks->AddrAt(trackCounter)) Track(); //Track is created empty, helix parameters are not available because the fit failed, but other variables may give some hint on the reason for the failure

              //Create relation
              if (trackCandidates[i]->getMcTrackId() != -999) {
                gfTracksToMCPart.add(trackCounter, trackCandidates[i]->getMcTrackId());
              }

              else B2WARNING("No MCParticle contributed to this track! No GFTrack<->MCParticle relation will be created!");

              //Set non-helix parameters
              tracks[trackCounter]->setFitFailed(true);
              tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
              tracks[trackCounter]->setNHits(gfTrack.getNumHits());
              tracks[trackCounter]->setNCDCHits(nCDC);
              tracks[trackCounter]->setNSVDHits(nSVD);
              tracks[trackCounter]->setNPXDHits(nPXD);
              tracks[trackCounter]->setMCId(trackCandidates[i]->getMcTrackId());
              tracks[trackCounter]->setPDG(trackCandidates[i]->getPdgCode());
              tracks[trackCounter]->setPurity(trackCandidates[i]->getDip());
              tracks[trackCounter]->setPValue(pValue);
              //Set helix parameters
              tracks[trackCounter]->setD0(-999);
              tracks[trackCounter]->setPhi(-999);
              tracks[trackCounter]->setOmega(gfTrack.getCharge());
              tracks[trackCounter]->setZ0(-999);
              tracks[trackCounter]->setCotTheta(-999);

            }
          } else {            //fit successful
            ++m_successfulFitCounter;
            ++trackCounter;

            candFitted = true;
            //Create output tracks
            new(gfTracks->AddrAt(trackCounter)) GFTrack(gfTrack);  //GFTrack can be assigned directly

            new(tracks->AddrAt(trackCounter)) Track();  //Track is created empty, parameters are set later on

            //Create relation
            if (trackCandidates[i]->getMcTrackId() != -999) {
              gfTracksToMCPart.add(trackCounter, trackCandidates[i]->getMcTrackId());
            }

            else B2WARNING("No MCParticle contributed to this track! No GFTrack<->MCParticle relation will be created!");

            //Set non-helix parameters
            tracks[trackCounter]->setFitFailed(false);
            tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
            tracks[trackCounter]->setNHits(gfTrack.getNumHits());
            tracks[trackCounter]->setNCDCHits(nCDC);
            tracks[trackCounter]->setNSVDHits(nSVD);
            tracks[trackCounter]->setNPXDHits(nPXD);
            tracks[trackCounter]->setMCId(trackCandidates[i]->getMcTrackId());
            tracks[trackCounter]->setPDG(trackCandidates[i]->getPdgCode());
            tracks[trackCounter]->setPurity(trackCandidates[i]->getDip());
            tracks[trackCounter]->setPValue(pValue);
            tracks[trackCounter]->setExtrapFailed(false);

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
              GFDetPlane testPlane(testPos, dirInPoca);
              TMatrixT<double> testCovariance;

              //get momentum, position and covariance matrix
              TVector3 resultPosition;
              TVector3 resultMomentum;
              TMatrixT<double> resultCovariance;
              gfTrack.getPosMomCov(plane, resultPosition, resultMomentum, resultCovariance);

              // store position
              tracks[trackCounter]->setPosition(resultPosition);
              // store covariance matrix
              tracks[trackCounter]->setErrorMatrix(resultCovariance);

              //store position errors
              double xErr = sqrt(resultCovariance[0][0]);
              double yErr = sqrt(resultCovariance[1][1]);
              double zErr = sqrt(resultCovariance[2][2]);
              B2DEBUG(99, "Position standard deviation: " << xErr << "  " << yErr << "  " << zErr);
              tracks[trackCounter]->setVertexErrors(xErr, yErr, zErr);

              //store momentum errors
              double pxErr = sqrt(resultCovariance[3][3]);
              double pyErr = sqrt(resultCovariance[4][4]);
              double pzErr = sqrt(resultCovariance[5][5]);
              B2DEBUG(99, "Momentum standard deviation: " << pxErr << "  " << pyErr << "  " << pzErr);
              tracks[trackCounter]->setPErrors(pxErr, pyErr, pzErr);


              //Now calculate the parameters for helix parametrisation to fill the Track objects

              //calculate transverse momentum
              double pt = sqrt(gfTrack.getMom(plane).x() * gfTrack.getMom(plane).x() + gfTrack.getMom(plane).y() * gfTrack.getMom(plane).y());

              //determine angle phi for perigee parametrisation, distributed from -pi to pi
              double phi = atan2(dirInPoca.y() , dirInPoca.x());

              //determine d0 sign for perigee parametrization
              double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x() + poca.y() * dirInPoca.y());

              //coefficient to illiminate the B field and get the 'pure' curvature

              double alpha = 1 / (1.5 * 0.00299792458);

              //Now set the helix parameters for perigee parametrization
              tracks[trackCounter]->setD0(d0Sign * sqrt(poca.x() * poca.x() + poca.y() * poca.y()));
              tracks[trackCounter]->setPhi(phi);
              tracks[trackCounter]->setOmega((gfTrack.getCharge() / (pt * alpha)));
              tracks[trackCounter]->setZ0(poca.z());
              tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));

              //Print helix parameters
              B2DEBUG(99, ">>>>>>> Helix Parameters <<<<<<<");
              B2DEBUG(99, "D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
              B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
              //Additional check
              B2DEBUG(99, "Recalculate momentum from perigee: px: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha)) * (cos(tracks[trackCounter]->getPhi())) << "  py: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*sin(tracks[trackCounter]->getPhi()) << "  pz: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*tracks[trackCounter]->getCotTheta());
              B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");

              if (m_createTextFile) {
                //Additional code
                //print helix parameter to a file
                //useful if one like to quickly plot track trajectories
                //-------------------------------------
                HelixParam << tracks[trackCounter]->getD0() << " \t"
                           << tracks[trackCounter]->getPhi() << " \t"
                           << tracks[trackCounter]->getOmega() << " \t"
                           << tracks[trackCounter]->getZ0() << " \t"
                           << tracks[trackCounter]->getCotTheta() << "\t" << poca.x()
                           << "\t" << poca.y() << "\t" << poca.z() << endl;
                //----------------------------------------
                //end additional code
              }
            }

            catch (...) {
              B2WARNING("Something went wrong during the extrapolation of fit results!");
              tracks[trackCounter]->setExtrapFailed(true);
            }

          }// end else for successful fits

        } catch (...) {
          B2WARNING("Something went wrong during the fit!");
          ++m_failedFitCounter;
        }


      } //end loop over all track candidates

      factory.clear();
      pdgCounter--;
    } //end while

    if (candFitted == true) m_successfulGFTrackCandFitCounter++;
    else m_failedGFTrackCandFitCounter++;

  }// end else (track has hits)
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

