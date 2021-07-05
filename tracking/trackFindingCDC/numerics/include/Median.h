/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Return the median of the given values.
    double median(std::vector<double> values);

    /// Return the weighted median of the given values.
    double weightedMedian(std::vector<WithWeight<double> > weightedValues);
  }
}
