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
#include <alignment/reconstruction/BKLMRecoHit.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>

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

MeasurementAdder::MeasurementAdder(
  const std::string& storeArrayNameOfPXDHits,
  const std::string& storeArrayNameOfSVDHits,
  const std::string& storeArrayNameOfCDCHits,
  const std::string& storeArrayNameOfBKLMHits,
  const std::string& storeArrayNameOfEKLMHits,
  const bool initializeCDCTranslators) :
  m_param_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
  m_param_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_param_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_param_storeArrayNameOfBKLMHits(storeArrayNameOfBKLMHits),
  m_param_storeArrayNameOfEKLMHits(storeArrayNameOfEKLMHits)
{
  /** Configures whether the CDC Translators should be initialized by the FitterModule
   * especially useful for VXD-only beamtest. In the future this could be changed to check
   * implicitly if the cdc is available in the geometry.*/
  if (initializeCDCTranslators) {
    // Create new Translators and give them to the CDCRecoHits.
    CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                               new CDC::RealisticCDCGeometryTranslator(true),
                               new CDC::RealisticTDCCountTranslator(true),
                               true);
  }
  createGenfitMeasurementFactory();
  resetMeasurementCreatorsToDefaultSettings();
}

void MeasurementAdder::createGenfitMeasurementFactory()
{
  StoreArray<RecoHitInformation::UsedCDCHit> cdcHits(m_param_storeArrayNameOfCDCHits);
  StoreArray<RecoHitInformation::UsedSVDHit> svdHits(m_param_storeArrayNameOfSVDHits);
  StoreArray<RecoHitInformation::UsedPXDHit> pxdHits(m_param_storeArrayNameOfPXDHits);
  StoreArray<RecoHitInformation::UsedBKLMHit> bklmHits(m_param_storeArrayNameOfBKLMHits);
  StoreArray<RecoHitInformation::UsedEKLMHit> eklmHits(m_param_storeArrayNameOfEKLMHits);

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
  if (bklmHits.isOptional())
    m_genfitMeasurementFactory.addProducer(Const::BKLM,
                                           new genfit::MeasurementProducer<RecoHitInformation::UsedBKLMHit, BKLMRecoHit>(bklmHits.getPtr()));
  if (eklmHits.isOptional())
    m_genfitMeasurementFactory.addProducer(Const::EKLM,
                                           new genfit::MeasurementProducer<RecoHitInformation::UsedEKLMHit, AlignableEKLMRecoHit>(eklmHits.getPtr()));
}

void MeasurementAdder::resetMeasurementCreatorsToDefaultSettings()
{
  m_pxdMeasurementCreators = { std::shared_ptr<PXDBaseMeasurementCreator>(new PXDCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_svdMeasurementCreators = { std::shared_ptr<SVDBaseMeasurementCreator>(new SVDCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_cdcMeasurementCreators = { std::shared_ptr<CDCBaseMeasurementCreator>(new CDCCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_bklmMeasurementCreators = { std::shared_ptr<BKLMBaseMeasurementCreator>(new BKLMCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_eklmMeasurementCreators = { std::shared_ptr<EKLMBaseMeasurementCreator>(new EKLMCoordinateMeasurementCreator(m_genfitMeasurementFactory)) };
  m_additionalMeasurementCreators.clear();

  m_skipDirtyCheck = false;
}

void MeasurementAdder::resetMeasurementCreators(
  const std::vector<std::shared_ptr<PXDBaseMeasurementCreator>>& pxdMeasurementCreators,
  const std::vector<std::shared_ptr<SVDBaseMeasurementCreator>>& svdMeasurementCreators,
  const std::vector<std::shared_ptr<CDCBaseMeasurementCreator>>& cdcMeasurementCreators,
  const std::vector<std::shared_ptr<BKLMBaseMeasurementCreator>>& bklmMeasurementCreators,
  const std::vector<std::shared_ptr<EKLMBaseMeasurementCreator>>& eklmMeasurementCreators,
  const std::vector<std::shared_ptr<BaseMeasurementCreator>>& additionalMeasurementCreators)
{
  m_pxdMeasurementCreators = pxdMeasurementCreators;
  m_svdMeasurementCreators = svdMeasurementCreators;
  m_cdcMeasurementCreators = cdcMeasurementCreators;
  m_bklmMeasurementCreators = bklmMeasurementCreators;
  m_eklmMeasurementCreators = eklmMeasurementCreators;
  m_additionalMeasurementCreators = additionalMeasurementCreators;

  m_skipDirtyCheck = true;
}

void MeasurementAdder::resetMeasurementCreatorsUsingFactories(
  const std::map<std::string, std::map<std::string, std::string>>& pxdMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& svdMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& cdcMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& bklmMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& eklmMeasurementCreators,
  const std::map<std::string, std::map<std::string, std::string>>& additionalMeasurementCreators)
{
  PXDMeasurementCreatorFactory pxdMeasurementCreatorFactory(m_genfitMeasurementFactory);
  SVDMeasurementCreatorFactory svdMeasurementCreatorFactory(m_genfitMeasurementFactory);
  CDCMeasurementCreatorFactory cdcMeasurementCreatorFactory(m_genfitMeasurementFactory);
  BKLMMeasurementCreatorFactory bklmMeasurementCreatorFactory(m_genfitMeasurementFactory);
  EKLMMeasurementCreatorFactory eklmMeasurementCreatorFactory(m_genfitMeasurementFactory);
  AdditionalMeasurementCreatorFactory additionalMeasurementCreatorFactory;

  pxdMeasurementCreatorFactory.setParameters(pxdMeasurementCreators);
  svdMeasurementCreatorFactory.setParameters(svdMeasurementCreators);
  cdcMeasurementCreatorFactory.setParameters(cdcMeasurementCreators);
  bklmMeasurementCreatorFactory.setParameters(bklmMeasurementCreators);
  eklmMeasurementCreatorFactory.setParameters(eklmMeasurementCreators);
  additionalMeasurementCreatorFactory.setParameters(additionalMeasurementCreators);

  pxdMeasurementCreatorFactory.initialize();
  svdMeasurementCreatorFactory.initialize();
  cdcMeasurementCreatorFactory.initialize();
  bklmMeasurementCreatorFactory.initialize();
  eklmMeasurementCreatorFactory.initialize();
  additionalMeasurementCreatorFactory.initialize();

  m_pxdMeasurementCreators = pxdMeasurementCreatorFactory.getCreators();
  m_svdMeasurementCreators = svdMeasurementCreatorFactory.getCreators();
  m_cdcMeasurementCreators = cdcMeasurementCreatorFactory.getCreators();
  m_bklmMeasurementCreators = bklmMeasurementCreatorFactory.getCreators();
  m_eklmMeasurementCreators = eklmMeasurementCreatorFactory.getCreators();
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
  B2ASSERT("Reco Track was built with another BKLM store array: Reco Track "
           << recoTrack.getStoreArrayNameOfBKLMHits()
           << ", this class " << m_param_storeArrayNameOfBKLMHits,
           checkTwoStoreArrayNames<RecoHitInformation::UsedBKLMHit>(recoTrack.getStoreArrayNameOfBKLMHits(),
               m_param_storeArrayNameOfBKLMHits));
  B2ASSERT("Reco Track was built with another EKLM store array: Reco Track "
           << recoTrack.getStoreArrayNameOfEKLMHits()
           << ", this class " << m_param_storeArrayNameOfEKLMHits,
           checkTwoStoreArrayNames<RecoHitInformation::UsedEKLMHit>(recoTrack.getStoreArrayNameOfEKLMHits(),
               m_param_storeArrayNameOfEKLMHits));

  if (m_cdcMeasurementCreators.size() == 0 and m_svdMeasurementCreators.size() == 0 and m_pxdMeasurementCreators.size() == 0 and
      m_bklmMeasurementCreators.size() == 0 and m_eklmMeasurementCreators.size() == 0 and m_additionalMeasurementCreators.size() == 0) {
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
  RecoTrackGenfitAccess::getGenfitTrack(recoTrack).deleteTrackPointsAndFitStatus();
  recoTrack.setDirtyFlag();

  // Add the measurements created by the CDC, SVD and PXD measurement creators.
  recoTrack.mapOnHits<RecoHitInformation::UsedPXDHit>(m_param_storeArrayNameOfPXDHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedPXDHit * pxdHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedPXDHit, Const::PXD>(recoTrack, recoHitInformation, pxdHit,
        m_pxdMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedSVDHit>(m_param_storeArrayNameOfSVDHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedSVDHit * svdHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedSVDHit, Const::SVD>(recoTrack, recoHitInformation, svdHit,
        m_svdMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedCDCHit>(m_param_storeArrayNameOfCDCHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedCDCHit * cdcHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedCDCHit, Const::CDC>(recoTrack, recoHitInformation, cdcHit,
        m_cdcMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedBKLMHit>(m_param_storeArrayNameOfBKLMHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedBKLMHit * bklmHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedBKLMHit, Const::BKLM>(recoTrack, recoHitInformation, bklmHit,
        m_bklmMeasurementCreators);
  });
  recoTrack.mapOnHits<RecoHitInformation::UsedEKLMHit>(m_param_storeArrayNameOfEKLMHits, [&](RecoHitInformation & recoHitInformation,
  RecoHitInformation::UsedEKLMHit * EKLMHit) {
    addMeasurementsFromHitToRecoTrack<RecoHitInformation::UsedEKLMHit, Const::EKLM>(recoTrack, recoHitInformation, EKLMHit,
        m_eklmMeasurementCreators);
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
