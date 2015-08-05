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

#include <tracking/measurementCreator/MeasurementCreatorFactory.h>
#include <tracking/measurementCreator/creators/CoordinateMeasurementCreator.h>
#include <tracking/measurementCreator/creators/VXDMomentumEstimationMeasurementCreator.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  class PXDMeasurementCreatorFactory : public
    MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedPXDHit, Const::PXD>> {

  public:
    PXDMeasurementCreatorFactory(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      MeasurementCreatorFactory<BaseMeasurementCreatorFromHit<RecoTrack::UsedPXDHit, Const::PXD>>(),
          m_measurementFactory(measurementFactory) {}

    /** Only a simple reco hit creator is implemented in the moment */
    BaseMeasurementCreatorFromHit<RecoTrack::UsedPXDHit, Const::PXD>* createMeasurementCreatorFromName(
      const std::string& creatorName) const
    {
      if (creatorName == "RecoHitCreator") {
        return new CoordinateMeasurementCreator<RecoTrack::UsedPXDHit, Const::PXD>(m_measurementFactory);
      } else if (creatorName == "MomentumEstimationCreator") {
        return new VXDMomentumEstimationMeasurementCreator<RecoTrack::UsedPXDHit, Const::PXD>(m_measurementFactory);
      }

      return nullptr;
    }
  private:
    /** A reference to the prefilled measurement factory */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;
  };
}
