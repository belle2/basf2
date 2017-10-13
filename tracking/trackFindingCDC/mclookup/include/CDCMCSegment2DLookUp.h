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

#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    extern template class CDCMCHitCollectionLookUp<CDCSegment2D>;

    /// Specialisation of the lookup for the truth values of two dimensional segments.
    class CDCMCSegment2DLookUp : public CDCMCHitCollectionLookUp<CDCSegment2D> {

    public:
      /// Getter for the singletone instance
      static const CDCMCSegment2DLookUp& getInstance();
    };
  }
}
