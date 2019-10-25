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
  /** Add all measurement creators related to CDC hits. */
  class CDCMeasurementCreatorFactory : public
    MeasurementCreatorFactory<CDCBaseMeasurementCreator> {

  public:
    /** Initialize with a measurement factory. */
    explicit CDCMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<CDCBaseMeasurementCreator>(),
      m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator is implemented in the moment. */
    CDCBaseMeasurementCreator* createMeasurementCreatorFromName(
      const std::string& creatorName) const override
    {
      if (creatorName == "RecoHitCreator") {
        return new CDCCoordinateMeasurementCreator(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory. */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
