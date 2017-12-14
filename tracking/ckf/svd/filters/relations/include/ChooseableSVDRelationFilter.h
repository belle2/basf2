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

#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.dcl.h>
#include <tracking/ckf/svd/filters/relations/SVDPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  using ChooseableSVDRelationFilter = LayerSVDRelationFilter<TrackFindingCDC::ChooseableFilter<SVDPairFilterFactory>>;
}