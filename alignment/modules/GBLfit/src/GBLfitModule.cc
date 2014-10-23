/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contacts: Tadeas Bilka and Sergey Yashchenko                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <alignment/modules/GBLfit/GBLfitModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>


#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
#include <testbeam/vxd/dataobjects/TelCluster.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <testbeam/vxd/reconstruction/TelRecoHit.h>
#endif

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
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

#include <TFile.h>
#include <TH1.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
/*
TFile *file;
TH1F *hist;
TH1F *chi2;
TH1F *chi2ondf;
*/
REG_MODULE(GBLfit)

GBLfitModule::GBLfitModule() :
  Module(), m_usePdgCodeFromTrackCand(true), m_milleFile(0)
{

  setDescription(
    "Uses GenFit2 to fit tracks. Needs genfit::TrackCands as input and provides genfit::Tracks and Tracks as output.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(""));
  addParam("TelHitsColName", m_telHitsColName, "TelHits collection", string(""));

  addParam("MCParticlesColName", m_mcParticlesColName,
           "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)", string(""));
  addParam("UseClusters", m_useClusters, "if set to true cluster hits (PXD/SVD clusters) will be used for fitting. If false Gaussian smeared trueHits will be used", true);
  addParam("PDGCodes", m_pdgCodes, "List of PDG codes used to set the mass hypothesis for the fit. All your codes will be tried with every track. The sign of your codes will be ignored and the charge will always come from the genfit::TrackCand. If you do not set any PDG code the code will be taken from the genfit::TrackCand. This is the default behavior)", vector<int>(0));

  addParam("resolveWireHitAmbi", m_resolveWireHitAmbi, "Determines how the ambiguity in wire hits is to be dealt with.  This only makes sense for the Kalman fitters.  Values are either 'default' (use the default for the respective fitter algorithm), 'weightedAverage', 'unweightedClosestToReference' (default for the Kalman filter), or 'unweightedClosestToPrediction' (default for the Kalman filter without reference track).", string("default"));

  addParam("beamSpot", m_beamSpot, "point to which the fitted track will be extrapolated in order to put together the TrackFitResults", vector<double>(3, 0.0));

  addParam("suppressGFExceptionOutput", m_suppressGFExceptionOutput, "Suppress error messages in GenFit.", true);
  addParam("StoreFailedTracks", m_storeFailed, "Set true if the tracks where the fit failed should also be stored in the output", bool(false));
  // keep GFExceptions quiet or not
  genfit::Exception::quiet(m_suppressGFExceptionOutput);

  addParam("internalIterations", m_gblInternalIterations, "GBL: internal downweighting setting (separatd by ',' for each external iteration, e.g ',,Hh'", std::string(""));
  addParam("externalIterations", m_gblExternalIterations, "GBL: Number of times the GBL trajectory should be fitted and updated with results", int(1));
  addParam("pValueCut", m_gblPvalueCut, "GBL: p-value cut to output track to millepede file", 0.0);
  addParam("minNdf", m_gblMinNdf, "GBL: minimum NDF to output track to millepede file", 1);
  addParam("milleFileName", m_gblMilleFileName, "GBL: Name of the mille binary file to be produced for alignment", std::string("millefile.dat"));
  addParam("chi2Cut", m_chi2Cut, "GBL: Cut on single measurement Chi2", double(50.));
  addParam("enableScatterers", m_enableScatterers, "GBL: Enable scattering in GBL trajectory", bool(true));
  addParam("enableIntermediateScatterer", m_enableIntermediateScatterer, "GBL: Enable intermediate scatterers for simulation of thick scatterer", bool(true));
  addParam("resortHits", m_resortHits, "GBL: Sort hits by extrapolation before fit. Turn on if you see a lot of 'Extrapolation stepped back by ...' errors. The candidates should be already sorted!", bool(true));
  addParam("recalcJacobians", m_recalcJacobians, "GBL: Recalculate Jacobians/planes: 0=do not recalc, 1=after 1st fit, 2=1 & after 2nd fit, etc. Use '1' for 1 iteration + output to mille or if iteration>=2 ", int(0));

  addParam("useOldGbl", m_useOldGbl, "GBL: Use old GBL interface ", bool(false));


  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;

  m_failedGFTrackCandFitCounter = 0;
  m_successfulGFTrackCandFitCounter = 0;
}

GBLfitModule::~GBLfitModule()
{
}

void GBLfitModule::initialize()
{
  /*
  StoreArray<genfit::TrackCand>::required(m_gfTrackCandsColName);

  StoreArray<Track>::registerPersistent();
  StoreArray<TrackFitResult>::registerPersistent();
  //FIXME name
  StoreArray < genfit::Track >::registerPersistent("");
  StoreArray < genfit::TrackCand >::registerPersistent();

  if (!m_tracksColName.empty() and m_tracksColName != "Tracks") {
    B2ERROR("Setting a collection name with TracksColName is not implemented.");
    //TODO: implementation might also need different name for TrackFitResults?
  }

  StoreArray::registerRelationTo<StoreArray<genfit::Track> >(MCParticles);
  RelationArray::registerPersistent<MCParticle, Track> ();
  RelationArray::registerPersistent<genfit::Track, TrackFitResult>(m_gfTracksColName, "");
  RelationArray::registerPersistent<genfit::TrackCand, TrackFitResult>(m_gfTrackCandsColName, "");
  RelationArray::registerPersistent<genfit::TrackCand, genfit::Track>(m_gfTrackCandsColName, m_gfTracksColName);
  */

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

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2WARNING("Material effects not set up, doing this myself with default values.");

    if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
    }
    //pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();

    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

    // activate / deactivate material effects in genfit
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(true);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(true);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(true);

    genfit::MaterialEffects::getInstance()->setMscModel("Highland");
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
  CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(), new IdealCDCGeometryTranslator(), new SimpleTDCCountTranslator());

  // Set GBL parameters
  m_gbl.setOptions(m_gblInternalIterations, m_enableScatterers, m_enableIntermediateScatterer, m_gblExternalIterations, m_recalcJacobians);
  m_oldGbl.setGBLOptions(m_gblInternalIterations, m_enableScatterers, m_enableIntermediateScatterer);
  m_oldGbl.setMP2Options(m_gblPvalueCut, m_gblMinNdf, m_gblMilleFileName, m_chi2Cut);

  if (!m_useOldGbl)
    m_milleFile = new gbl::MilleBinary(m_gblMilleFileName);

}

void GBLfitModule::beginRun()
{
  m_oldGbl.beginRun();
}

void GBLfitModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "**********   GBLfitModule processing event number: " << eventCounter << " ************");

  StoreArray < MCParticle > mcParticles(m_mcParticlesColName);
  B2DEBUG(149, "GBLfit: total Number of MCParticles: " << mcParticles.getEntries());
  if (mcParticles.getEntries() == 0) { B2DEBUG(100, "GBLfit: MCParticlesCollection is empty!"); }

  StoreArray < genfit::TrackCand > trackCandidates(m_gfTrackCandsColName);
  B2DEBUG(99, "GBLfit: Number of genfit::TrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2DEBUG(100, "GBLfit: genfit::TrackCandidatesCollection is empty!");

  StoreArray < CDCHit > cdcHits(m_cdcHitsColName);
  B2DEBUG(149, "GBLfit: Number of CDCHits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2DEBUG(100, "GBLfit: CDCHitsCollection is empty!");

  StoreArray < SVDTrueHit > svdTrueHits(m_svdHitsColName);
  B2DEBUG(149, "GBLfit: Number of SVDHits: " << svdTrueHits.getEntries());
  if (svdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GBLfit: SVDHitsCollection is empty!");

  StoreArray < PXDTrueHit > pxdTrueHits(m_pxdHitsColName);
  B2DEBUG(149, "GBLfit: Number of PXDHits: " << pxdTrueHits.getEntries());
  if (pxdTrueHits.getEntries() == 0)
    B2DEBUG(100, "GBLfit: PXDHitsCollection is empty!");

#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
  StoreArray < TelTrueHit > telTrueHits(m_telHitsColName);
  B2DEBUG(149, "GBLfit: Number of TelHits: " << telTrueHits.getEntries());
  if (telTrueHits.getEntries() == 0)
    B2DEBUG(100, "GBLfit: PXDHitsCollection is empty!");
#endif

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters("");
  int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(149, "GBLfit: Number of PXDClusters: " << nPXDClusters);
  if (nPXDClusters == 0) {B2DEBUG(100, "GBLfit: PXDClustersCollection is empty!");}

#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
  //Telescope clusters
  StoreArray<TelCluster> telClusters("");
  int nTelClusters = telClusters.getEntries();
  B2DEBUG(149, "GBLfit: Number of TelClusters: " << nTelClusters);
  if (nTelClusters == 0) {B2DEBUG(100, "GBLfit: TelClustersCollection is empty!");}
#endif

  //SVD clusters
  StoreArray<SVDCluster> svdClusters("");
  int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(149, "GBLfit: Number of SVDClusters: " << nSVDClusters);
  if (nSVDClusters == 0) {B2DEBUG(100, "GBLfit: SVDClustersCollection is empty!");}

  //StoreArrays to store the fit results
  StoreArray < Track > tracks;
  tracks.create();
  StoreArray <TrackFitResult> trackFitResults;
  trackFitResults.create();
  StoreArray < genfit::Track > gfTracks(m_gfTracksColName);
  gfTracks.create();

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
      m_pdgCodes.clear(); //clear the pdg code from the last track
      m_pdgCodes.push_back(aTrackCandPointer->getPdgCode());
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
      B2DEBUG(99, "GBLfit: current PDG code: " << m_pdgCodes[iPdg]);
      int currentPdgCode = boost::math::sign(aTrackCandPointer->getChargeSeed()) * m_pdgCodes[iPdg];
      if (currentPdgCode == 0) {
        B2FATAL("Either the charge of the current genfit::TrackCand is 0 or you set 0 as a PDG code");
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


      //get fit starting values from the from the track candidate
      const TVector3& posSeed = aTrackCandPointer->getPosSeed();
      const TVector3& momentumSeed = aTrackCandPointer->getMomSeed();

      B2DEBUG(99, "Fit track with start values: ");

      B2DEBUG(100, "Start values: momentum (x,y,z,abs): " << momentumSeed.x() << "  " << momentumSeed.y() << "  " << momentumSeed.z() << " " << momentumSeed.Mag());
      //B2DEBUG(100, "Start values: momentum std: " << sqrt(covSeed(3, 3)) << "  " << sqrt(covSeed(4, 4)) << "  " << sqrt(covSeed(5, 5)));
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      //B2DEBUG(100, "Start values: pos std:   " << sqrt(covSeed(0, 0)) << "  " << sqrt(covSeed(1, 1)) << "  " << sqrt(covSeed(2, 2)));
      B2DEBUG(100, "Start values: pdg:      " << currentPdgCode);

      //initialize track representation and give the seed helix parameters and cov and the pdg code to the track fitter
      // Do this in two steps, because for now we use the genfit::TrackCand from Genfit 1.
      genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(currentPdgCode);


      genfit::MeasurementFactory<genfit::AbsMeasurement> factory;

      //create MeasurementProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_useClusters == false) { // use the trueHits
        if (pxdTrueHits.getEntries()) {
          genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit>* PXDProducer =  NULL;
          PXDProducer =  new genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
        if (telTrueHits.getEntries()) {
          genfit::MeasurementProducer <TelTrueHit, TelRecoHit>* TelProducer =  NULL;
          TelProducer =  new genfit::MeasurementProducer <TelTrueHit, TelRecoHit> (telTrueHits.getPtr());
          factory.addProducer(Const::TEST, TelProducer);
        }
#endif
        if (svdTrueHits.getEntries()) {
          genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D>* SVDProducer =  NULL;
          SVDProducer =  new genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }
      } else {
        if (nPXDClusters) {
          genfit::MeasurementProducer <PXDCluster, PXDRecoHit>* pxdClusterProducer = NULL;
          pxdClusterProducer =  new genfit::MeasurementProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
        if (nTelClusters) {
          genfit::MeasurementProducer <TelCluster, TelRecoHit>* telClusterProducer = NULL;
          telClusterProducer =  new genfit::MeasurementProducer <TelCluster, TelRecoHit> (telClusters.getPtr());
          factory.addProducer(Const::TEST, telClusterProducer);
        }
#endif
        if (nSVDClusters) {
          genfit::MeasurementProducer <SVDCluster, SVDRecoHit>* svdClusterProducer = NULL;
          svdClusterProducer =  new genfit::MeasurementProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      }
      if (cdcHits.getEntries()) {
        genfit::MeasurementProducer <CDCHit, CDCRecoHit>* CDCProducer =  NULL;
        CDCProducer =  new genfit::MeasurementProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
        factory.addProducer(Const::CDC, CDCProducer);
      }

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
      //aTrackCandPointer->Print();

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
        } else if (detId == Const::TEST) {
          nTel++;
        } else if (detId == Const::SVD) {
          nSVD++;
        } else if (detId == Const::CDC) {
          nCDC++;
        } else {
          B2WARNING("Hit from unknown detectorID has contributed to this track! The unknown id is: " << detId);
        }
      }

      B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ", Tel: " << nTel << ")");

      if (aTrackCandPointer->getNHits() < 3) { // this should not be nessesary because track finder should only produce track candidates with enough hits to calculate a momentum
        B2WARNING("GBLfit: only " << aTrackCandPointer->getNHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
        continue;
      }

      // SVD cluster combination ------------------------------------------------------------------------------------------------
      if (m_useClusters) {
        try {
          for (unsigned int i = 0; i < gfTrack.getNumPoints() - 1; ++i) {
            //if (gfTrack.getPointWithMeasurement(i)->getNumRawMeasurements() != 1)
            //  continue;
            genfit::PlanarMeasurement* planarMeas1 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(i)->getRawMeasurement(0));
            genfit::PlanarMeasurement* planarMeas2 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(i + 1)->getRawMeasurement(0));

            if (planarMeas1 != NULL && planarMeas2 != NULL &&
                planarMeas1->getDetId() == planarMeas2->getDetId() &&
                planarMeas1->getPlaneId() != -1 &&   // -1 is default plane id
                planarMeas1->getPlaneId() == planarMeas2->getPlaneId()) {
              Belle2::SVDRecoHit* hit1 = dynamic_cast<Belle2::SVDRecoHit*>(planarMeas1);
              Belle2::SVDRecoHit* hit2 = dynamic_cast<Belle2::SVDRecoHit*>(planarMeas2);
              if (hit1 && hit2) {
                Belle2::SVDRecoHit* hitU(NULL);
                Belle2::SVDRecoHit* hitV(NULL);
                // We have to decide U/V now (else SVDRecoHit2D could throw FATAL)
                if (hit1->isU() && !hit2->isU()) {
                  hitU = hit1;
                  hitV = hit2;
                } else if (!hit1->isU() && hit2->isU()) {
                  hitU = hit2;
                  hitV = hit1;
                } else {
                  continue;
                }
                Belle2::SVDRecoHit2D* hit = new Belle2::SVDRecoHit2D(*hitU, *hitV);
                // insert measurement before point i (increases number of currect point to i+1)
                gfTrack.insertMeasurement(hit, i);
                // now delete current point (at its original place, we have the new 2D recohit)
                gfTrack.deletePoint(i + 1);
                gfTrack.deletePoint(i + 1);
              }
            }
          }
        } catch (...) {
          B2FATAL("Hit combination of SVD clusters in genfit::Track failed.");
        }
      }
      // end of SVD cluster combination -------------------------------------------------------------------------------------

      /*
      //now fit the track
      try {
        // Prepare the the track (set ReferenceStates)
        //genfit::KalmanFitterRefTrack* refKalman = new genfit::KalmanFitterRefTrack();
        //int failedHits = 0;
        //refKalman->prepareTrack(&gfTrack, trackRep, true, failedHits);
        //if (failedHits > 0 && (nHitsInTrack - failedHits) < 3)
        //  continue;
        // Let's try to pass the track to GBL
        // Reference state (and planes) are used by GBL.

        m_gbl.processTrack(&gfTrack, m_resortHits);

        genfit::GblFitStatus* gblfs = 0;
        if (gfTrack.hasFitStatus(trackRep) && (gblfs = dynamic_cast<genfit::GblFitStatus*>(gfTrack.getFitStatus(trackRep)))) {
          //Fill histos
          hist->Fill(gblfs->getPVal());
          chi2->Fill(gblfs->getChi2());
          chi2ondf->Fill(gblfs->getChi2()/gblfs->getNdf());

          //If cuts are fullfilled, create GBL trajectory and out to Mille binary
          if (gblfs->getPVal() >= m_gblPvalueCut && gblfs->getNdf() >= m_gblMinNdf) {
            gbl::GblTrajectory traj(m_gbl.collectGblPoints(&gfTrack, trackRep), gblfs->hasCurvature());
            traj.milleOut(*m_milleFile);

          }

          gfTracks.appendNew(gfTrack);  //genfit::Track can be assigned directly

        }


      } catch (genfit::Exception e) {
        std::string error( e.getExcString() );
        B2WARNING("Exception during GBL fit / returning GBL fit status / storing of genfit::Track: " << error);
        e.info();
        ++m_failedFitCounter;
      } catch (std::exception e) {
        B2WARNING("Something went wrong during the fit: " << e.what());
        ++m_failedFitCounter;
      } catch (...) {
        B2WARNING("Something really unexpected happended during the fit");
        ++m_failedFitCounter;
      }

      */

      //now fit the track
      try {

        if (m_useOldGbl) {
          try {
            // Prepare the the track (set ReferenceStates)
            genfit::KalmanFitterRefTrack* refKalman = new genfit::KalmanFitterRefTrack();
            int failedHits = 0;
            refKalman->prepareTrack(&gfTrack, trackRep, true, failedHits);
            if (failedHits > 0 && (nHitsInTrack - failedHits) < 3)
              continue;
            // Let's try to pass the track to GBL
            // Reference state (and planes) are used by GBL.
            m_oldGbl.processTrack(&gfTrack);

          } catch (...) {
            B2WARNING("Something went wrong during the fit!");
            ++m_failedFitCounter;
          }
          // Nothing more can be done here with old GBL interface
          return;

        } else {
          // New GBL interface
          m_gbl.processTrack(&gfTrack, m_resortHits);
        }

        //gfTrack.Print();
        bool fitSuccess = gfTrack.hasFitStatus(trackRep);
        genfit::GblFitStatus* gfs = 0;
        if (fitSuccess) {
          genfit::FitStatus* fs = 0;
          fs = gfTrack.getFitStatus(trackRep);
          fitSuccess = fitSuccess && fs->isFitted();
          //fitSuccess = fitSuccess && fs->isFitConverged();
          fitSuccess = fitSuccess && fs->isFitConverged();

          gfs = dynamic_cast<genfit::GblFitStatus*>(fs);
          //hNit->Fill(kfs->getNumIterations());
          //if (!fitSuccess && kfs)
          //kfs->Print();
          fitSuccess = fitSuccess && gfs;
        }
        B2DEBUG(99, "-----> Fit results:");
        B2DEBUG(99, "       Fitted and converged: " << fitSuccess);

        //hSuccess->Fill(fitSuccess);

        if (fitSuccess) {
          B2DEBUG(99, "       Chi2 of the fit: " << gfs->getChi2());
          //B2DEBUG(99,"       Forward Chi2: "<<gfTrack.getForwardChi2());
          B2DEBUG(99, "       NDF of the fit: " << gfs->getNdf());
          //Calculate probability
          double pValue = gfTrack.getFitStatus()->getPVal();
          B2DEBUG(99, "       pValue of the fit: " << pValue);


          //hist->Fill(gfs->getPVal());
          //chi2->Fill(gfs->getChi2());
          //chi2ondf->Fill(gfs->getChi2()/gfs->getNdf());

          //If cuts are fullfilled, create GBL trajectory and out to Mille binary
          if (m_milleFile && gfs->getPVal() >= m_gblPvalueCut && gfs->getNdf() >= m_gblMinNdf) {
            bool overChi2Cut = false;
            for (auto tp : gfTrack.getPoints()) {
              if (m_chi2Cut <= 0.)
                break;
              genfit::MeasuredStateOnPlane residual = tp->getFitterInfo()->getResidual();
              for (int idim = 0; idim < residual.getState().GetNoElements(); idim++) {
                double pull = residual.getState()(idim); //residual
                pull = pull * pull; // residual squared
                pull = pull / residual.getCov()(idim, idim); // residual squared over error squared
                if (pull > m_chi2Cut) {
                  overChi2Cut = true;
                  break;
                }
              }
            }
            if (!overChi2Cut) {
              gbl::GblTrajectory traj(m_gbl.collectGblPoints(&gfTrack, trackRep), gfs->hasCurvature());
              traj.milleOut(*m_milleFile);
            }
          }


        }// end if(fitSuccess)

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
             *                                 tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
             *                                 tracks[trackCounter]->setNHits(gfTrack.getNumHits());
             *                                 tracks[trackCounter]->setNCDCHits(nCDC);
             *                                 tracks[trackCounter]->setNSVDHits(nSVD);
             *                                 tracks[trackCounter]->setNPXDHits(nPXD);
             *                                 tracks[trackCounter]->setMCId(aTrackCandPointer->getMcTrackId());
             *                                 tracks[trackCounter]->setPDG(aTrackCandPointer->getPdgCode());
             *                                 //tracks[trackCounter]->setPurity(aTrackCandPointer->getDip()); //setDip will be deleted soon. If purity is used it has to be passed differently to the Track class
             *                                 tracks[trackCounter]->setPValue(pValue);
             *                                 //Set helix parameters
             *                                 tracks[trackCounter]->setD0(-999);
             *                                 tracks[trackCounter]->setPhi(-999);
             *                                 tracks[trackCounter]->setOmega(gfTrack.getCharge());
             *                                 tracks[trackCounter]->setZ0(-999);
             *                                 tracks[trackCounter]->setCotTheta(-999);
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
          //TODO: This caused incosistent relations! Inspect!
          //if (gfs->getPVal() > m_gblPvalueCut)
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
           *               tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
           *               tracks[trackCounter]->setNHits(gfTrack.getNumHits());
           *               tracks[trackCounter]->setNCDCHits(nCDC);
           *               tracks[trackCounter]->setNSVDHits(nSVD);
           *               tracks[trackCounter]->setNPXDHits(nPXD);
           *               tracks[trackCounter]->setMCId(aTrackCandPointer->getMcTrackId());
           *               tracks[trackCounter]->setPDG(aTrackCandPointer->getPdgCode());
           *               //tracks[trackCounter]->setPurity(aTrackCandPointer->getDip()); //setDip will be deleted soon. If purity is used it has to be passed differently to the Track class
           *               tracks[trackCounter]->setPValue(pValue);
           *               tracks[trackCounter]->setExtrapFailed(false);
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
            // +/- predictions of GBL are the same at 1st point, so no matter if we take forward state for
            // most likely backward propagation (to interaction point)
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

            genfit::FitStatus* fs = gfTrack.getFitStatus();
            int charge = fs->getCharge();
            double pVal = fs->getPVal();
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
             *                   tracks[trackCounter]->setPhi(phi);
             *                   tracks[trackCounter]->setOmega((gfTrack.getCharge() / (pt * alpha)));
             *                   tracks[trackCounter]->setZ0(poca.z());
             *                   tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));
             */
            //Print helix parameters
            /*              B2DEBUG(99, ">>>>>>> Helix Parameters <<<<<<<");
             *                   B2DEBUG(99, "D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
             *                   B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
             *                   //Additional check
             *                   B2DEBUG(99, "Recalculate momentum from perigee: px: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha)) * (cos(tracks[trackCounter]->getPhi())) << "  py: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*sin(tracks[trackCounter]->getPhi()) << "  pz: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*tracks[trackCounter]->getCotTheta());
             *                   B2DEBUG(99, "<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
             */
            //if (m_createTextFile) {
            //Additional code
            //print helix parameter to a file
            //useful if one like to quickly plot track trajectories
            //-------------------------------------
            /*                HelixParam << tracks[trackCounter]->getD0() << " \t"
             *                                  << tracks[trackCounter]->getPhi() << " \t"
             *                                  << tracks[trackCounter]->getOmega() << " \t"
             *                                  << tracks[trackCounter]->getZ0() << " \t"
             *                                  << tracks[trackCounter]->getCotTheta() << "\t" << poca.x()
             *                                  << "\t" << poca.y() << "\t" << poca.z() << endl;
             *                       //----------------------------------------
             */              //end additional code
            //}
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

      } catch (genfit::Exception& e) {
        std::string error(e.getExcString());
        B2WARNING("genfit::Exception during the fit: " << error);
        e.info();
        ++m_failedFitCounter;

      } catch (std::exception& e) {
        B2WARNING("std::exception during the fit: " << e.what());
        ++m_failedFitCounter;

      } catch (...) {
        // Final catch
        B2WARNING("Something went wrong during the fit.");
        ++m_failedFitCounter;
      }

    } //end loop over all pdg hypothesis


    if (candFitted == true) m_successfulGFTrackCandFitCounter++;
    else m_failedGFTrackCandFitCounter++;

  }//end loop over all track candidates
  B2DEBUG(99, "GBLfit event summary: " << trackCounter + 1 << " tracks were processed");


}

void GBLfitModule::endRun()
{
  if (m_milleFile)
    delete m_milleFile;

}

void GBLfitModule::terminate()
{
  m_oldGbl.endRun();
  //delete m_milleFile;
}

