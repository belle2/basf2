/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilterFactory.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Chooseable<BaseFacetRelationFilter>;
    // extern template class TrackingUtilities::ChooseableFilter<FacetRelationFilterFactory>;
    using ChooseableFacetRelationFilter = TrackingUtilities::ChooseableFilter<FacetRelationFilterFactory>;
  }
}
