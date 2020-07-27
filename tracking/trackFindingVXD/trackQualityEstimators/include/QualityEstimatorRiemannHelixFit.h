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

  /** Based on R. Fruehwirth, A. Strandlie, W. Waltenberger,
   * Nuclear instruments and Methods in Physics Research A 490 (2002) 366-378
   */
  class QualityEstimatorRiemannHelixFit : public QualityEstimatorBase {

  public:

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

  protected:
  };
}

