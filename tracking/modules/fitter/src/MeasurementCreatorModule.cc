#include <tracking/modules/fitter/MeasurementCreatorModule.h>

#include <genfit/PlanarMomentumMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>


#include <framework/gearbox/Const.h>

using namespace Belle2;

namespace {
  /** Create a measurement from a hit */
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

  /** Create a TrackPOint from a measurement */
  genfit::TrackPoint* createTrackPoint(genfit::AbsMeasurement* coordinateMeasurement, RecoTrack& recoTrack,
                                       const RecoHitInformation& recoHitInformation)
  {
    genfit::TrackPoint* coordinateTrackPoint = new genfit::TrackPoint(coordinateMeasurement, &recoTrack);
    coordinateTrackPoint->setSortingParameter(recoHitInformation.getSortingParameter());

    return coordinateTrackPoint;
  }

  /** Create for VXD measurements another measurement with a momentum estimation */
  template <class HitType>
  genfit::AbsMeasurement* createVXDMomentumMeasurement(genfit::AbsMeasurement* coordinateMeasurement, HitType* const hit,
                                                       RecoTrack& recoTrack)
  {
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
      return momentumMeasurement;
    }

    return nullptr;
  }

}

MeasurementCreatorModule::MeasurementCreatorModule() : Module()
{
  setDescription("Create measurements from the hits added to the RecoTracks and add them to the genfit tracks. Can also create new measurements like momentum estimations.");
  addParam("useVXDMomentumEstimation", m_param_useVXDMomentumEstimation, "Use the momentum estimation from VXD.", false);
}

void MeasurementCreatorModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

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

void MeasurementCreatorModule::addCDCMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation,
                                                 RecoTrack::UsedCDCHit* const hit) const
{
  genfit::AbsMeasurement* coordinateMeasurement = createMeasurement(Const::CDC, recoHitInformation, hit, m_measurementFactory);
  genfit::TrackPoint* coordinateTrackPoint = createTrackPoint(coordinateMeasurement, recoTrack, recoHitInformation);
  recoTrack.insertPoint(coordinateTrackPoint);
}

void MeasurementCreatorModule::addSVDMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation,
                                                 RecoTrack::UsedSVDHit* const hit) const
{
  genfit::AbsMeasurement* coordinateMeasurement = createMeasurement(Const::CDC, recoHitInformation, hit, m_measurementFactory);
  genfit::TrackPoint* coordinateTrackPoint = createTrackPoint(coordinateMeasurement, recoTrack, recoHitInformation);
  recoTrack.insertPoint(coordinateTrackPoint);

  if (m_param_useVXDMomentumEstimation) {
    genfit::AbsMeasurement* momentumMeasurement = createVXDMomentumMeasurement(coordinateMeasurement, hit, recoTrack);
    if (momentumMeasurement != nullptr) {
      genfit::TrackPoint* momentumTrackPoint = createTrackPoint(momentumMeasurement, recoTrack, recoHitInformation);
      recoTrack.insertPoint(momentumTrackPoint);
    }
  }
}

void MeasurementCreatorModule::addPXDMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation,
                                                 RecoTrack::UsedPXDHit* const hit) const
{
  genfit::AbsMeasurement* coordinateMeasurement = createMeasurement(Const::CDC, recoHitInformation, hit, m_measurementFactory);
  genfit::TrackPoint* coordinateTrackPoint = createTrackPoint(coordinateMeasurement, recoTrack, recoHitInformation);
  recoTrack.insertPoint(coordinateTrackPoint);

  if (m_param_useVXDMomentumEstimation) {
    genfit::AbsMeasurement* momentumMeasurement = createVXDMomentumMeasurement(coordinateMeasurement, hit, recoTrack);
    if (momentumMeasurement != nullptr) {
      genfit::TrackPoint* momentumTrackPoint = createTrackPoint(momentumMeasurement, recoTrack, recoHitInformation);
      recoTrack.insertPoint(momentumTrackPoint);
    }
  }
}

void MeasurementCreatorModule::constructHitsForTrack(RecoTrack& recoTrack) const
{
  // create TrackPoints
  // Loop over all hits and create an abs measurement with the factory.
  // then create a TrackPoint from that and set the sorting parameter
  recoTrack.mapOnHits<RecoTrack::UsedCDCHit>(recoTrack.getStoreArrayNameOfCDCHits(),
                                             std::bind(&MeasurementCreatorModule::addCDCMeasurement, this, recoTrack, std::placeholders::_1, std::placeholders::_2));

  recoTrack.mapOnHits<RecoTrack::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(),
                                             std::bind(&MeasurementCreatorModule::addSVDMeasurement, this, recoTrack, std::placeholders::_1, std::placeholders::_2));

  recoTrack.mapOnHits<RecoTrack::UsedPXDHit>(recoTrack.getStoreArrayNameOfSVDHits(),
                                             std::bind(&MeasurementCreatorModule::addPXDMeasurement, this, recoTrack, std::placeholders::_1, std::placeholders::_2));

  recoTrack.sort();
}
