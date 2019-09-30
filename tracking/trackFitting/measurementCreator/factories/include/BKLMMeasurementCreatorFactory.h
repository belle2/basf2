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

#include <tracking/trackFitting/measurementCreator/factories/MeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/creators/CoordinateMeasurementCreator.h>

namespace Belle2 {
  /** Add all measurement creators related to BKLM hits. */
  class BKLMMeasurementCreatorFactory : public
    MeasurementCreatorFactory<BKLMBaseMeasurementCreator> {

  public:
    /** Initialize with a measurement factory. */
    explicit BKLMMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<BKLMBaseMeasurementCreator>(),
      m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator is implemented in the moment. */
    BKLMBaseMeasurementCreator* createMeasurementCreatorFromName(
      const std::string& creatorName) const override
    {
      if (creatorName == "RecoHitCreator") {
        return new BKLMCoordinateMeasurementCreator(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory. */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
