/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentTriple;

    // Guard to prevent repeated instantiations
    extern template class Filter<CDCSegmentTriple>;

    /// Base filter for the constuction of segment triples.
    using BaseSegmentTripleFilter = Filter<CDCSegmentTriple>;
  }
}
