/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
