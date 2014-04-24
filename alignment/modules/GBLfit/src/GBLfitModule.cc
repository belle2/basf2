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

//#define ALLOW_TELESCOPES_IN_GBLFITMODULE
//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//#include <testbeam/vxd/dataobjects/TelCluster.h>
//#include <testbeam/vxd/dataobjects/TelTrueHit.h>
//#include <testbeam/vxd/reconstruction/TelRecoHit.h>
//#endif

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


using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(GBLfit)

GBLfitModule::GBLfitModule() :
  Module()
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

  addParam("energyLossBetheBloch", m_energyLossBetheBloch, "activate the material effect: EnergyLossBetheBloch", false);
  addParam("noiseBetheBloch", m_noiseBetheBloch, "activate the material effect: NoiseBetheBloch", false);
  addParam("noiseCoulomb", m_noiseCoulomb, "activate the material effect: NoiseCoulomb", false);
  addParam("energyLossBrems", m_energyLossBrems, "activate the material effect: EnergyLossBrems", false);
  addParam("noiseBrems", m_noiseBrems, "activate the material effect: NoiseBrems", false);
  addParam("noEffects", m_noEffects, "switch off all material effects in Genfit. This overwrites all individual material effects switches", false);
  addParam("MSCModel", m_mscModel, "Multiple scattering model", string("Highland"));
  addParam("resolveWireHitAmbi", m_resolveWireHitAmbi, "Determines how the ambiguity in wire hits is to be dealt with.  This only makes sense for the Kalman fitters.  Values are either 'default' (use the default for the respective fitter algorithm), 'weightedAverage', 'unweightedClosestToReference' (default for the Kalman filter), or 'unweightedClosestToPrediction' (default for the Kalman filter without reference track).", string("default"));

  addParam("suppressGFExceptionOutput", m_suppressGFExceptionOutput, "Suppress error messages in GenFit.", true);

  // keep GFExceptions quiet or not
  genfit::Exception::quiet(m_suppressGFExceptionOutput);

  addParam("internalIterations", m_gblInternalIterations, "GBL internal downweighting setting", std::string(""));
  addParam("pValueCut", m_gblPvalueCut, "GBL p-value cut to output track to millepede file", 0.0);
  addParam("minNdf", m_gblMinNdf, "GBL minimum NDF to output track to millepede file", 1);
  addParam("milleFileName", m_gblMilleFileName, "GBL: Name of the mille binary file to be produced for alignments", std::string("millefile.dat"));
  addParam("chi2Cut", m_chi2Cut, "GBL: Cut on single measurement Chi2", double(50.));
  addParam("enableScatterers", m_enableScatterers, "GBL: Enable scattering in GBL trajectory", bool(true));
  addParam("enableIntermediateScatterer", m_enableIntermediateScatterer, "GBL: Enable intermediate scatterers for simulation of thick scatterer", bool(true));


}

GBLfitModule::~GBLfitModule()
{
}

void GBLfitModule::initialize()
{

  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;

  m_failedGFTrackCandFitCounter = 0;
  m_successfulGFTrackCandFitCounter = 0;

  StoreArray<genfit::TrackCand>::required(m_gfTrackCandsColName);

  //StoreArray<Track>::registerPersistent();
  //StoreArray<TrackFitResult>::registerPersistent();
  //StoreArray < genfit::Track >::registerPersistent(m_gfTracksColName);
  //StoreArray < genfit::TrackCand >::registerPersistent();

  if (!m_tracksColName.empty() and m_tracksColName != "Tracks") {
    B2ERROR("Setting a collection name with TracksColName is not implemented.");
    //TODO: implementation might also need different name for TrackFitResults?
  }

  //RelationArray::registerPersistent<genfit::Track, MCParticle>(m_gfTracksColName, m_mcParticlesColName);
  //RelationArray::registerPersistent<MCParticle, Track> ();
  //RelationArray::registerPersistent<genfit::Track, TrackFitResult>(m_gfTracksColName, "");
  //RelationArray::registerPersistent<genfit::TrackCand, TrackFitResult>(m_gfTrackCandsColName, "");
  //RelationArray::registerPersistent<genfit::TrackCand, genfit::Track>(m_gfTrackCandsColName, m_gfTracksColName);

  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
  }
  //pass the magnetic field to genfit
  if (!genfit::FieldManager::getInstance()->isInitialized())
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
  genfit::FieldManager::getInstance()->useCache();

  if (!genfit::MaterialEffects::getInstance()->isInitialized())
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
  // activate / deactivate material effects in genfit
  if (m_noEffects == true) {
    genfit::MaterialEffects::getInstance()->setNoEffects(true);
  } else {
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
  }
  genfit::MaterialEffects::getInstance()->setMscModel(m_mscModel);

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
  m_gbl.setGBLOptions(m_gblInternalIterations, m_enableScatterers, m_enableIntermediateScatterer);
  m_gbl.setMP2Options(m_gblPvalueCut, m_gblMinNdf, m_gblMilleFileName, m_chi2Cut);

}

void GBLfitModule::beginRun()
{
  m_gbl.beginRun();
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

//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//  StoreArray < TelTrueHit > telTrueHits(m_telHitsColName);
//  B2DEBUG(149, "GBLfit: Number of TelHits: " << telTrueHits.getEntries());
//  if (telTrueHits.getEntries() == 0)
//    B2DEBUG(100, "GBLfit: PXDHitsCollection is empty!");
//#endif

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters("");
  int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(149, "GBLfit: Number of PXDClusters: " << nPXDClusters);
  if (nPXDClusters == 0) {B2DEBUG(100, "GBLfit: PXDClustersCollection is empty!");}

//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//  //Telescope clusters
//  StoreArray<TelCluster> telClusters("");
//  int nTelClusters = telClusters.getEntries();
//  B2DEBUG(149, "GBLfit: Number of TelClusters: " << nTelClusters);
//  if (nTelClusters == 0) {B2DEBUG(100, "GBLfit: TelClustersCollection is empty!");}
//#endif

  //SVD clusters
  StoreArray<SVDCluster> svdClusters("");
  int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(149, "GBLfit: Number of SVDClusters: " << nSVDClusters);
  if (nSVDClusters == 0) {B2DEBUG(100, "GBLfit: SVDClustersCollection is empty!");}

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  //int trackCounter = -1;
  //int trackFitResultCounter = 0;

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
    // bool candFitted = false;   //boolean to mark if the track candidates was fitted successfully with at least one PDG hypothesis

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

      genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit>* PXDProducer =  NULL;
//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//      genfit::MeasurementProducer <TelTrueHit, TelRecoHit>* TelProducer =  NULL;
//#endif
      genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D>* SVDProducer =  NULL;
      //TODO: CDC
      // genfit::MeasurementProducer <CDCHit, CDCRecoHit>* CDCProducer =  NULL;

      genfit::MeasurementProducer <PXDCluster, PXDRecoHit>* pxdClusterProducer = NULL;
//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//      genfit::MeasurementProducer <TelCluster, TelRecoHit>* telClusterProducer = NULL;
//#endif
      genfit::MeasurementProducer <SVDCluster, SVDRecoHit>* svdClusterProducer = NULL;

      //create MeasurementProducers for PXD, SVD and CDC and add producers to the factory with correct detector Id
      if (m_useClusters == false) { // use the trueHits
        if (pxdTrueHits.getEntries()) {
          PXDProducer =  new genfit::MeasurementProducer <PXDTrueHit, PXDRecoHit> (pxdTrueHits.getPtr());
          factory.addProducer(Const::PXD, PXDProducer);
        }
//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//        if (telTrueHits.getEntries()) {
//          TelProducer =  new genfit::MeasurementProducer <TelTrueHit, TelRecoHit> (telTrueHits.getPtr());
//          factory.addProducer(Const::TEST, TelProducer);
//        }
//#endif
        if (svdTrueHits.getEntries()) {
          SVDProducer =  new genfit::MeasurementProducer <SVDTrueHit, SVDRecoHit2D> (svdTrueHits.getPtr());
          factory.addProducer(Const::SVD, SVDProducer);
        }
      } else {
        if (nPXDClusters) {
          pxdClusterProducer =  new genfit::MeasurementProducer <PXDCluster, PXDRecoHit> (pxdClusters.getPtr());
          factory.addProducer(Const::PXD, pxdClusterProducer);
        }
//#ifdef ALLOW_TELESCOPES_IN_GBLFITMODULE
//        if (nTelClusters) {
//          telClusterProducer =  new genfit::MeasurementProducer <TelCluster, TelRecoHit> (telClusters.getPtr());
//          factory.addProducer(Const::TEST, telClusterProducer);
//        }
//#endif
        if (nSVDClusters) {
          svdClusterProducer =  new genfit::MeasurementProducer <SVDCluster, SVDRecoHit> (svdClusters.getPtr());
          factory.addProducer(Const::SVD, svdClusterProducer);
        }
      }
      if (cdcHits.getEntries()) {
        B2WARNING("GBLfit: CDC hits not yet supported. Will be ignored during track construction.");
        //TODO: CDC
        //CDCProducer =  new genfit::MeasurementProducer <CDCHit, CDCRecoHit> (cdcHits.getPtr());
        //factory.addProducer(Const::CDC, CDCProducer);
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

      //B2DEBUG(99, "            (CDC: " << nCDC << ", SVD: " << nSVD << ", PXD: " << nPXD << ", Tel: " << nTel << ")");
      B2DEBUG(99, "            (SVD: " << nSVD << ", PXD: " << nPXD << ", Tel: " << nTel << ")");

      if (aTrackCandPointer->getNHits() < 3) { // this should not be nessesary because track finder should only produce track candidates with enough hits to calculate a momentum
        B2WARNING("GBLfit: only " << aTrackCandPointer->getNHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
        continue;
      }

      //now fit the track
      try {
        // Prepare the the track (set ReferenceStates)
        genfit::KalmanFitterRefTrack* refKalman = new genfit::KalmanFitterRefTrack();
        int failedHits = 0;
        refKalman->prepareTrack(&gfTrack, trackRep, true, failedHits);
        if (failedHits > 0 && (nHitsInTrack - failedHits) < 3)
          continue;
        // Let's try to pass the track to GBL
        // Reference state (and planes) are used by GBL.
        m_gbl.processTrack(&gfTrack);

      } catch (...) {
        B2WARNING("Something went wrong during the fit!");
        ++m_failedFitCounter;
      }

    } //end loop over all pdg hypothesis

  }//end loop over all track candidates

}

void GBLfitModule::endRun()
{
  // Needed to be able to store ROOT data from GBL. During destruction, it is too late.
  // Also the Millepede binary file must be closed before running pede (dtor is too late).
  m_gbl.endRun();
}

void GBLfitModule::terminate()
{

}

