/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  /// Class containing the algorithm to perform the simple circle fit
  class QualityEstimatorCircleFit : public QualityEstimatorBase {

  public:
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;
  };
}

