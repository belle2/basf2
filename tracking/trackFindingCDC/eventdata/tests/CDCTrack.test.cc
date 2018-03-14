/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <cdc/dataobjects/WireID.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(TrackFindingCDCTestWithTopology, eventdata_tracks_CDCTrack_modifyable)
{
  CDCWireHit wireHit(WireID(0, 0, 0), 0.01);

  // Create a track and add one hit
  CDCTrack track;
  {
    CDCRLWireHit rlWireHit(&wireHit, ERightLeft::c_Right);
    double perpS = 0;
    CDCRecoHit3D recoHit3D(rlWireHit, wireHit.getRefPos3D(), perpS);
    track.push_back(recoHit3D);
  }
  // Try to iterate over the track
  for (CDCRecoHit3D& recoHit3D : track) {
    recoHit3D.setRecoPos3D(Vector3D(0.0, 0.0, 0.0));
  }

  EXPECT_ALL_NEAR(Vector3D(0.0, 0.0, 0.0), track[0].getRecoPos3D(), 1);
}
