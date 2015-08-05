#include <tracking/modules/fitter/MeasurementCreatorModule.h>

#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>


#include <tracking/measurementCreator/creators/CoordinateMeasurementCreator.h>

#include <framework/gearbox/Const.h>

using namespace Belle2;

REG_MODULE(MeasurementCreator)

namespace {
  /** Helper: Create a TrackPoint from a measurement */
  genfit::TrackPoint* createTrackPoint(genfit::AbsMeasurement* coordinateMeasurement, RecoTrack& recoTrack,
                                       const RecoHitInformation& recoHitInformation)
  {
    genfit::TrackPoint* coordinateTrackPoint = new genfit::TrackPoint(coordinateMeasurement, &recoTrack);
    coordinateTrackPoint->setSortingParameter(recoHitInformation.getSortingParameter());

    return coordinateTrackPoint;
  }

  /** Helper: Go through all measurement creators in the given list and create the measurement with a given hit */
  template <class HitType, Const::EDetector detector>
  void measurementAdder(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, HitType* const hit,
                        const std::vector<std::unique_ptr<BaseMeasurementCreatorFromHit<HitType, detector>>>& measurementCreators)
  {
    for (const auto& measurementCreator : measurementCreators) {
      const std::vector<genfit::AbsMeasurement*>& measurements = measurementCreator->createMeasurements(hit, recoTrack,
                                                                 recoHitInformation);
      for (genfit::AbsMeasurement* measurement : measurements) {
        genfit::TrackPoint* trackPointFromMeasurement = createTrackPoint(measurement, recoTrack, recoHitInformation);
        recoTrack.insertPoint(trackPointFromMeasurement);
      }
    }
  }
}

MeasurementCreatorModule::MeasurementCreatorModule() : Module(),
  m_cdcMeasurementCreatorFactory(m_measurementFactory),
  m_svdMeasurementCreatorFactory(m_measurementFactory),
  m_pxdMeasurementCreatorFactory(m_measurementFactory)
{
  setDescription("Create measurements from the hits added to the RecoTracks and add them to the genfit tracks. Can also create new measurements like momentum estimations.");
  addParam("useVXDMomentumEstimation", m_param_useVXDMomentumEstimation, "Use the momentum estimation from VXD.", false);
  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "Store array name for the reco tracks to add the measurements to.", std::string("RecoTracks"));
  addParam("storeArrayNameOfCDCHits", m_param_storeArrayNameOfCDCHits, "Store array name for the cdc hits.", std::string("CDCHits"));
  addParam("storeArrayNameOfSVDHits", m_param_storeArrayNameOfSVDHits, "Store array name for the svd hits.",
           std::string("SVDClusters"));
  addParam("storeArrayNameOfPXDHits", m_param_storeArrayNameOfPXDHits, "Store array name for the pxd hits.",
           std::string("PXDClusters"));

  addParam("usedCDCMeasurementCreators", m_cdcMeasurementCreatorFactory.getParameters(),
           "Dictionary with the used CDC measurement creators and their parameters (as dict also)");
  addParam("usedSVDMeasurementCreators", m_svdMeasurementCreatorFactory.getParameters(),
           "Dictionary with the used SVD measurement creators and their parameters (as dict also)");
  addParam("usedPXDMeasurementCreators", m_pxdMeasurementCreatorFactory.getParameters(),
           "Dictionary with the used PXD measurement creators and their parameters (as dict also)");
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

  // Create the measurement creators
  m_cdcMeasurementCreatorFactory.initialize();
  m_svdMeasurementCreatorFactory.initialize();
  m_pxdMeasurementCreatorFactory.initialize();
}


void MeasurementCreatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  for (RecoTrack& recoTrack : recoTracks) {
    constructHitsForTrack(recoTrack);
  }
}


void MeasurementCreatorModule::constructHitsForTrack(RecoTrack& recoTrack) const
{
  // create TrackPoints
  // Loop over all hits and create an abs measurement with the creators.
  // then create a TrackPoint from that and set the sorting parameter
  recoTrack.mapOnHits<RecoTrack::UsedCDCHit>(recoTrack.getStoreArrayNameOfCDCHits(),
                                             std::bind(measurementAdder<RecoTrack::UsedCDCHit, Const::CDC>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_cdcMeasurementCreatorFactory.getCreators())));

  recoTrack.mapOnHits<RecoTrack::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(),
                                             std::bind(measurementAdder<RecoTrack::UsedSVDHit, Const::SVD>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_svdMeasurementCreatorFactory.getCreators())));

  recoTrack.mapOnHits<RecoTrack::UsedPXDHit>(recoTrack.getStoreArrayNameOfPXDHits(),
                                             std::bind(measurementAdder<RecoTrack::UsedPXDHit, Const::PXD>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_pxdMeasurementCreatorFactory.getCreators())));

  recoTrack.sort();
}
