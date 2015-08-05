/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/PlanarMomentumMeasurement.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/FitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FieldManager.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/modules/fitter/BaseRecoFitterModule.h>

using namespace std;
using namespace Belle2;

namespace {
  template <class HitType>
  genfit::AbsMeasurement* createMeasurement(Const::EDetector detector,
                                            RecoHitInformation& recoHitInformation, HitType* const hit,
                                            const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory)
  {
    genfit::TrackCandHit* trackCandHit = new genfit::TrackCandHit(detector, hit->getArrayIndex(), -1,
        recoHitInformation.getSortingParameter());

    genfit::AbsMeasurement* coordinateMeasurement = measurementFactory.createOne(trackCandHit->getDetId(), trackCandHit->getHitId(),
                                                    trackCandHit);
    return coordinateMeasurement;
  }


  genfit::TrackPoint* createTrackPoint(genfit::AbsMeasurement* coordinateMeasurement, RecoTrack& recoTrack,
                                       const RecoHitInformation& recoHitInformation)
  {
    genfit::TrackPoint* coordinateTrackPoint = new genfit::TrackPoint(coordinateMeasurement, &recoTrack);
    coordinateTrackPoint->setSortingParameter(recoHitInformation.getSortingParameter());

    return coordinateTrackPoint;
  }


  template <class HitType>
  void addVXDMomentumEstimationToGenfitTrack(Const::EDetector detector, RecoTrack& recoTrack,
                                             RecoHitInformation& recoHitInformation, HitType* const hit,
                                             const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory)
  {
    genfit::AbsMeasurement* coordinateMeasurement = createMeasurement(detector, recoHitInformation, hit, measurementFactory);
    genfit::TrackPoint* coordinateTrackPoint = createTrackPoint(coordinateMeasurement, recoTrack, recoHitInformation);
    recoTrack.insertPoint(coordinateTrackPoint);

    genfit::PlanarMeasurement* planarMeasurement = dynamic_cast<genfit::PlanarMeasurement*>(coordinateMeasurement);
    if (planarMeasurement == nullptr) {
      B2FATAL("Can only add VXD hits which are based on PlanarMeasurements with momentum estimation!")
    }

    const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

    const TVector3& momentum = recoTrack.getMomentum();
    const TVector3& position = recoTrack.getPosition();
    short charge = recoTrack.getCharge();

    if (momentum.Mag() < 0.1) {
      TVectorD rawHitCoordinates(1);
      rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*hit, momentum, position, charge);

      TMatrixDSym rawHitCovariance(1);
      rawHitCovariance(0, 0) = 0.2;

      genfit::PlanarMomentumMeasurement* momentumMeasurement = new genfit::PlanarMomentumMeasurement(*planarMeasurement);
      momentumMeasurement->setRawHitCoords(rawHitCoordinates);
      momentumMeasurement->setRawHitCov(rawHitCovariance);

      genfit::TrackPoint* momentumTrackPoint = createTrackPoint(momentumMeasurement, recoTrack, recoHitInformation);
      recoTrack.insertPoint(momentumTrackPoint);
    }
  };

  template <class HitType>
  void addHitToGenfitTrack(Const::EDetector detector, RecoTrack& recoTrack,
                           RecoHitInformation& recoHitInformation, HitType* const hit,
                           const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory)
  {
    genfit::AbsMeasurement* coordinateMeasurement = createMeasurement(detector, recoHitInformation, hit, measurementFactory);
    genfit::TrackPoint* coordinateTrackPoint = createTrackPoint(coordinateMeasurement, recoTrack, recoHitInformation);
    recoTrack.insertPoint(coordinateTrackPoint);
  };
}

BaseRecoFitterModule::BaseRecoFitterModule() :
  Module()
{
  setDescription("Fit the given reco tracks with the given fitter parameters.");

  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           std::string("RecoTracks"));

  addParam("pdgCodeToUseForFitting", m_param_pdgCodeToUseForFitting,
           "Use this particle hypothesis for fitting. Please use the positive pdg code only.", static_cast<unsigned int>(211));

  addParam("useVXDMomentumEstimation", m_param_useVXDMomentumEstimation, "Use the momentum estimation from VXD.", false);
}

void BaseRecoFitterModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up. Please use SetupGenfitExtrapolationModule.");
  }

  // Create new Translators and give them to the CDCRecoHits.
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true);


  // Create a measurement factory
  StoreArray<RecoTrack::UsedPXDHit> pxdHits(m_param_storeArrayNameOfPXDHits);
  StoreArray<RecoTrack::UsedSVDHit> svdHits(m_param_storeArrayNameOfSVDHits);
  StoreArray<RecoTrack::UsedCDCHit> cdcHits(m_param_storeArrayNameOfCDCHits);

  // Create the related measurement factory
  if (pxdHits.isOptional())
    m_measurementFactory.addProducer(Const::PXD, new genfit::MeasurementProducer<RecoTrack::UsedPXDHit, PXDRecoHit>(pxdHits.getPtr()));
  if (svdHits.isOptional())
    m_measurementFactory.addProducer(Const::SVD, new genfit::MeasurementProducer<RecoTrack::UsedSVDHit, SVDRecoHit>(svdHits.getPtr()));
  if (cdcHits.isOptional())
    m_measurementFactory.addProducer(Const::CDC, new genfit::MeasurementProducer<RecoTrack::UsedCDCHit, CDCRecoHit>(cdcHits.getPtr()));
}

void BaseRecoFitterModule::constructHitsForTrack(RecoTrack& recoTrack) const
{
  // TODO: It may be better to do this already when adding the hits to the reco track.

  // create TrackPoints
  // Loop over all hits and create an abs measurement with the factory.
  // then create a TrackPoint from that and set the sorting parameter
  recoTrack.mapOnHits<RecoTrack::UsedCDCHit>(recoTrack.getStoreArrayNameOfCDCHits(), [&recoTrack,
  this](RecoHitInformation & recoHitInformation, RecoTrack::UsedCDCHit * hit) {
    addHitToGenfitTrack<RecoTrack::UsedCDCHit>(Const::CDC, recoTrack, recoHitInformation, hit, m_measurementFactory);
  });

  if (m_param_useVXDMomentumEstimation) {
    recoTrack.mapOnHits<RecoTrack::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(), [&recoTrack,
    this](RecoHitInformation & recoHitInformation, RecoTrack::UsedSVDHit * hit) {
      addVXDMomentumEstimationToGenfitTrack<RecoTrack::UsedSVDHit>(Const::SVD, recoTrack, recoHitInformation, hit, m_measurementFactory);
    });
    recoTrack.mapOnHits<RecoTrack::UsedPXDHit>(recoTrack.getStoreArrayNameOfPXDHits(), [&recoTrack,
    this](RecoHitInformation & recoHitInformation, RecoTrack::UsedPXDHit * hit) {
      addVXDMomentumEstimationToGenfitTrack<RecoTrack::UsedPXDHit>(Const::PXD, recoTrack, recoHitInformation, hit, m_measurementFactory);
    });
  } else {
    recoTrack.mapOnHits<RecoTrack::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(), [&recoTrack,
    this](RecoHitInformation & recoHitInformation, RecoTrack::UsedSVDHit * hit) {
      addHitToGenfitTrack<RecoTrack::UsedSVDHit>(Const::SVD, recoTrack, recoHitInformation, hit, m_measurementFactory);
    });
    recoTrack.mapOnHits<RecoTrack::UsedPXDHit>(recoTrack.getStoreArrayNameOfPXDHits(), [&recoTrack,
    this](RecoHitInformation & recoHitInformation, RecoTrack::UsedPXDHit * hit) {
      addHitToGenfitTrack<RecoTrack::UsedPXDHit>(Const::PXD, recoTrack, recoHitInformation, hit, m_measurementFactory);
    });

  }

  recoTrack.sort();
}

void BaseRecoFitterModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  B2DEBUG(100, "Number of reco track candidates to process: " << recoTracks.getEntries());
  unsigned int recoTrackCounter = 0;

  const std::shared_ptr<genfit::AbsKalmanFitter>& fitter = createFitter();
  fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);

  for (RecoTrack& recoTrack : recoTracks) {
    constructHitsForTrack(recoTrack);

    B2DEBUG(100, "Fitting reco track candidate number " << recoTrackCounter);
    B2DEBUG(100, "Reco track candidate has start values: ")
    B2DEBUG(100, "Momentum: " << recoTrack.getMomentum().X() << " " << recoTrack.getMomentum().Y() << " " <<
            recoTrack.getMomentum().Z())
    B2DEBUG(100, "Position: " << recoTrack.getPosition().X() << " " << recoTrack.getPosition().Y() << " " <<
            recoTrack.getPosition().Z())
    B2DEBUG(100, "Total number of hits assigned to the track: " << recoTrack.getNumberOfTotalHits())

    recoTrack.fit(fitter, m_param_pdgCodeToUseForFitting);

    B2DEBUG(100, "-----> Fit results:");
    B2DEBUG(100, "       Fitted and converged: " << recoTrack.wasLastFitSucessfull());

    if (recoTrack.wasLastFitSucessfull()) {
      genfit::FitStatus* fs = recoTrack.getFitStatus(recoTrack.getCardinalRep());
      genfit::KalmanFitStatus* kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);

      B2DEBUG(100, "       Chi2 of the fit: " << kfs->getChi2());
      B2DEBUG(100, "       NDF of the fit: " << kfs->getBackwardNdf());
      B2DEBUG(100, "       pValue of the fit: " << kfs->getPVal());
    }

    recoTrackCounter += 1;
  }
}
