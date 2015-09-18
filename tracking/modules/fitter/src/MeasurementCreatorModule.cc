#include <tracking/modules/fitter/MeasurementCreatorModule.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <framework/gearbox/Const.h>

using namespace Belle2;

REG_MODULE(MeasurementCreator)

namespace {
  /** Helper: Go through all measurement creators in the given list and create the measurement with a given hit */
  template <class HitType, Const::EDetector detector>
  void measurementAdder(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, HitType* const hit,
                        const std::vector<std::unique_ptr<BaseMeasurementCreatorFromHit<HitType, detector>>>& measurementCreators)
  {
    for (const auto& measurementCreator : measurementCreators) {
      const std::vector<genfit::TrackPoint*>& trackPoints = measurementCreator->createMeasurementPoints(hit, recoTrack,
                                                            recoHitInformation);
      for (genfit::TrackPoint* trackPoint : trackPoints) {
        recoTrack.insertPoint(trackPoint);
      }
    }
  }
}

MeasurementCreatorModule::MeasurementCreatorModule() : Module(),
  m_cdcMeasurementCreatorFactory(m_measurementFactory),
  m_svdMeasurementCreatorFactory(m_measurementFactory),
  m_pxdMeasurementCreatorFactory(m_measurementFactory),
  m_additionalMeasurementCreatorFactory()
{
  setDescription("Create measurements from the hits added to the RecoTracks and add them to the genfit tracks. Can also create new measurements like momentum estimations.");
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
  addParam("usedAdditionalMeasurementCreators", m_additionalMeasurementCreatorFactory.getParameters(),
           "Dictionary with the used additional measurement creators and their parameters (as dict also)");
}

void MeasurementCreatorModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  // Create a measurement factory
  StoreArray<RecoTrack::UsedPXDHit> pxdHits(m_param_storeArrayNameOfPXDHits);
  StoreArray<RecoTrack::UsedSVDHit> svdHits(m_param_storeArrayNameOfSVDHits);
  StoreArray<RecoTrack::UsedCDCHit> cdcHits(m_param_storeArrayNameOfCDCHits);

  // Create new Translators and give them to the CDCRecoHits.
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true);

  // Create the related measurement factory
  if (pxdHits.isOptional())
    m_measurementFactory.addProducer(Const::PXD, new genfit::MeasurementProducer<RecoTrack::UsedPXDHit, PXDRecoHit>(pxdHits.getPtr()));
  if (svdHits.isOptional())
    m_measurementFactory.addProducer(Const::SVD, new genfit::MeasurementProducer<RecoTrack::UsedSVDHit, SVDRecoHit>(svdHits.getPtr()));
  if (cdcHits.isOptional())
    m_measurementFactory.addProducer(Const::CDC, new genfit::MeasurementProducer<RecoTrack::UsedCDCHit, CDCRecoHit>(cdcHits.getPtr()));

  // Init the measurement creators
  m_cdcMeasurementCreatorFactory.initialize();
  m_svdMeasurementCreatorFactory.initialize();
  m_pxdMeasurementCreatorFactory.initialize();
  m_additionalMeasurementCreatorFactory.initialize();
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
                                             std::bind(&measurementAdder<RecoTrack::UsedCDCHit, Const::CDC>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_cdcMeasurementCreatorFactory.getCreators())));

  recoTrack.mapOnHits<RecoTrack::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(),
                                             std::bind(&measurementAdder<RecoTrack::UsedSVDHit, Const::SVD>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_svdMeasurementCreatorFactory.getCreators())));

  recoTrack.mapOnHits<RecoTrack::UsedPXDHit>(recoTrack.getStoreArrayNameOfPXDHits(),
                                             std::bind(&measurementAdder<RecoTrack::UsedPXDHit, Const::PXD>, std::ref(recoTrack), std::placeholders::_1, std::placeholders::_2,
                                                       std::cref(m_pxdMeasurementCreatorFactory.getCreators())));

  // Special case is with the additional measurement creator factories. They do not need any hits:
  for (const auto& measurementCreator : m_additionalMeasurementCreatorFactory.getCreators()) {
    const std::vector<genfit::TrackPoint*>& trackPoints = measurementCreator->createMeasurementPoints(recoTrack);
    for (genfit::TrackPoint* trackPoint : trackPoints) {
      recoTrack.insertPoint(trackPoint);
    }
  }

  recoTrack.sort();
}
