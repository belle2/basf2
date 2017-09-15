/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <Eigen/Core>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// The representation type of a covariance matrix of n related parameters
    template <int N>
    using CovarianceMatrix = Eigen::Matrix<double, N, N>;

  }
}
