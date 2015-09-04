/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    // for now a typedef is enough
    // may get additional methods if necessary
    /// A segment consisting of adjacent tangents
    class  CDCFacetSegment : public  CDCSegment<CDCFacet> {
    };

  }
}
