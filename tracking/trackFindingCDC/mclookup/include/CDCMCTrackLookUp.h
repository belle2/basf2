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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCMCTrackLookUp  : public CDCMCHitCollectionLookUp<CDCTrack> {

    public:
      /// Getter for the singletone instance
      static
      const CDCMCTrackLookUp&
      getInstance();

    };

  } // end namespace TrackFindingCDC
} // namespace Belle2
