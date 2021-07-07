/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
