/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/relations/LayerRelationFilter.dcl.h>
#include <tracking/vxdHoughTracking/filters/relations/RelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    /// A chooseable filter for picking out the relations between states
    using ChooseableRelationFilter = LayerRelationFilter<TrackFindingCDC::ChooseableFilter<RelationFilterFactory>>;
  }
}
