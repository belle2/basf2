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
    class CDCSegment3D;

    extern template class CDCMCHitCollectionLookUp<CDCSegment3D>;

    /// Specialisation of the lookup for the truth values of two dimensional segments.
    class CDCMCSegment3DLookUp : public CDCMCHitCollectionLookUp<CDCSegment3D> {

    public:
      /// Getter for the singletone instance
      static const CDCMCSegment3DLookUp& getInstance();

      /// Default constructor, needs to be public for initialization in CDCMCManager
      CDCMCSegment3DLookUp() = default;
      /// Singleton: Delete copy constructor and assignment operator
      CDCMCSegment3DLookUp(CDCMCSegment3DLookUp&) = delete;
      CDCMCSegment3DLookUp& operator=(const CDCMCSegment3DLookUp&) = delete;
    };
  }
}
