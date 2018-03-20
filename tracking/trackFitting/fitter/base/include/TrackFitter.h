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
#include <framework/gearbox/Const.h>

#include <TError.h>

#include <string>
#include <memory>

namespace genfit {
  class AbsFitter;
  class AbsTrackRep;
}

namespace Belle2 {

  class RecoTrack;

  /**
   * Algorithm class to handle the fitting of RecoTrack objects. The fitting of reco tracks implies non-trivial synchronisation,
   * so you should only use this class for fitting RecoTrack instances.
   *
   * After having created a RecoTrack and added hits to it, you can fit the RecoTrack.
   *
   *   RecoTrack recoTrack;
   *   ... add CDC hits etc. ...
   *   ... set seeds etc. ...
   *
   * The standard case
   * -----------------
   *
   * Typical use case if you do not use the fitter modules but want to fit in your own module.
   *
   * TrackFitter trackFitter;
   * trackFitter.fit(recoTrack);
   *
   * -> Create measurements if hit content has changed (dirty flag)
   * -> Fit (again) if (a) no fit result present or (b) hit content has changed (dirty flag)
   *
   * Non-default fitter
   * ------------------
   *
   * May be used if you know that the default fitting algorithm is not suited for your use case.
   * Please be aware, that the following modules may use your fit results, if the hit content is not changed,
   * although they may expect a "default"-fit.
   * Because the currently present fit results were probably created with the default fitter, the fit is executed always (and not only when the hit content has changed).
   *
   * ATTENTION: If the hit content did not change (the dirty flag is not set), the track will not be refitted later! Future modules may expect a default fit!
   *
   * TrackFitter trackFitter;
   * trackFitter.resetFitter( ... );
   * trackFitter.fit(recoTrack);
   *
   * -> Create measurements if hit content has changed (dirty flag)
   * -> Always fitted
   *
   * Non-default track representations
   * ---------------------------------
   *
   * Useful if you know you have an electron/etc. instead of a pion. Please make sure that the cardinal representation points to the correct representation - especially when you
   * try out non-RKTrackRep objects.
   * ATTENTION: Once a track representation is added to the track, it will always be also used when fitting! If you do not want this, remove this track representation again.
   *
   * TrackFitter trackFitter;
   * // Maybe set other fit algorithm.
   * trackFitter.fit(recoTrack, particleType);
   * or
   * trackFitter.fit(recoTrack, trackRep);
   *
   * -> Create measurements if hit content has changed (dirty flag)
   * -> Fit (again) if (a) added track representation is new or (b) hit content has changed (dirty flag) or (c) you use a non-default fitter.
   *
   * Non-default measurements
   * ------------------------
   *
   * This is only needed in very rare cases, when you want to transform the added hits into measurement points for your special use case e.g. alignment or VXD-dEdX-momentum estimation.
   *
   * ATTENTION: Again, please be aware that future modules may expect fit results from a default fit!
   *
   * TrackFitter trackFitter;
   * // Possibility 1
   * trackFitter.resetMeasurementCreators({ cdcMeasurementCreatorSharedPointer1, cdcMeasurementCreatorSharedPointer2, ...},
   *                                     { svdMeasurementCreatorSharedPointer1, ...},
   *                                     { pxdMeasurementCreatorSharedPointer1, ...},
   *                                     { additionalMeasurementCreatorSharedPointer1, ...})
   * // Possibility 2
   * measurementAdder.resetMeasurementCreatorsUsingFactories( mapOfCreatorNamesAndSettingsForCDC,
   *                                                          mapOfCreatorNamesAndSettingsForSVD,
   *                                                          mapOfCreatorNamesAndSettingsForPXD,
   *                                                          mapOfCreatorNamesAndSettingsForAdditionalMeasurements )
   *
   * // Maybe set other fit algorithm.
   * trackFitter.fit(recoTrack);
   * // or maybe use a different track representation/particle hypothesis.

   * -> Always recreate all measurements (not only when hit content has changed).
   * -> Always refit (not only when using non default parameters or hit content has changed or track representation is new).
   *
   * Because the different cases are rather complicated, there is a flow chart available.
   * TODO: Create flow chart.
   */
  class TrackFitter {
  public:
    /// Default deltaPValue for the default DAF fitter
    static constexpr double s_defaultDeltaPValue = 1.0;
    /// Default probCut for the default DAF fitter
    static constexpr double s_defaultProbCut = 0.001;
    /// Default maxFailedHits for the default DAF fitter
    static constexpr unsigned int s_defaultMaxFailedHits = 5;

    /// Create a new fitter instance.
    TrackFitter(const std::string& storeArrayNameOfPXDHits = "",
                const std::string& storeArrayNameOfSVDHits = "",
                const std::string& storeArrayNameOfCDCHits = "",
                const std::string& storeArrayNameOfBKLMHits = "",
                const std::string& storeArrayNameOfEKLMHits = "",
                const bool initializeCDCTranslators = true):
      m_measurementAdder(storeArrayNameOfPXDHits, storeArrayNameOfSVDHits, storeArrayNameOfCDCHits,
                         storeArrayNameOfBKLMHits, storeArrayNameOfEKLMHits, initializeCDCTranslators)
    {
      resetFitterToDefaultSettings();
    }

    /// Helper function to multiply the PDG code of a charged stable with the charge of the reco track (if needed)
    static int createCorrectPDGCodeForChargedStable(const Const::ChargedStable& particleType, const RecoTrack& recoTrack);

    /**
     * Set the internal storage of the fitter to a provided one, if you want to use non-default settings.
     *
     * Whenever you call this function, all tracks will be automatically refitted, although
     * they might be already fitted (because you use non-default parameters, so we assume you want other fit results).
     */
    void resetFitter(const std::shared_ptr<genfit::AbsFitter>& fitter);

    /**
     * Use the default settings of the fitter to fit the reco tracks.
     * This method is called on construction automatically.
     */
    void resetFitterToDefaultSettings();

    /**
     * Fit a reco track with a given non-default track representation.
     * You have to use a pointer to a track representation already in the reco track
     * (use recoTrack.getRepresentations() to get a list of pointers).
     *
     * If hit content did not change (indicated by the dirty flag of the reco track),
     * the track will not be refitted.
     *
     * This fit function is only to be used for non-standard expert use.
     * For the typical use case, please use the other fit function.
     *
     * Return bool if the track was successful.
     */
    bool fit(RecoTrack& recoTrack, genfit::AbsTrackRep* trackRepresentation) const;

    /**
     * Fit a reco track with the given particle hypothesis, or with pion as default.
     *
     * If the particle hypothesis is already
     * in the track representation list and the hit content did not change (the dirty flag is set to false),
     * the track is not refitted. If you still want to refit the track, set the dirty flag.
     *
     * Please be aware that in every fit process, ALL added track representations are fitted,
     * no matter if you added them in this function call or they where added before.
     *
     * Internally, a new track representation with the given particle hypothesis is created
     * and added to the reco track, if not already present. For this, a RKTrackRep is used as a
     * base class. The PDG-code-sign is deduced from the reco track charge.
     *
     * Return bool if the track was successful.
     */
    bool fit(RecoTrack& recoTrack, const Const::ChargedStable& particleType = Const::pion) const;

    /**
     * Reset the internal measurement creator storage to the default settings.
     * The measurements will not be recreated if the dirty flag is not set (the hit content did not change).
     */
    void resetMeasurementCreatorsToDefaultSettings()
    {
      m_measurementAdder.resetMeasurementCreatorsToDefaultSettings();
    }

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
      const std::vector<std::shared_ptr<BaseMeasurementCreator>>& additionalMeasurementCreators)
    {
      m_measurementAdder.resetMeasurementCreators(pxdMeasurementCreators,
                                                  svdMeasurementCreators,
                                                  cdcMeasurementCreators,
                                                  bklmMeasurementCreators,
                                                  eklmMeasurementCreators,
                                                  additionalMeasurementCreators);
    }

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
      const std::map<std::string, std::map<std::string, std::string>>& additionalMeasurementCreators)
    {
      m_measurementAdder.resetMeasurementCreatorsUsingFactories(pxdMeasurementCreators,
                                                                svdMeasurementCreators,
                                                                cdcMeasurementCreators,
                                                                bklmMeasurementCreators,
                                                                eklmMeasurementCreators,
                                                                additionalMeasurementCreators);
    }

    /// Set the gErrorIgnoreLevel for the fitter.
    void setgErrorIgnoreLevel(Int_t errorIgnoreLevel) { m_gErrorIgnoreLevel = errorIgnoreLevel; }
    /// Return the currently set gErrorIgnoreLevel for the fitter.
    Int_t getgErrorIgnoreLevel() { return m_gErrorIgnoreLevel; }

  private:
    /// The internal storage of the used fitting algorithms.
    std::shared_ptr<genfit::AbsFitter> m_fitter;
    /// Flag to skip the dirty flag check which is needed when using non-default fitters.
    bool m_skipDirtyCheck = false;

    /// The measurement adder algorithm class
    MeasurementAdder m_measurementAdder;

    /// Control the output level of the ROOT functions used by the GenFit fitter. Default is increased from kError to kFatal;
    Int_t m_gErrorIgnoreLevel = kFatal;

    /**
     * Helper function to do the fit.
     * This function will neither check the dirty flag nor if the track representation is added to the
     * trackrep list of the reco track. This should only be called internally.
     *
     * In every fit step, all track representations are fitted with genfit. The given track representation is only used
     * for calculating the time seed for the fit. For this, the track representation needs to have the correct PDG code set
     * (indicating the correct particle AND the correct charge).
     */
    bool fitWithoutCheck(RecoTrack& recoTrack, const genfit::AbsTrackRep& trackRepresentation) const;
  };
}

