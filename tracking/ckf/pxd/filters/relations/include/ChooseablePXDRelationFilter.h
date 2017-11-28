/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/relations/LayerPXDRelationFilter.dcl.h>
#include <tracking/ckf/pxd/filters/relations/PXDPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  extern template class LayerPXDRelationFilter<TrackFindingCDC::ChooseableFilter<PXDPairFilterFactory>>;
  using ChooseablePXDRelationFilter = LayerPXDRelationFilter<TrackFindingCDC::ChooseableFilter<PXDPairFilterFactory>>;
}