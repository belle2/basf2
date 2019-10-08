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

namespace Belle2 {
  /** Add measurement creators that do not rely on a specific hit type,
   * but rather add measurements without corresponding hit. */
  class AdditionalMeasurementCreatorFactory : public
    MeasurementCreatorFactory<BaseMeasurementCreator> {

  public:
    /** Initialize. */
    AdditionalMeasurementCreatorFactory() : MeasurementCreatorFactory<BaseMeasurementCreator>()
    {}

    /** No creator is implemented in the moment. */
    BaseMeasurementCreator* createMeasurementCreatorFromName(const std::string& /*creatorName*/) const override
    {
      return nullptr;
    }
  };
}
