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

#include <genfit/MeasurementFactory.h>
#include <vector>
#include <framework/logging/Logger.h>

namespace genfit {
  class AbsMeasurement;
}

namespace Belle2 {
  class RecoTrack;
  class RecoHitInformation;

  class BaseMeasurementCreator {
  public:
    /** Creates a MeasurementCreator which handles the creation of measurements of a given kind **/
    BaseMeasurementCreator() {}

    /** Destructor **/
    virtual ~BaseMeasurementCreator() { }

    /** Overload this method to create measurements **/
    virtual std::vector<genfit::AbsMeasurement*> createMeasurements(const RecoTrack& recoTrack,
        const RecoHitInformation& recoHitInformation) const = 0;

    /** Use this to set the parameters of the creator. Does nothing per default. */
    virtual void setParameter(const std::string& parameterName, const std::string& parameterValue)
    {
      B2FATAL("A parameter with the name " << parameterName << " and the value " << parameterValue << " could not be set.")
    }
  };
}
