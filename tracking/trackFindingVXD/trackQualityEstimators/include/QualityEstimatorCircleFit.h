/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  class QualityEstimatorCircleFit : public QualityEstimatorBase {

  public:

    QualityEstimatorCircleFit(double magneticFieldZ): QualityEstimatorBase(magneticFieldZ) {};

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;
  };
}

