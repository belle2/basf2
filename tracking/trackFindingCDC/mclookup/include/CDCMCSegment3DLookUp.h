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
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Specialisation of the lookup for the truth values of two dimensional segments.
    class CDCMCSegment3DLookUp : public CDCMCHitCollectionLookUp<CDCRecoSegment3D> {

    public:
      /// Getter for the singletone instance
      static const CDCMCSegment3DLookUp& getInstance();
    };
  }
}
