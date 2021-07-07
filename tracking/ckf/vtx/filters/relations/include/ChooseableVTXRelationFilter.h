/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/relations/LayerVTXRelationFilter.dcl.h>
#include <tracking/ckf/vtx/filters/relations/VTXPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  extern template class LayerVTXRelationFilter<TrackFindingCDC::ChooseableFilter<VTXPairFilterFactory>>;
  /// A chooseable filter for picking out the relations between states
  using ChooseableVTXRelationFilter = LayerVTXRelationFilter<TrackFindingCDC::ChooseableFilter<VTXPairFilterFactory>>;
}
