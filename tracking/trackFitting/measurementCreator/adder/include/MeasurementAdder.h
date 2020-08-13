/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFitting/measurementCreator/adder/MeasurementAdder.h>

#include <tracking/trackFitting/measurementCreator/factories/CDCMeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/factories/SVDMeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/factories/PXDMeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/factories/BKLMMeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/factories/EKLMMeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/factories/AdditionalMeasurementCreatorFactory.h>

#include <genfit/MeasurementFactory.h>
#include <string>
#include <map>


namespace genfit {
  class AbsMeasurement;
}


namespace Belle2 {

  class RecoTrack;

  /**
   * Algorithm class to translate the added detector hits (e.g. CDCHits) to internal TrackPoints
   * before fitting. This can be - depending on your setting - a non trivial process and should be taken
   * care only by this class.
   *
   * You probably do not have to use this class on your own but use the TrackFitter for that.
   *
   * Before fitting a reco track with the track fitter class, you have to translate all detector signals
   * into measurements (contained in TrackPoints) for genfit. This is done using predefined measurement creator classes.
   *
   * If you do not want non-default settings, you can call the measurement adder class with the following lines
   * of code:
   *
   *    MeasurementAdder measurementAdder;
   *    measurementAdder.setMeasurementCreatorsToDefaultSettings()
   *
   *    measurementAdder.addMeasurements(recoTrack);
   *
   * After that, you can fit the track (see the TrackFitter class).
   *
   * If you want to provide special settings, you can either create a list of MeasurementCreators for the three detectors
   * and for the case without detector by yourself or you use the MeasurementCreatorFactories. Both cases are shown below:
   *
   *    MeasurementAdder measurementAdder;
   *    // Possibility 1
   *    measurementAdder.setMeasurementCreators({ cdcMeasurementCreatorSharedPointer1, cdcMeasurementCreatorSharedPointer2, ...},
   *                                            { svdMeasurementCreatorSharedPointer1, ...},
   *                                            { pxdMeasurementCreatorSharedPointer1, ...},
   *                                            { additionalMeasurementCreatorSharedPointer1, ...})
   *
   *    // Possibility 2
   *    measurementAdder.setMeasurementCreatorsUsingFactories( mapOfCreatorNamesAndSettingsForCDC,
   *                                                           mapOfCreatorNamesAndSettingsForSVD,
   *                                                           mapOfCreatorNamesAndSettingsForPXD,
   *                                                           mapOfCreatorNamesAndSettingsForAdditionalMeasurements )
   *
   *    measurementAdder.addMeasurements(recoTrack);
   *
   */
  class MeasurementAdder {
  public:
    /**
     * Create a new instance of the measurement adder.
     * You probably only have to do this once in your module, except the case you want to test out different settings.
     */
    MeasurementAdder(const std::string& storeArrayNameOfPXDHits,
                     const std::string& storeArrayNameOfSVDHits,
                     const std::string& storeArrayNameOfCDCHits,
                     const std::string& storeArrayNameOfBKLMHits,
                     const std::string& storeArrayNameOfEKLMHits,
                     const bool initializeCDCTranslators = true);

    /**
     * Reset the internal measurement creator storage to the default settings.
     * The measurements will not be recreated if the dirty flag is not set (the hit content did not change).
     */
    void resetMeasurementCreatorsToDefaultSettings();

    /**
     * If you want to use non-default settings for the store arrays, you can create your own instances
     * of the measurement creators for the different detectors and use them here.
     *
     * For this method, you have to create the measurement creators by yourself.
     *
     * It is assumed, that you only do this for non-default settings. This is why all
     * measurement points will be recreated (independent on the dirty flag)
     * and the track must be refitted afterwards.
     */
    void resetMeasurementCreators(
      const std::vector<std::shared_ptr<PXDBaseMeasurementCreator>>& pxdMeasurementCreators,
      const std::vector<std::shared_ptr<SVDBaseMeasurementCreator>>& svdMeasurementCreators,
      const std::vector<std::shared_ptr<CDCBaseMeasurementCreator>>& cdcMeasurementCreators,
      const std::vector<std::shared_ptr<BKLMBaseMeasurementCreator>>& bklmMeasurementCreators,
      const std::vector<std::shared_ptr<EKLMBaseMeasurementCreator>>& eklmMeasurementCreators,
      const std::vector<std::shared_ptr<BaseMeasurementCreator>>& additionalMeasurementCreators);

    /**
     * If you want to use non-default settings for the store arrays, you can create your own instances
     * of the measurement creators for the different detectors and use them here.
     *
     * In this method, the measurement creators are created by MeasurementCreatorFactories. For this,
     * you have to give a map in the form
     *   { "creatorName" :
     *     { "parameterName1": parameterValue1,
     *       "parameterName2": parameterValue2, ... },
     *     "otherCreatorName" :
     *     { "otherParameterName1": otherParameterValue1, ... }, ... }
     * for each detector and for additional measurements. See the MeasurementCreatorFactories for more details.
     *
     * It is assumed, that you only do this for non-default settings. This is why all
     * measurement points will be recreated (independent on the dirty flag)
     * and the track must be refitted afterwards.
     */
    void resetMeasurementCreatorsUsingFactories(
      const std::map<std::string, std::map<std::string, std::string>>& pxdMeasurementCreators,
      const std::map<std::string, std::map<std::string, std::string>>& svdMeasurementCreators,
      const std::map<std::string, std::map<std::string, std::string>>& cdcMeasurementCreators,
      const std::map<std::string, std::map<std::string, std::string>>& bklmMeasurementCreators,
      const std::map<std::string, std::map<std::string, std::string>>& eklmMeasurementCreators,
      const std::map<std::string, std::map<std::string, std::string>>& additionalMeasurementCreators);

    /**
     * After you have filled the internal storage with measurement creators
     * (either by providing your own or by using the default settings), you can use then to add measurements to the
     * reco tracks with this function.
     *
     * @attention If you are using default measurement creators and the dirty flag of the reco track is not set,
     * this method will return and not nothing, as we do not want to
     * recreate all measurements twice. If you want to create all measurements again
     * (because you used different settings before), you have to set the dirty flag of the reco track manually.
     *
     * Returns true, if the track should be refitted afterwards and false else.
     */
    bool addMeasurements(RecoTrack& recoTrack) const;

  private:
    /// The name of the store array for the PXD hits.
    std::string m_param_storeArrayNameOfPXDHits = "";
    /// The name of the store array for the SVD hits.
    std::string m_param_storeArrayNameOfSVDHits = "";
    /// The name of the store array for the CDC hits.
    std::string m_param_storeArrayNameOfCDCHits = "";
    /// The name of the store array for the BKLM hits.
    std::string m_param_storeArrayNameOfBKLMHits = "";
    /// The name of the store array for the EKLM hits.
    std::string m_param_storeArrayNameOfEKLMHits = "";
    /// Flag to skip the dirty check. Useful when using non default measurement creators.
    bool m_skipDirtyCheck = false;

    /// Internal storage of the PXD measurement creators.
    std::vector<std::shared_ptr<PXDBaseMeasurementCreator>> m_pxdMeasurementCreators;
    /// Internal storage of the SVD measurement creators.
    std::vector<std::shared_ptr<SVDBaseMeasurementCreator>> m_svdMeasurementCreators;
    /// Internal storage of the CDC measurement creators.
    std::vector<std::shared_ptr<CDCBaseMeasurementCreator>> m_cdcMeasurementCreators;
    /// Internal storage of the BKLM measurement creators.
    std::vector<std::shared_ptr<BKLMBaseMeasurementCreator>> m_bklmMeasurementCreators;
    /// Internal storage of the EKLM measurement creators.
    std::vector<std::shared_ptr<EKLMBaseMeasurementCreator>> m_eklmMeasurementCreators;
    /// Internal storage of the additional measurement creators.
    std::vector<std::shared_ptr<BaseMeasurementCreator>> m_additionalMeasurementCreators;

    /// Internal storage of the genfit measurement factory;
    genfit::MeasurementFactory<genfit::AbsMeasurement> m_genfitMeasurementFactory;

    /// Helper function to create a genfit::MeasurementFactory, needed in the MeasurementCreators.
    void createGenfitMeasurementFactory();

    /// Helper: Go through all measurement creators in the given list and create the measurement with a given hit.
    template <class HitType, Const::EDetector detector>
    void addMeasurementsFromHitToRecoTrack(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, HitType* hit,
                                           const std::vector<std::shared_ptr<BaseMeasurementCreatorFromHit<HitType, detector>>>& measurementCreators,
                                           std::map<genfit::TrackPoint*, RecoHitInformation*>& trackPointHitMapping) const
    {
      if (not recoHitInformation.useInFit()) {
        return;
      }

      genfit::Track& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);

      for (const auto& measurementCreator : measurementCreators) {
        const std::vector<genfit::TrackPoint*>& trackPoints = measurementCreator->createMeasurementPoints(hit, recoTrack,
                                                              recoHitInformation);
        for (genfit::TrackPoint* trackPoint : trackPoints) {
          genfitTrack.insertPoint(trackPoint);
          // FIXME: hotfix: to get a correct mapping between reco hit information and the track point.
          // We are not able to store the TrackPoint in the RecoHitInformation directly because of problems in streaming
          // the genfit::TrackPoint. So what we do is store the index of the track points in the vector of the genfit::Track.
          // As this vector is sorted after this function, we can not set it here directly.
          trackPointHitMapping[trackPoint] = &recoHitInformation;
        }
      }
    }

    /// Helper: Go through all measurement creators in the given list and create the measurement without a given hit.
    void addMeasurementsToRecoTrack(RecoTrack& recoTrack,
                                    const std::vector<std::shared_ptr<BaseMeasurementCreator>>& measurementCreators) const;
  };
}

