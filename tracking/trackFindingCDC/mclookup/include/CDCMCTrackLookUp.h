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
    class CDCTrack;

    extern template class CDCMCHitCollectionLookUp<CDCTrack>;

    /// Specialisation of the lookup for the truth values of reconstructed tracks.
    class CDCMCTrackLookUp  : public CDCMCHitCollectionLookUp<CDCTrack> {

    public:
      /// Getter for the singletone instance
      static const CDCMCTrackLookUp& getInstance();

      /// Default constructor, needs to be public for initialization in CDCMCManager
      CDCMCTrackLookUp() = default;
      /// Singleton: Delete copy constructor and assignment operator
      CDCMCTrackLookUp(CDCMCTrackLookUp&) = delete;
      CDCMCTrackLookUp& operator=(const CDCMCTrackLookUp&) = delete;

    };

  }
}
