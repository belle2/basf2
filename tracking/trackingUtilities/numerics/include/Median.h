/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/numerics/WithWeight.h>
#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {

    /// Return the median of the given values.
    double median(std::vector<double> values);

    /// Return the weighted median of the given values.
    double weightedMedian(std::vector<WithWeight<double> > weightedValues);
  }
}
