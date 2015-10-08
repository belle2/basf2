/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCRecoSegment2D;
    class CDCTrack;

    class SegmentsProcessor {

    public:

      SegmentsProcessor();

      ~SegmentsProcessor();

      static std::vector<CDCRecoSegment2D> getSegmentsOfTheTrack(CDCTrack& track);

      static void reassignSegments(CDCTrack& track1, CDCTrack& track2);

    };

  }

}

