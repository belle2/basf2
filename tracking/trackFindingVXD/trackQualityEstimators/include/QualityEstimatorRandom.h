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
  class QualityEstimatorRandom : public QualityEstimatorBase {

  public:

    QualityEstimatorRandom(double magneticFieldZ): QualityEstimatorBase(magneticFieldZ) {};

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final {return gRandom->Uniform(1.0);};
  };
}

