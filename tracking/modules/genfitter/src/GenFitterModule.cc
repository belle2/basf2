/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *               & Tobias Schlüter                                        *
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

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/WireID.h>

#include <framework/dataobjects/Helix.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
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

REG_MODULE(GenFitter)

GenFitterModule::GenFitterModule() :
  Module()
{

  setDescription(
    "Uses GenFit2 to fit tracks. Needs genfit::TrackCands as input and provides genfit::Tracks and Tracks as output.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (should be "
           "created by the pattern recognition or MCTrackFinderModule).",
           string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(""));

  addParam("MCParticlesColName", m_mcParticlesColName,
           "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)",
           string(""));
  //select the filter and set some parameters
  addParam("FilterId", m_filterId,
           "Set to 'Kalman' use Kalman Filter, 'DAF' to use the DAF and 'simpleKalman' for the Kalman without reference track",
           string("DAF"));
  addParam("NMinIterations", m_nMinIter, "Minimum number of iterations for the Kalman filter", int(3));
  addParam("NMaxIterations", m_nMaxIter, "Maximum number of iterations for the Kalman filter", int(10));
  addParam("NMaxFailedHits", m_nMaxFailed, "Maximum number of of failed hits before aborting the fit", int(5));
  addParam("ProbCut", m_probCut,
           "Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001",
           double(0.001));
  addParam("PruneFlags", m_pruneFlags,
           "Determine which information to keep after track fit, by default we "
           "keep everything, but please note that add_reconstruction prunes "
           "track after all other reconstruction is processed.  See "
           "genfit::Track::prune for options.",
           std::string(""));
  addParam("StoreFailedTracks", m_storeFailed,
           "Set true if the tracks where the fit failed should also be stored in the output",
           bool(false));
  addParam("UseClusters", m_useClusters,
           "If set to true cluster hits (PXD/SVD clusters) will be used for fitting. If false Gaussian smeared trueHits will be used",
           true);
  addParam("RealisticCDCGeoTranslator", m_realisticCDCGeoTranslator,
           "If true, realistic CDC geometry translators will be used (wire sag, misalignment).",
           true);
  addParam("CDCWireSag", m_enableWireSag,
           "Whether to enable wire sag in the CDC geometry translation.  Needs to agree with simulation/digitization.",
           true);
  addParam("UseTrackTime", m_useTrackTime,
           "Determines whether the realistic TDC track time converter and the "
           "CDCRecoHits will take the track propagation time into account.  "
           "The setting has to agree with those of the CDCDigitizer.  Requires "
           "EstimateSeedTime with current input (2015-03-11).",
           true);
  addParam("EstimateSeedTime", m_estimateSeedTime,
           "If set, time for the seed will be recalculated based on a helix "
           "approximation.  Only makes a difference if UseTrackTime is set.",
           true);
  addParam("PDGCodes", m_pdgCodes,
           "List of PDG codes used to set the mass hypothesis for the fit.  "
           "All your codes will be tried with every track. The sign of your "
           "codes will be ignored and the charge will always come from the "
           "genfit::TrackCand. If you do not set any PDG code the code will "
           "be taken from the genfit::TrackCand.  This is the default behavior.)",
           vector<int>(0));

  // Output
  addParam("GFTracksColName", m_gfTracksColName,
           "Name of collection holding the final genfit::Tracks (will be created by this module)",
           string(""));

  addParam("DAFTemperatures", m_dafTemperatures,
           "Set the annealing scheme (temperatures) for the DAF.  Length of vector will determine DAF iterations.",
           vector<double>(1, -999.0));
  addParam("resolveWireHitAmbi", m_resolveWireHitAmbi,
           "Determines how the ambiguity in wire hits is to be dealt with.  "
           "This only makes sense for the Kalman fitters.  Values are either "
           "'default' (use the default for the respective fitter algorithm), "
           "'weightedAverage', 'unweightedClosestToReference' (default for "
           "the Kalman filter), or 'unweightedClosestToPrediction' (default "
           "for the Kalman filter without reference track).",
           string("default"));

  addParam("suppressGFExceptionOutput", m_suppressGFExceptionOutput, "Suppress error messages in GenFit.", true);

  // keep GFExceptions quiet or not
  genfit::Exception::quiet(m_suppressGFExceptionOutput);

  // Initial values for counters.
  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;
  m_failedGFTrackCandFitCounter = 0;
  m_successfulGFTrackCandFitCounter = 0;

  // Silence cppcheck, set in initialize()
  m_usePdgCodeFromTrackCand = false;
}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{
  B2WARNING("This module is depricated as it uses genfit::Track(Cand)s instead of RecoTracks. It will be removed in the future. If you need information on the transition, please contact Nils Braun (nils.braun@kit.edu).");
  StoreArray<genfit::TrackCand> trackCandidates(m_gfTrackCandsColName);
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  trackCandidates.isRequired();
  mcParticles.isOptional();

  StoreArray<genfit::Track> gftracks(m_gfTracksColName);
  gftracks.registerInDataStore();

  gftracks.registerRelationTo(mcParticles);
  trackCandidates.registerRelationTo(gftracks);

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
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
  if (m_realisticCDCGeoTranslator) {
    CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(),
                               new RealisticCDCGeometryTranslator(m_enableWireSag),
                               new RealisticTDCCountTranslator(m_useTrackTime),
                               m_useTrackTime);
  } else {
    if (m_enableWireSag)
      B2WARNING("Wire sag requested, but using idealized translator which ignores this.");
    CDCRecoHit::setTranslators(new LinearGlobalADCCountTranslator(),
                               new IdealCDCGeometryTranslator(),
                               new RealisticTDCCountTranslator(m_useTrackTime),
                               m_useTrackTime);
  }
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

  StoreArray < genfit::TrackCand > trackCandidates(m_gfTrackCandsColName);
  B2DEBUG(99, "GenFitter: Number of genfit::TrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2DEBUG(100, "GenFitter: genfit::TrackCandidatesCollection is empty!");

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

  if (m_filterId == "DAF") {
    B2DEBUG(99, "DAF with probability cut " << m_probCut << " will be used ");
  } else {
    B2DEBUG(99, "Kalman filter with " << m_nMinIter << " to " << m_nMaxIter << " iterations will be used ");
  }


  StoreArray < genfit::Track > gfTracks(m_gfTracksColName);
  gfTracks.create();

  RelationArray gfTrackCandidatesTogfTracks(trackCandidates, gfTracks);

  //Create a relation between the gftracks and their most probable 'mother' MC particle
  RelationArray gfTracksToMCPart(gfTracks, mcParticles);

  //counter for fitted tracks
  int trackCounter = -1;


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
      // Make sure the track fit starts with the correct PDG code
      // because the sign of the PDG code will also set the charge in
      // the TrackRep.
      TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdgCodes[iPdg]);
      B2DEBUG(99, "GenFitter: current PDG code: " << m_pdgCodes[iPdg]);
      int currentPdgCode = m_pdgCodes[iPdg];
      // Note that for leptons positive PDG codes correspond to the
      // negatively charged particles.
      if (std::signbit(part->Charge()) != std::signbit(aTrackCandPointer->getChargeSeed()))
        currentPdgCode *= -1;
      // Charges in the PDG tables are counted in multiples of 1/3e.
      if (TDatabasePDG::Instance()->GetParticle(currentPdgCode)->Charge()
          != aTrackCandPointer->getChargeSeed() * 3)
        B2FATAL("Charge of candidate and PDG particle don't match.  (Code assumes |q| = 1).");
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

      // We reset the track's time after constructing it from the TrackCand.
      if (m_estimateSeedTime) {
        // Particle velocity in cm / ns.
        const double m = part->Mass();
        const double p = momentumSeed.Mag();
        const double E = hypot(m, p);
        const double beta = p / E;
        const double v = beta * Const::speedOfLight;

        // Arc length from IP to posSeed in cm.
        // Calculate the arc-length.  Helix doesn't provide a way of
        // obtaining this directly from the difference in z, as it
        // only provide arc-lengths in the transverse plane, so we do
        // it like this.
        // const Helix h(posSeed, momentumSeed, part->Charge() / 3, 1.5);
        const Helix h(posSeed, momentumSeed, part->Charge() / 3, 0);
        const double s2D = h.getArcLength2DAtXY(posSeed.X(), posSeed.Y());
        const double s = s2D * hypot(1, h.getTanLambda());

        // Time (ns) from trigger (= 0 ns) to posSeed assuming constant velocity.
        double timeSeed = s / v;

        if (not(fabs(timeSeed) < 1000)) {
          // Guard against NaN or just something silly.
          B2WARNING("Fixing calculated seed Time " << timeSeed << " to zero.");
          timeSeed = 0;
        }
        // Update the TrackCand in case we estimated the time.
        aTrackCandPointer->setTimeSeed(timeSeed);

      }
      double timeSeed = aTrackCandPointer->getTimeSeed();

      B2DEBUG(99, "Fit track with start values: ");

      B2DEBUG(100, "Start values: momentum (x,y,z,abs): "
              << momentumSeed.x() << "  " << momentumSeed.y() << "  "
              << momentumSeed.z() << " " << momentumSeed.Mag());
      B2DEBUG(100, "Start values: pos:   " << posSeed.x() << "  " << posSeed.y() << "  " << posSeed.z());
      B2DEBUG(100, "Start values: pdg:      " << currentPdgCode << " time: " << timeSeed);

      genfit::MeasurementFactory<genfit::AbsMeasurement> factory;

      //create MeasurementProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_useClusters) {
        // By default use clusters.
        if (nPXDClusters) {
          auto pxdClusterProducer =  new genfit::MeasurementProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
        if (nSVDClusters) {
          auto svdClusterProducer =  new genfit::MeasurementProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      } else {
        // Use MC true hits instead.
        if (pxdTrueHits.getEntries()) {
          auto PXDProducer =  new genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
        if (svdTrueHits.getEntries()) {
          auto SVDProducer =  new genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }

      }
      if (cdcHits.getEntries()) {
        auto CDCProducer =  new genfit::MeasurementProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
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

      genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(currentPdgCode);
      genfit::Track gfTrack(*aTrackCandPointer, factory, trackRep); //create the track with the corresponding track representation
      // Reset the time seed to deal with the case where we have recalculated it.
      gfTrack.setTimeSeed(timeSeed);

      const int nHitsInTrack = gfTrack.getNumPointsWithMeasurement();
      B2DEBUG(99, "Total Nr of Hits assigned to the Track: " << nHitsInTrack);


      //Check which hits are contributing to the track
      int nCDC = 0;
      int nSVD = 0;
      int nPXD = 0;

      for (unsigned int hit = 0; hit < aTrackCandPointer->getNHits(); hit++) {
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

      if (aTrackCandPointer->getNHits() < 3) {
        // This should not be necessary because track finders should
        // only produce track candidates with enough hits to calculate
        // a momentum.
        B2WARNING("Genfit2Module: only " << aTrackCandPointer->getNHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
        continue;
      }

      // Select the fitter.  unique_ptr ensures that it's destructed at the right point.
      std::unique_ptr<genfit::AbsKalmanFitter> fitter(nullptr);
      if (m_filterId == "Kalman") {
        fitter.reset(new genfit::KalmanFitterRefTrack());
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
        fitter->setMinIterations(m_nMinIter);
        fitter->setMaxIterations(m_nMaxIter);
      } else if (m_filterId == "DAF") {
        // FIXME ... testing
        //fitter.reset(new genfit::DAF(false));
        fitter.reset(new genfit::DAF(true, m_dafDeltaPval));
        ((genfit::DAF*)fitter.get())->setProbCut(m_probCut);
      } else if (m_filterId == "simpleKalman") {
        fitter.reset(new genfit::KalmanFitter(4, 1e-3, 1e3, false));
        fitter->setMultipleMeasurementHandling(genfit::unweightedClosestToPredictionWire);
        fitter->setMinIterations(m_nMinIter);
        fitter->setMaxIterations(m_nMaxIter);
      } else {
        B2FATAL("Unknown filter id " << m_filterId << " requested.");
      }
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
          fitSuccess = fitSuccess && kfs;
        }
        B2DEBUG(99, "-----> Fit results:");
        B2DEBUG(99, "       Fitted and converged: " << fitSuccess);
        if (fitSuccess) {
          B2DEBUG(99, "       Chi2 of the fit: " << kfs->getChi2());
          //B2DEBUG(99,"       Forward Chi2: "<<gfTrack.getForwardChi2());
          B2DEBUG(99, "       NDF of the fit: " << kfs->getBackwardNdf());
          //Calculate probability
          double pValue = gfTrack.getFitStatus()->getPVal();
          B2DEBUG(99, "       pValue of the fit: " << pValue);
        }

        if (!fitSuccess) {    //if fit failed
          B2DEBUG(99, "Event " << eventCounter << ", genfit::TrackCand: " << iCand
                  << ", PDG hypo: " << currentPdgCode
                  << ": fit failed. GenFit returned an error (with fitSuccess "
                  << fitSuccess << ").");
          ++m_failedFitCounter;
          if (m_storeFailed == true) {
            ++trackCounter;

            //Create output tracks
            gfTracks.appendNew(gfTrack);  //genfit::Track can be assigned directly
            gfTrackCandidatesTogfTracks.add(iCand, trackCounter);

            //Create relation
            if (aTrackCandPointer->getMcTrackId() >= 0) {
              gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
            }
          }
        } else {            //fit successful
          ++m_successfulFitCounter;
          ++trackCounter;

          candFitted = true;

          //Create output tracks
          gfTrack.prune(m_pruneFlags.c_str());
          gfTracks.appendNew(gfTrack);  //genfit::Track can be assigned directly
          gfTrackCandidatesTogfTracks.add(iCand, trackCounter);

          //Create relation
          if (aTrackCandPointer->getMcTrackId() >= 0) {
            gfTracksToMCPart.add(trackCounter, aTrackCandPointer->getMcTrackId());
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

void GenFitterModule::endRun()
{
  B2DEBUG(5, "----- GenFitter '" << getName() << "' run summary");
  B2DEBUG(5, "      " << m_successfulGFTrackCandFitCounter << " track candidates were fitted successfully");
  B2DEBUG(5, "      in total " << m_successfulFitCounter << " tracks were fitted");
  if (m_failedFitCounter > 0) {
    B2RESULT("GenFitter '" << getName() << "': " << m_failedGFTrackCandFitCounter
             << " of " << m_successfulGFTrackCandFitCounter + m_failedGFTrackCandFitCounter
             << " track candidates could not be fitted in this run");
    B2DEBUG(5,
            "GenFitter '" << getName() << "': " << m_failedFitCounter
            << " of " << m_successfulFitCounter + m_failedFitCounter
            << " tracks could not be fitted in this run");
  }
}

void GenFitterModule::terminate()
{
}
