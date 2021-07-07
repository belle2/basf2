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

    /// The representation type of derivatives from n to m parameters as used in the transport of covariance matrices.
    template <int M, int N = M>
    using JacobianMatrix = Matrix<double, M, N>;

  }
}
