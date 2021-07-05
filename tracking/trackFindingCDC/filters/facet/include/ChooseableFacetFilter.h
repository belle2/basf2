/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/FacetFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class Chooseable<BaseFacetFilter>;
    extern template class ChooseableFilter<FacetFilterFactory>;
    using ChooseableFacetFilter = ChooseableFilter<FacetFilterFactory>;
  }
}
