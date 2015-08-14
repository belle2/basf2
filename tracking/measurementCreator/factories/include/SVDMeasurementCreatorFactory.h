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

#include <tracking/measurementCreator/factories/MeasurementCreatorFactory.h>
#include <tracking/measurementCreator/creators/CoordinateMeasurementCreator.h>
#include <tracking/measurementCreator/creators/VXDMomentumEstimationMeasurementCreator.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  class SVDMeasurementCreatorFactory : public
    MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedSVDHit, Const::SVD>> {

  public:
    /** Initialize with a measurement factory */
    explicit SVDMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedSVDHit, Const::SVD>>(),
          m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator and the momentum estimation is implemented in the moment */
    BaseMeasurementCreatorFromHit<RecoTrack::UsedSVDHit, Const::SVD>* createMeasurementCreatorFromName(
      const std::string& creatorName) const
    {
      if (creatorName == "RecoHitCreator") {
        return new CoordinateMeasurementCreator<RecoTrack::UsedSVDHit, Const::SVD>(m_measurementFactory);
      } else if (creatorName == "MomentumEstimationCreator") {
        return new VXDMomentumEstimationMeasurementCreator<RecoTrack::UsedSVDHit, Const::SVD>(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
