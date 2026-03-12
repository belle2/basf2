/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/results/BasePXDResultFilter.h>
#include <tracking/ckf/pxd/filters/results/PXDResultFilterFactory.h>
#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// Filter to weight the CKF PXD results
  extern template class TrackingUtilities::Chooseable<BasePXDResultFilter>;
  /// Filter to weight the PXD clusters
  extern template class TrackingUtilities::ChooseableFilter<PXDResultFilterFactory>;
  /// Alias for filter to weight the PXD clusters
  using ChooseablePXDResultFilter = TrackingUtilities::ChooseableFilter<PXDResultFilterFactory>;
}
