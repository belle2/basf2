/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Matrix.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// The representation type of a covariance matrix of n related parameters
    template <int N>
    using CovarianceMatrix = Matrix<double, N, N>;

  }
}
