/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCGenHit.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

// Tests if CDCGenHitVector can handle CDCWireHits
TEST_F(TrackFindingCDCTestWithTopology, eventdata_hits_CDCGenHit_totalOrdering)
{
  CDCGenHit lowestGenHit;
  CDCGenHit secondLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 0.0));
  CDCGenHit thirdLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 1.0));

  ASSERT_LT(lowestGenHit, secondLowestGenHit);
  ASSERT_LT(secondLowestGenHit, thirdLowestGenHit);
  ASSERT_LT(lowestGenHit, thirdLowestGenHit);

}





