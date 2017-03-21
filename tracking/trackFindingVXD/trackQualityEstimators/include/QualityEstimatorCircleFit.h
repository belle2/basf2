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
#include <TVector3.h>

namespace Belle2 {
  class QualityEstimatorCircleFit : QualityEstimatorBase {

  public:

    QualityEstimatorCircleFit(double bFieldValue): QualityEstimatorBase(bFieldValue) {};

    virtual float calcChiSquared(std::vector<Measurement> const& measurements) final;
  };
}

