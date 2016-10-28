/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFitting/measurementCreator/adder/MeasurementAdder.h>
#include <tracking/trackFitting/measurementCreator/creators/CoordinateMeasurementCreator.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <genfit/MeasurementFactory.h>
#include <string>

using namespace Belle2;


namespace {
  /// Check if two store array names of the given type T describe the same store array (also with default names).
  template <class T>
  bool checkTwoStoreArrayNames(const std::string& firstName, const std::string& secondName)
  {
    return ((firstName == secondName) or
            (firstName.empty() and secondName == DataStore::defaultArrayName<T>()) or
            (secondName.empty() and firstName == DataStore::defaultArrayName<T>()));
  }
}

MeasurementAdder::MeasurementAdder(const std::string& storeArrayNameOfCDCHits,
                                   const std::string& storeArrayNameOfSVDHits,
                                   const std::string& storeArrayNameOfPXDHits,
                                   const bool& cosmicsTemporaryFix) :
  m_param_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_param_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_param_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits)
{
  // Create new Translators and give them to the CDCRecoHits.
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true, cosmicsTemporaryFix);

  createGenfitMeasurementFactory();
  resetMeasurementCreatorsToDefaultSettings();
}

void MeasurementAdder::createGenfitMeasurementFactory()
{
  StoreArray<RecoHitInformation::UsedPXDHit> pxdHits(m_param_storeArrayNameOfPXDHits);
  StoreArray<RecoHitInformation::UsedSVDHit> svdHits(m_param_storeArrayNameOfSVDHits);
  StoreArray<RecoHitInformation::UsedCDCHit> cdcHits(m_param_storeArrayNameOfCDCHits);

  // Create the related measurement factory
  if (pxdHits.isOptional())
    m_genfitMeasurementFactory.addProducer(Const::PXD,
                                           new genfit::MeasurementProducer<RecoHitInformation::UsedPXDHit, PXDRecoHit>(pxdHits.getPtr()));
  if (svdHits.isOptional())
    m_genfitMeasurementFactory.addProducer(Const::SVD,
                                           new genfit::MeasurementProducer<RecoHitInformation::UsedSVDHit, SVDRecoHit>(svdHits.getPtr()));
  if (cdcHits.isOptional())
    m_genfitMeasurementFactory.addProducer(Const::CDC,
                                           new genfit::MeasurementProducer<RecoHitInformation::UsedCDCHit, CDCRecoHit>(cdcHits.getPtr()));
}

void MeasurementAdder::resetMeasurementCreatorsToDefaultSettings()
{
  m_cdcMeasurementCreators = { std::shared_ptr<CDCBaseMeasurementCreator>(new CDCCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_svdMeasurementCreators = { std::shared_ptr<SVDBaseMeasurementCreator>(new SVDCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_pxdMeasurementCreators = { std::shared_ptr<PXDBaseMeasurementCreator>(new PXDCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_additionalMeasurementCreators.clear();

  m_skipDirtyCheck = false;
}

void MeasurementAdder::resetMeasurementCreators(
  const std::vector<std::shared_ptr<CDCBaseMeasurementCreator>>& cdcMeasurementCreators,
  const std::vector<std::shared_ptr<SVDBaseMeasurementCreator>>& svdMeasurementCreators,
  const std::vector<std::shared_ptr<PXDBaseMeasurementCreator>>& pxdMeasurementCreators,
  const std::vector<std::shared_ptr<BaseMeasurementCreator>>& additionalMeasurementCreators)
{
  m_cdcMeasurementCreators = cdcMeasurementCreators;
  m_svdMeasurementCreators = svdMeasurementCreators;
  m_pxdMeasurementCreators = pxdMeasurementCreators;
  m_additionalMeasurementCreators = additionalMeasurementCreators;

  m_skipDirtyCheck = true;
}

void MeasurementAdder::resetMeasurementCreatorsUsingFactories(
  const std::map<std::string, std::map<std::string, std::string>>& cdcMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& svdMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& pxdMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& additionalMeasurementCreators)
{
  CDCMeasurementCreatorFactory cdcMeasurementCreatorFactory(m_genfitMeasurementFactory);
  SVDMeasurementCreatorFactory svdMeasurementCreatorFactory(m_genfitMeasurementFactory);
  PXDMeasurementCreatorFactory pxdMeasurementCreatorFactory(m_genfitMeasurementFactory);
  AdditionalMeasurementCreatorFactory additionalMeasurementCreatorFactory;

  cdcMeasurementCreatorFactory.setParameters(cdcMeasurementCreators);
  svdMeasurementCreatorFactory.setParameters(svdMeasurementCreators);
  pxdMeasurementCreatorFactory.setParameters(pxdMeasurementCreators);
  additionalMeasurementCreatorFactory.setParameters(additionalMeasurementCreators);

  cdcMeasurementCreatorFactory.initialize();
  svdMeasurementCreatorFactory.initialize();
  pxdMeasurementCreatorFactory.initialize();
  additionalMeasurementCreatorFactory.initialize();

  m_cdcMeasurementCreators = cdcMeasurementCreatorFactory.getCreators();
  m_svdMeasurementCreators = svdMeasurementCreatorFactory.getCreators();
  m_pxdMeasurementCreators = pxdMeasurementCreatorFactory.getCreators();
  m_additionalMeasurementCreators = additionalMeasurementCreatorFactory.getCreators();

  m_skipDirtyCheck = true;
}

bool MeasurementAdder::addMeasurements(RecoTrack& recoTrack) const
{
  B2ASSERT("Reco Track was built with another CDC store array: Reco Track "
           << recoTrack.getStoreArrayNameOfCDCHits()
           << ", this class " << m_param_storeArrayNameOfCDCHits,
           checkTwoStoreArrayNames<RecoHitInformation::UsedCDCHit>(recoTrack.getStoreArrayNameOfCDCHits(), m_param_storeArrayNameOfCDCHits));
  B2ASSERT("Reco Track was built with another SVD store array: Reco Track "
           << recoTrack.getStoreArrayNameOfSVDHits()
           << ", this class " << m_param_storeArrayNameOfSVDHits,
           checkTwoStoreArrayNames<RecoHitInformation::UsedSVDHit>(recoTrack.getStoreArrayNameOfSVDHits(), m_param_storeArrayNameOfSVDHits));
  B2ASSERT("Reco Track was built with another PXD store array: Reco Track "
           << recoTrack.getStoreArrayNameOfPXDHits()
           << ", this class " << m_param_storeArrayNameOfPXDHits,
           checkTwoStoreArrayNames<RecoHitInformation::UsedPXDHit>(recoTrack.getStoreArrayNameOfPXDHits(), m_param_storeArrayNameOfPXDHits));

  if (m_cdcMeasurementCreators.size() == 0 and m_svdMeasurementCreators.size() == 0 and m_pxdMeasurementCreators.size() == 0
      and m_additionalMeasurementCreators.size() == 0) {
    B2WARNING("No measurement creators where added to this instance, so no measurements where created. Did you forget to call setMeasurementCreators?");
    return false;
  }

  if (not recoTrack.getDirtyFlag() and not m_skipDirtyCheck) {
    B2DEBUG(100, "Hit content did not change since last measurement adding and you are using default parameters." <<
            "I will not add the measurements again. " <<
            "If you still want to do so, set the dirty flag to true.");
    return false;
  }

  // Delete all other measurements.
  recoTrack.deleteTrackPointsAndFitStatus();

  // Add the measurements created by the CDC, SVD and PXD measurement creators.
  recoTrack.mapOnHits<RecoHitInformation::UsedCDCHit>(m_param_storeArrayNameOfCDCHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedCDCHit * cdcHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedCDCHit, Const::CDC>(recoTrack, recoHitInformation, cdcHit,
        m_cdcMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedSVDHit>(m_param_storeArrayNameOfSVDHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedSVDHit * svdHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedSVDHit, Const::SVD>(recoTrack, recoHitInformation, svdHit,
        m_svdMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedPXDHit>(m_param_storeArrayNameOfPXDHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedPXDHit * pxdHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedPXDHit, Const::PXD>(recoTrack, recoHitInformation, pxdHit,
        m_pxdMeasurementCreators);
  });

  // Special case is with the additional measurement creator factories. They do not need any hits:
  addMeasurementsToRecoTrack(recoTrack, m_additionalMeasurementCreators);

  RecoTrackGenfitAccess::getGenfitTrack(recoTrack).sort();

  return true;
}

void MeasurementAdder::addMeasurementsToRecoTrack(RecoTrack& recoTrack,
                                                  const std::vector<std::shared_ptr<BaseMeasurementCreator>>& measurementCreators) const
{
  for (const auto& measurementCreator : measurementCreators) {
    const std::vector<genfit::TrackPoint*>& trackPoints = measurementCreator->createMeasurementPoints(recoTrack);
    for (genfit::TrackPoint* trackPoint : trackPoints) {
      RecoTrackGenfitAccess::getGenfitTrack(recoTrack).insertPoint(trackPoint);
    }
  }
}
