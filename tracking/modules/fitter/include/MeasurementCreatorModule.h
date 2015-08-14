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

#include <tracking/measurementCreator/factories/CDCMeasurementCreatorFactory.h>
#include <tracking/measurementCreator/factories/SVDMeasurementCreatorFactory.h>
#include <tracking/measurementCreator/factories/PXDMeasurementCreatorFactory.h>
#include <tracking/measurementCreator/factories/AdditionalMeasurementCreatorFactory.h>

#include <genfit/MeasurementFactory.h>
#include <framework/core/Module.h>
#include <string>


namespace genfit {
  class AbsMeasurement;
}


namespace Belle2 {

  class RecoTrack;

  /**
   * If you want to provide another measurement type to the tracks, use the parameter used*Creators for that.
   * You have to implement a BaseMeasurementCreatorFromHit derived class for that and add it to the *MeasurementCreatorFactory.
   * Then you can choose it and its parameters from a steering file.
   */
  class MeasurementCreatorModule : public Module {

  public:
    /**
     * Constructor .
     */
    MeasurementCreatorModule();

    /**
     * Initialize the Module.
     * Setup the used measurement creators from the factories.
     */
    void initialize() override;

    /**
     * Add the measurements to the factories.
     */
    void event() override;


  private:
    /** StoreArray name of the input and output reco tracks */
    std::string m_param_recoTracksStoreArrayName;
    /** The name of the store array for the cdc hits */
    std::string m_param_storeArrayNameOfCDCHits;
    /** The name of the store array for the svd hits */
    std::string m_param_storeArrayNameOfSVDHits;
    /** The name of the store array for the pxd hits */
    std::string m_param_storeArrayNameOfPXDHits;

    /** The measurement factory we will use */
    genfit::MeasurementFactory<genfit::AbsMeasurement> m_measurementFactory;

    /** The factory for creating the measurement creators for cdc hits */
    CDCMeasurementCreatorFactory m_cdcMeasurementCreatorFactory;
    /** The factory for creating the measurement creators for svd hits */
    SVDMeasurementCreatorFactory m_svdMeasurementCreatorFactory;
    /** The factory for creating the measurement creators for pxd hits */
    PXDMeasurementCreatorFactory m_pxdMeasurementCreatorFactory;
    /** The factory for creating the measurement creators which are not base on a hit type */
    AdditionalMeasurementCreatorFactory m_additionalMeasurementCreatorFactory;

    /**
     * Helper function to construct the measurements for the added hits of a reco track.
     * @param recoTrack to construct the measurements/track hits for.
     */
    void constructHitsForTrack(RecoTrack& recoTrack) const;
  };
}

