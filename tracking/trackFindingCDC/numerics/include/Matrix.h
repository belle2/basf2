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

// #include <Eigen/Core>
// namespace Belle2 {
//   namespace TrackFindingCDC {
//     template <class T, int M, int N>
//     using Matrix = Eigen::Matrix<T, M, N>;
//   }
// }

#include <tracking/trackFindingCDC/numerics/PlainMatrix.h>
namespace Belle2 {
  namespace TrackFindingCDC {
    template <class T, int M, int N>
    using Matrix = PlainMatrix<T, M, N>;
  }
}
