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
  /** Add all measurement creators related to PXD hits. */
  class PXDMeasurementCreatorFactory : public
    MeasurementCreatorFactory<PXDBaseMeasurementCreator> {

  public:
    /** Initialize with a measurement factory. */
    explicit PXDMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<PXDBaseMeasurementCreator>(),
      m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator and the momentum estimation is implemented in the moment. */
    PXDBaseMeasurementCreator* createMeasurementCreatorFromName(
      const std::string& creatorName) const override
    {
      if (creatorName == "RecoHitCreator") {
        return new PXDCoordinateMeasurementCreator(m_measurementFactory);
      } else if (creatorName == "MomentumEstimationCreator") {
        return new PXDMomentumMeasurementCreator(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory. */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
