/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Jonas Wagner                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <TRandom.h>

namespace Belle2 {
  /// Class implementing a random quality estimation
  class QualityEstimatorRandom : public QualityEstimatorBase {

  public:
    /** Returning a random quality indicator for the given track candidate
     * Value is uniformly distributed between 0.0 and 1.0.
     * @return : random quality indicator value
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const&) final { return gRandom->Uniform(1.0); };
  };
}
