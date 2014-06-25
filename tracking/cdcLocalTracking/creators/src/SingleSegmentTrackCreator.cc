/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/SingleSegmentTrackCreator.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


SingleSegmentTrackCreator::SingleSegmentTrackCreator()
{
}

SingleSegmentTrackCreator::~SingleSegmentTrackCreator()
{
}


void
SingleSegmentTrackCreator::append(
  const std::vector<CDCRecoSegment2D>& recoSegments,
  std::vector<CDCTrack>& tracks)
const
{

  // Add all left over segments of the first superlayer
  // dummy improve to get a higher score,
  // but probably not the best in terms of fake segements
  // plus no z information from the

  BOOST_FOREACH(const CDCRecoSegment2D & segment, recoSegments) {
    if (segment.getISuperLayer() == 0 and
        not segment.getAutomatonCell().hasDoNotUseFlag() and
        segment.size() >= 8) {

      tracks.push_back(CDCTrack());
      bool created = m_trackCreator.create(segment, tracks.back());
      if (not created) {
        tracks.pop_back();
      } else {
        segment.getAutomatonCell().setDoNotUseFlag();
      }
    }

  }

}




