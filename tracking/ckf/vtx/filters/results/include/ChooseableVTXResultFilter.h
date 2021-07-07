/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/results/BaseVTXResultFilter.h>
#include <tracking/ckf/vtx/filters/results/VTXResultFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// Filter to weight the CKF VTX results
  extern template class TrackFindingCDC::Chooseable<BaseVTXResultFilter>;
  /// Filter to weight the VTX clusters
  extern template class TrackFindingCDC::ChooseableFilter<VTXResultFilterFactory>;
  /// Alias for filter to weight the VTX clusters
  using ChooseableVTXResultFilter = TrackFindingCDC::ChooseableFilter<VTXResultFilterFactory>;
}
