/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.dcl.h>
#include <tracking/ckf/svd/filters/relations/SVDPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  using ChooseableSVDRelationFilter = LayerSVDRelationFilter<TrackFindingCDC::ChooseableFilter<SVDPairFilterFactory>>;
}