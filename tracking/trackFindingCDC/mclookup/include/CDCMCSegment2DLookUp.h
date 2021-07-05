/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

      /// Default constructor, needs to be public for initialization in CDCMCManager
      CDCMCSegment2DLookUp() = default;
      /// Singleton: Delete copy constructor and assignment operator,
      CDCMCSegment2DLookUp(CDCMCSegment2DLookUp&) = delete;
      CDCMCSegment2DLookUp& operator=(const CDCMCSegment2DLookUp&) = delete;
    };
  }
}
