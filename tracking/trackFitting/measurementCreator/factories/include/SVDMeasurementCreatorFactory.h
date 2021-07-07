/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFitting/measurementCreator/factories/MeasurementCreatorFactory.h>
#include <tracking/trackFitting/measurementCreator/creators/CoordinateMeasurementCreator.h>
#include <tracking/trackFitting/measurementCreator/creators/VXDMomentumEstimationMeasurementCreator.h>

namespace Belle2 {
  /** Add all measurement creators related to SVD hits. */
  class SVDMeasurementCreatorFactory : public
    MeasurementCreatorFactory<SVDBaseMeasurementCreator> {

  public:
    /** Initialize with a measurement factory. */
    explicit SVDMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<SVDBaseMeasurementCreator>(),
      m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator and the momentum estimation is implemented in the moment. */
    SVDBaseMeasurementCreator* createMeasurementCreatorFromName(
      const std::string& creatorName) const override
    {
      if (creatorName == "RecoHitCreator") {
        return new SVDCoordinateMeasurementCreator(m_measurementFactory);
      } else if (creatorName == "MomentumEstimationCreator") {
        return new SVDMomentumMeasurementCreator(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory. */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
