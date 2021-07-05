/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
