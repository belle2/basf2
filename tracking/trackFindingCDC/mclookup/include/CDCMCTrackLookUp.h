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
    class CDCTrack;

    extern template class CDCMCHitCollectionLookUp<CDCTrack>;

    /// Specialisation of the lookup for the truth values of reconstructed tracks.
    class CDCMCTrackLookUp  : public CDCMCHitCollectionLookUp<CDCTrack> {

    public:
      /// Getter for the singletone instance
      static
      const CDCMCTrackLookUp&
      getInstance();

    };

  }
}
