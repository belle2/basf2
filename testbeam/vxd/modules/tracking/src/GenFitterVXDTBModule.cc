/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <testbeam/vxd/modules/tracking/GenFitterVXDTBModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>

#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>

#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <testbeam/vxd/reconstruction/TelRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>

#include <mdst/dataobjects/Track.h>

#include <boost/scoped_ptr.hpp>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/MeasurementFactory.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/MeasurementFactory.h>

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

REG_MODULE(GenFitterVXDTB)

GenFitterVXDTBModule::GenFitterVXDTBModule() :
  Module()
{
  setDescription(
    "Uses GenFit2 to fit tracks with support for EUDET telescopes. Needs genfit::TrackCands as input and provides genfit::Tracks and Tracks as output.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)",
           string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(""));
  addParam("TelHitsColName", m_telHitsColName, "TelHits collection", string(""));

  addParam("MCParticlesColName", m_mcParticlesColName,
           "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)", string(""));
  //select the filter and set some parameters
  addParam("FilterId", m_filterId,
           "Set to 'Kalman' use Kalman Filter, 'DAF' to use the DAF and 'simpleKalman' for the Kalman without reference track", string("DAF"));
  addParam("NMinIterations", m_nMinIter, "Minimum number of iterations for the Kalman filter", int(3));
  addParam("NMaxIterations", m_nMaxIter, "Maximum number of iterations for the Kalman filter", int(10));
  addParam("NMaxFailedHits", m_nMaxFailed, "Maximum number of of failed hits before aborting the fit", int(5));
  addParam("ProbCut", m_probCut,
           "Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001", double(0.001));
  addParam("StoreFailedTracks", m_storeFailed, "Set true if the tracks where the fit failed should also be stored in the output",
           bool(false));
  addParam("UseClusters", m_useClusters,
           "if set to true cluster hits (PXD/SVD clusters) will be used for fitting. If false Gaussian smeared trueHits will be used", true);
  addParam("PDGCodes", m_pdgCodes,
           "List of PDG codes used to set the mass hypothesis for the fit. All your codes will be tried with every track. The sign of your codes will be ignored and the charge will always come from the genfit::TrackCand. If you do not set any PDG code the code will be taken from the genfit::TrackCand. This is the default behavior)",
           vector<int>(0));
  //output
  addParam("GFTracksColName", m_gfTracksColName,
           "Name of collection holding the final genfit::Tracks (will be created by this module)", string(""));
  addParam("TracksColName", m_tracksColName,
           "Name of collection holding the final Tracks (will be created by this module). NOT IMPLEMENTED!", string(""));

  addParam("HelixOutput", m_createTextFile, "Set true if you want to have a text file with perigee helix parameters of all tracks",
           bool(false));
  addParam("DAFTemperatures", m_dafTemperatures,
           "set the annealing scheme (temperatures) for the DAF. Length of vector will determine DAF iterations", vector<double>(1, -999.0));
  addParam("energyLossBetheBloch", m_energyLossBetheBloch, "activate the material effect: EnergyLossBetheBloch", true);
  addParam("noiseBetheBloch", m_noiseBetheBloch, "activate the material effect: NoiseBetheBloch", true);
  addParam("noiseCoulomb", m_noiseCoulomb, "activate the material effect: NoiseCoulomb", true);
  addParam("energyLossBrems", m_energyLossBrems, "activate the material effect: EnergyLossBrems", true);
  addParam("noiseBrems", m_noiseBrems, "activate the material effect: NoiseBrems", true);
  addParam("noEffects", m_noEffects,
           "switch off all material effects in Genfit. This overwrites all individual material effects switches", false);
  addParam("MSCModel", m_mscModel, "Multiple scattering model", string("Highland"));
  addParam("resolveWireHitAmbi", m_resolveWireHitAmbi,
           "Determines how the ambiguity in wire hits is to be dealt with.  This only makes sense for the Kalman fitters.  Values are either 'default' (use the default for the respective fitter algorithm), 'weightedAverage', 'unweightedClosestToReference' (default for the Kalman filter), or 'unweightedClosestToPrediction' (default for the Kalman filter without reference track).",
           string("default"));

  addParam("beamSpot", m_beamSpot,
           "point to which the fitted track will be extrapolated in order to put together the TrackFitResults", vector<double>(3, 0.0));

  addParam("suppressGFExceptionOutput", m_suppressGFExceptionOutput, "Suppress error messages in GenFit.", true);

  // keep GFExceptions quiet or not
  genfit::Exception::quiet(m_suppressGFExceptionOutput);
}

GenFitterVXDTBModule::~GenFitterVXDTBModule()
{
}

void GenFitterVXDTBModule::initialize()
{

  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;

  m_failedGFTrackCandFitCounter = 0;
  m_successfulGFTrackCandFitCounter = 0;
  /*
  StoreArray<genfit::TrackCand>::required(m_gfTrackCandsColName);

  StoreArray<Track>::registerPersistent();
  StoreArray<TrackFitResult>::registerPersistent();
  StoreArray < genfit::Track >::registerPersistent(m_gfTracksColName);
  StoreArray < genfit::TrackCand >::registerPersistent();

  RelationArray::registerPersistent<genfit::Track, MCParticle>(m_gfTracksColName, m_mcParticlesColName);
  RelationArray::registerPersistent<MCParticle, Track> ();
  RelationArray::registerPersistent<genfit::Track, TrackFitResult>(m_gfTracksColName, "");
  RelationArray::registerPersistent<genfit::TrackCand, TrackFitResult>(m_gfTrackCandsColName, "");
  RelationArray::registerPersistent<genfit::TrackCand, genfit::Track>(m_gfTrackCandsColName, m_gfTracksColName);
  */

  if (!m_tracksColName.empty() and m_tracksColName != "Tracks") {
    B2ERROR("Setting a collection name with TracksColName is not implemented.");
    //TODO: implementation might also need different name for TrackFitResults?
  }

  StoreArray<Track> tracks;
  StoreArray<TrackFitResult> trackfitresults;
  StoreArray < genfit::Track > gf2tracks;
  StoreArray < genfit::TrackCand > trackcands(m_gfTrackCandsColName);
  StoreArray<MCParticle> mcparticles;

  trackcands.isRequired();

  tracks.registerPersistent();
  trackfitresults.registerPersistent();
  gf2tracks.registerPersistent();
  trackcands.registerPersistent();

  gf2tracks.registerRelationTo(mcparticles);
  mcparticles.registerRelationTo(tracks);
  gf2tracks.registerRelationTo(trackfitresults);
  trackcands.registerRelationTo(trackfitresults);
  trackcands.registerRelationTo(gf2tracks);

  if (m_createTextFile) {
    HelixParam.open("HelixParam.txt");
  }

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.");
  }

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

  // Create new Translators and give them to the CDCRecoHits.
  // The way, I'm going to do it here will produce some small resource leak, but this will stop, once we go to ROOT 6 and have the possibility to use sharead_ptr
  //   CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(), new IdealCDCGeometryTranslator(),
  //                              new RealisticTDCCountTranslator());
}

void GenFitterVXDTBModule::beginRun()
{

}

void GenFitterVXDTBModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "**********   GenFitterVXDTBModule processing event number: " << eventCounter << " ************");

  StoreArray < MCParticle > mcParticles(m_mcParticlesColName);
  B2DEBUG(149, "GenFitter: total Number of MCParticles: " << mcParticles.getEntries());
  if (mcParticles.getEntries() == 0) { B2DEBUG(100, "GenFitter: MCParticlesCollection is empty!"); }

  StoreArray < genfit::TrackCand > trackCandidates(m_gfTrackCandsColName);
  B2DEBUG(99, "GenFitter: Number of genfit::TrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2DEBUG(100, "GenFitter: genfit::TrackCandidatesCollection is empty!");

//   StoreArray < CDCHit > cdcHits(m_cdcHitsColName);
//   B2DEBUG(149, "GenFitter: Number of CDCHits: " << cdcHits.getEntries());
//   if (cdcHits.getEntries() == 0)
//     B2DEBUG(100, "GenFitter: CDCHitsCollection is empty!");

  StoreArray < SVDTrueHit > svdTrueHits(m_svdHitsColName);
  B2DEBUG(149, "GenFitter: Number of SVDHits: " << svdTrueHits.getEntries());
  if (svdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: SVDHitsCollection is empty!");

  StoreArray < PXDTrueHit > pxdTrueHits(m_pxdHitsColName);
  B2DEBUG(149, "GenFitter: Number of PXDHits: " << pxdTrueHits.getEntries());
  if (pxdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: PXDHitsCollection is empty!");

  StoreArray < TelTrueHit > telTrueHits(m_telHitsColName);
  B2DEBUG(149, "GenFitter: Number of TelHits: " << telTrueHits.getEntries());
  if (telTrueHits.getEntries() == 0)
    B2DEBUG(100, "GenFitter: TelHitsCollection is empty!");

  //Telescope clusters
  StoreArray<TelCluster> telClusters("");
  int nTelClusters = telClusters.getEntries();
  B2DEBUG(149, "GenFitter: Number of TelClusters: " << nTelClusters);
  if (nTelClusters == 0) {B2DEBUG(100, "GenFitter: TelClustersCollection is empty!");}


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

  //StoreArrays to store the fit results
  StoreArray < Track > tracks;
  StoreArray <TrackFitResult> trackFitResults;
  StoreArray < genfit::Track > gfTracks(m_gfTracksColName);

  //Relations for Tracks
  RelationArray mcParticlesToTracks(mcParticles, tracks);
  RelationArray gfTracksToTrackFitResults(gfTracks, trackFitResults);
  RelationArray gfTrackCandidatesTogfTracks(trackCandidates, gfTracks);

  RelationArray gfTrackCandidatesToTrackFitResults(trackCandidates, trackFitResults, "");//"GFTrackCandsToTrackFitResults");

  //Create a relation between the gftracks and their most probable 'mother' MC particle
  RelationArray gfTracksToMCPart(gfTracks, mcParticles);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;
  int trackFitResultCounter = 0;

  for (int iCand = 0; iCand < trackCandidates.getEntries(); ++iCand) { //loop over all track candidates
    B2DEBUG(99, "#############  Fit track candidate Nr. : " << iCand << "  ################");
    genfit::TrackCand* aTrackCandPointer = trackCandidates[iCand];


    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown

    if (m_usePdgCodeFromTrackCand == true) {
      // The following code assumes PDG code > 0, hence we take the absolute value.
      m_pdgCodes.clear(); //clear the pdg code from the last track
      m_pdgCodes.push_back(abs(aTrackCandPointer->getPdgCode()));
      B2DEBUG(100, "PDG code from track candidate will be used and it is: " << aTrackCandPointer->getPdgCode());
      if (m_pdgCodes[0] == 0) {
        B2FATAL("The current genfit::TrackCand has no valid PDG code (it is 0) AND you did not set any valid PDG Code in Genfit2Module module to use instead");
      }
    }

    const int nPdg = m_pdgCodes.size();  //number of pdg hypothesises
    bool candFitted = false;   //boolean to mark if the track candidates was fitted successfully with at least one PDG hypothesis

    for (int iPdg = 0; iPdg != nPdg; ++iPdg) {  // loop over all pdg hypothesises
      //make sure the track fit starts with the correct PDG code because the sign of the PDG code will also set the charge in the TrackRep
      TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdgCodes[iPdg]);
      B2DEBUG(99, "GenFitter: current PDG code: " << m_pdgCodes[iPdg]);
      int currentPdgCode = boost::math::sign(aTrackCandPointer->getChargeSeed()) * m_pdgCodes[iPdg];
      if (currentPdgCode == 0) {
        B2FATAL("Either the charge of the current genfit::TrackCand is 0 or you set 0 as a PDG code");
      }
      if (part->Charge() < 0.0) {
        currentPdgCode *= -1; //swap sign
      }
      //std::cout << "fitting with pdg " << currentPdgCode << " for charge " << aTrackCandPointer->getChargeSeed() << std::endl;
      //Find the particle with the correct PDG Code;
      Const::ChargedStable chargedStable = Const::pion;
      try {
        chargedStable = Const::ChargedStable(abs(currentPdgCode));
      } catch (...) {
        //use pion as default
      }


      //get fit starting values from the from the track candidate
      const TVector3& posSeed = aTrackCandPointer->getPosSeed();
      const TVector3& momentumSeed = aTrackCandPointer->getMomSeed();

      B2DEBUG(99, "Fit track with start values: ");

      B2DEBUG(100, "Start values: momentum (x,y,z,abs): " << momentumSeed.x() << "  " << momentumSeed.y() << "  " << momentumSeed.z() <<
              " " << momentumSeed.Mag());
      //B2DEBUG(100, "Start values: momentum std: " << sqrt(covSeed(3, 3)) << "  " << sqrt(covSeed(4, 4)) << "  " << sqrt(covSeed(5, 5)));
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      //B2DEBUG(100, "Start values: pos std:   " << sqrt(covSeed(0, 0)) << "  " << sqrt(covSeed(1, 1)) << "  " << sqrt(covSeed(2, 2)));
      B2DEBUG(100, "Start values: pdg:      " << currentPdgCode);

      //initialize track representation and give the seed helix parameters and cov and the pdg code to the track fitter
      // Do this in two steps, because for now we use the genfit::TrackCand from Genfit 1.
      genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(currentPdgCode);


      genfit::MeasurementFactory<genfit::AbsMeasurement> factory;

      genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit>* PXDProducer =  NULL;
      genfit::MeasurementProducer <TelTrueHit, TelRecoHit>* TelProducer =  NULL;
      genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D>* SVDProducer =  NULL;
//       genfit::MeasurementProducer <CDCHit, CDCRecoHit>* CDCProducer =  NULL;

      genfit::MeasurementProducer <PXDCluster, PXDRecoHit>* pxdClusterProducer = NULL;
      genfit::MeasurementProducer <SVDCluster, SVDRecoHit>* svdClusterProducer = NULL;
      genfit::MeasurementProducer <TelCluster, TelRecoHit>* telClusterProducer = NULL;

      //create MeasurementProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_useClusters == false) { // use the trueHits
        if (pxdTrueHits.getEntries()) {
          PXDProducer =  new genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
        if (telTrueHits.getEntries()) {
          TelProducer =  new genfit::MeasurementProducer <TelTrueHit, TelRecoHit> (telTrueHits.getPtr());
          factory.addProducer(Const::TEST, TelProducer);
        }
        if (svdTrueHits.getEntries()) {
          SVDProducer =  new genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }
      } else {
        if (nPXDClusters) {
          pxdClusterProducer =  new genfit::MeasurementProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
        if (nTelClusters) {
          telClusterProducer =  new genfit::MeasurementProducer <TelCluster, TelRecoHit> (telClusters.getPtr());
          factory.addProducer(Const::TEST, telClusterProducer);
        }
        if (nSVDClusters) {
          svdClusterProducer =  new genfit::MeasurementProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      }
//       if (cdcHits.getEntries()) {
//         CDCProducer =  new genfit::MeasurementProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
//         factory.addProducer(Const::CDC, CDCProducer);
//       }

      // The track fit needs an initial guess for the resolution.  The
      // values should roughly match the actual resolution (squared),
      // but it may be that different track-finders, regions of
      // phasespace and/or subdetectors perform better with different values.
      TMatrixDSym covSeed(6);
      covSeed(0, 0) = 1e-3;
      covSeed(1, 1) = 1e-3;
      covSeed(2, 2) = 4e-3;
      covSeed(3, 3) = 0.01e-3;
      covSeed(4, 4) = 0.01e-3;
      covSeed(5, 5) = 0.04e-3;
      aTrackCandPointer->setCovSeed(covSeed);

      genfit::Track gfTrack(*aTrackCandPointer, factory, trackRep); //create the track with the corresponding track representation

      const int nHitsInTrack = gfTrack.getNumPointsWithMeasurement();
      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << nHitsInTrack);


      //Check which hits are contributing to the track
      int nCDC = 0;
      int nSVD = 0;
      int nPXD = 0;
      int nTel = 0;

      for (unsigned int hit = 0; hit < aTrackCandPointer->getNHits(); hit++) {
        int detId = 0;
        int hitId = 0;
        aTrackCandPointer->getHit(hit, detId, hitId);
        if (detId == Const::PXD) {
          nPXD++;
        } else if (detId == Const::SVD) {
          nSVD++;
        } else if (detId == Const::TEST) {
          nTel++;
        } else if (detId == Const::CDC) {
          nCDC++;
        } else {
          B2WARNING("Hit from unknown detectorID has contributed to this track! The unknown id is: " << detId);
        }
      }
      B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ", Tel: " << nTel << ")");

      if (aTrackCandPointer->getNHits() <
          3) { // this should not be nessesary because track finder should only produce track candidates with enough hits to calculate a momentum
        B2WARNING("Genfit2Module: only " << aTrackCandPointer->getNHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
        continue;
      }

      // Select the fitter.  scoped_ptr ensures that it's destructed at the right point.
      boost::scoped_ptr<genfit::AbsKalmanFitter> fitter(0);
      if (m_filterId == "Kalman") {
        fitter.reset(new genfit::KalmanFitterRefTrack());
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
      } else if (m_filterId == "DAF") {
        // FIXME ... testing
        //fitter.reset(new genfit::DAF(false));
        fitter.reset(new genfit::DAF(true));
        ((genfit::DAF*)fitter.get())->setProbCut(m_probCut);
      } else if (m_filterId == "simpleKalman") {
        fitter.reset(new genfit::KalmanFitter(4, 1e-3, 1e3, false));
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
      } else {
        B2FATAL("Unknown filter id " << m_filterId << " requested.");
      }
      fitter->setMinIterations(m_nMinIter);
      fitter->setMaxIterations(m_nMaxIter);
      fitter->setMaxFailedHits(m_nMaxFailed);
      if (m_resolveWireHitAmbi != "default") {
        if (m_resolveWireHitAmbi == "weightedAverage") {
          fitter->setMultipleMeasurementHandling(genfit::weightedAverage);
        } else if (m_resolveWireHitAmbi == "unweightedClosestToReference") {
          fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToReference);
        } else if (m_resolveWireHitAmbi == "unweightedClosestToPrediction") {
          fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPrediction);
        } else {
          B2FATAL("Unknown wire hit ambiguity handling " << m_resolveWireHitAmbi << " requested");
        }
      }

      //now fit the track
      try {

        fitter->processTrack(&gfTrack);

        //gfTrack.Print();
        bool fitSuccess = gfTrack.hasFitStatus(trackRep);
        genfit::FitStatus* fs = 0;
        genfit::KalmanFitStatus* kfs = 0;
        if (fitSuccess) {
          fs = gfTrack.getFitStatus(trackRep);
          fitSuccess = fitSuccess && fs->isFitted();
          fitSuccess = fitSuccess && fs->isFitConverged();
          kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
          //hNit->Fill(kfs->getNumIterations());
          //if (!fitSuccess && kfs)
          //kfs->Print();
          fitSuccess = fitSuccess && kfs;
        }
        B2DEBUG(99, "-----> Fit results:");
        B2DEBUG(99, "       Fitted and converged: " << fitSuccess);
        //hSuccess->Fill(fitSuccess);
        if (fitSuccess) {
          B2DEBUG(99, "       Chi2 of the fit: " << kfs->getChi2());
          //B2DEBUG(99,"       Forward Chi2: "<<gfTrack.getForwardChi2());
          B2DEBUG(99, "       NDF of the fit: " << kfs->getBackwardNdf());
          //Calculate probability
          double pValue = gfTrack.getFitStatus()->getPVal();
          B2DEBUG(99, "       pValue of the fit: " << pValue);
        }
        //B2DEBUG(99,"       Covariance matrix: ");
        //gfTrack.getTrackRep(0)->getCov().Print();

        if (!fitSuccess) {    //if fit failed
          std::stringstream warningStreamFitFailed;
          warningStreamFitFailed << "Event " << eventCounter << ", genfit::TrackCand: " << iCand << ", PDG hypo: " << currentPdgCode <<
                                 ": fit failed. GenFit returned an error (with fitSuccess " << fitSuccess << ").";
          B2DEBUG(99, warningStreamFitFailed.str());
          ++m_failedFitCounter;
          if (m_storeFailed == true) {
            ++trackCounter;

            //Create output tracks
            gfTracks.appendNew(gfTrack);  //genfit::Track can be assigned directly
            tracks.appendNew(); //Track is created empty, helix parameters are not available because the fit failed, but other variables may give some hint on the reason for the failure

            //Create relation
            if (aTrackCandPointer->getMcTrackId() >= 0) {
              gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
            }

            //else B2WARNING("No MCParticle contributed to this track! No genfit::Track<->MCParticle relation will be created!");
            //FIXME: disabled, makes no sense with real data.

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
            //Create relations
            if (aTrackCandPointer->getMcTrackId() >= 0) {
              mcParticlesToTracks.add(aTrackCandPointer->getMcTrackId(), trackCounter);
            }
            //else B2WARNING("No MCParticle contributed to this track! No MCParticle<->Track relation will be created!");
            //FIXME: disabled, makes no sense with real data.
          }
        } else {            //fit successful
          ++m_successfulFitCounter;
          ++trackCounter;

          candFitted = true;
          //Create output tracks
          gfTracks.appendNew(gfTrack);  //genfit::Track can be assigned directly
          tracks.appendNew(); //Track is created empty, parameters are set later on

          //Create relation
          if (aTrackCandPointer->getMcTrackId() >= 0) {
            gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
          }

          //else B2WARNING("No MCParticle contributed to this track! No genfit::Track<->MCParticle relation will be created!");
          //FIXME: disabled, makes no sense for real data

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
          TVector3 pos(m_beamSpot.at(0), m_beamSpot.at(1), m_beamSpot.at(2)); //origin
          TVector3 lineDirection(0, 0, 1); // beam axis
          TVector3 poca(0., 0., 0.); //point of closest approach
          TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
          TMatrixDSym cov(6);

          try {
            //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
            genfit::MeasuredStateOnPlane mop = gfTrack.getFittedState();
            mop.extrapolateToLine(pos, lineDirection);
            mop.getPosMomCov(poca, dirInPoca, cov);

            B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
            B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

            //MH: this is new stuff...
            tracks[trackCounter]->setTrackFitResultIndex(chargedStable, trackFitResultCounter);
            //Create relations
            if (aTrackCandPointer->getMcTrackId() >= 0) {
              mcParticlesToTracks.add(aTrackCandPointer->getMcTrackId(), trackCounter);
            }

            genfit::KalmanFitStatus* fs = gfTrack.getKalmanFitStatus();
            int charge = fs->getCharge();
            double pVal = fs->getBackwardPVal();
            float bField = 1.5; //TODO: get magnetic field from genfit

            trackFitResults.appendNew(TrackFitResult(poca, dirInPoca, cov, charge, chargedStable, pVal, bField, 0, 0));

            gfTracksToTrackFitResults.add(trackCounter, trackFitResultCounter);
            gfTrackCandidatesToTrackFitResults.add(iCand, trackFitResultCounter);
            gfTrackCandidatesTogfTracks.add(iCand, trackCounter);
            trackFitResultCounter++;


            // store position
//              tracks[trackCounter]->setPosition(resultPosition);
            // store covariance matrix
//              tracks[trackCounter]->setErrorMatrix(resultCovariance);

            //store position errors
            double xErr = sqrt(cov(0, 0));
            double yErr = sqrt(cov(1, 1));
            double zErr = sqrt(cov(2, 2));
            B2DEBUG(99, "Position standard deviation: " << xErr << "  " << yErr << "  " << zErr);
//              tracks[trackCounter]->setVertexErrors(xErr, yErr, zErr);

            //store momentum errors
            double pxErr = sqrt(cov(3, 3));
            double pyErr = sqrt(cov(4, 4));
            double pzErr = sqrt(cov(5, 5));
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
            //Create relations
            if (aTrackCandPointer->getMcTrackId() >= 0) {
              mcParticlesToTracks.add(aTrackCandPointer->getMcTrackId(), trackCounter);
            }
            //else B2WARNING("No MCParticle contributed to this track! No MCParticle<->Track relation will be created!");
            //FIXME: disabled, makes no sense for real data
//              tracks[trackCounter]->setExtrapFailed(true);
          }

        }// end else for successful fits

      } catch (...) {
        B2WARNING("Something went wrong during the fit!");
        ++m_failedFitCounter;
      }

    } //end loop over all pdg hypothesis

    if (candFitted == true) m_successfulGFTrackCandFitCounter++;
    else m_failedGFTrackCandFitCounter++;

  }//end loop over all track candidates
  B2DEBUG(99, "GenFitter event summary: " << trackCounter + 1 << " tracks were processed");
}

void GenFitterVXDTBModule::endRun()
{
  B2INFO("----- GenFitter run summary");
  B2INFO("      " << m_successfulGFTrackCandFitCounter << " track candidates were fitted successfully");
  B2INFO("      in total " << m_successfulFitCounter << " tracks were fitted");
  if (m_failedFitCounter > 0) {
    B2WARNING("GenFitter: " << m_failedGFTrackCandFitCounter << " of " << m_successfulGFTrackCandFitCounter +
              m_failedGFTrackCandFitCounter << " track candidates could not be fitted in this run");
    B2WARNING("GenFitter: " << m_failedFitCounter << " of " << m_successfulFitCounter + m_failedFitCounter <<
              " tracks could not be fitted in this run");
  }
}

void GenFitterVXDTBModule::terminate()
{
  if (m_createTextFile) {
    HelixParam.close();
  }
}

