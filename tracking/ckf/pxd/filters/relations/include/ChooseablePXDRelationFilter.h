/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/relations/LayerPXDRelationFilter.dcl.h>
#include <tracking/ckf/pxd/filters/relations/PXDPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  extern template class LayerPXDRelationFilter<TrackFindingCDC::ChooseableFilter<PXDPairFilterFactory>>;
  /// A chooseable filter for picking out the relations between states
  using ChooseablePXDRelationFilter = LayerPXDRelationFilter<TrackFindingCDC::ChooseableFilter<PXDPairFilterFactory>>;
}
