/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/MeasurementFactory.h>
#include <framework/gearbox/Const.h>
#include <framework/core/Module.h>
#include <string>

namespace genfit {
  class AbsMeasurement;
}


namespace Belle2 {

  class RecoHitInformation;
  class CDCRecoHit;


  /**
   * If you want to provide another measurement type to the tracks, use the add*Measurement functions for that.
   */
  class MeasurementCreatorModule : public Module {

  public:
    /** Constructor .
     */
    MeasurementCreatorModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;


  private:
    /** StoreArray name of the input and output reco tracks */
    std::string m_param_recoTracksStoreArrayName;
    /** Use the momentum estimation from VXD */
    bool m_param_useVXDMomentumEstimation;
    /** The name of the store array for the cdc hits */
    std::string m_param_storeArrayNameOfCDCHits;
    /** The name of the store array for the svd hits */
    std::string m_param_storeArrayNameOfSVDHits;
    /** The name of the store array for the pxd hits */
    std::string m_param_storeArrayNameOfPXDHits;

    /** The measurement factory we will use */
    genfit::MeasurementFactory<genfit::AbsMeasurement> m_measurementFactory;

    /**
     * Helper function to construct the measurements for the added hits of a reco track.
     * @param recoTrack to construct the measurements/track hits for.
     */
    void constructHitsForTrack(RecoTrack& recoTrack) const;

    /** Add a measurement for a given cdc hit */
    void addCDCMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, RecoTrack::UsedCDCHit* const hit) const;

    /** Add a measurement for a given svd hit */
    void addSVDMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, RecoTrack::UsedSVDHit* const hit) const;

    /** Add a measurement for a given pxd hit */
    void addPXDMeasurement(RecoTrack& recoTrack, RecoHitInformation& recoHitInformation, RecoTrack::UsedPXDHit* const hit) const;
  };
}

