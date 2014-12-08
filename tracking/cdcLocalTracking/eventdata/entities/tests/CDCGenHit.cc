/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>

#include <tracking/cdcLocalTracking/test_fixtures/CDCLocalTrackingTestWithTopology.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

// Tests if CDCGenHitVector can handle CDCWireHits
TEST_F(CDCLocalTrackingTestWithTopology, eventdata_entities_CDCGenHit_totalOrdering)
{
  CDCGenHit lowestGenHit;
  CDCGenHit secondLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 0.0));
  CDCGenHit thirdLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 1.0));

  ASSERT_LT(lowestGenHit, secondLowestGenHit);
  ASSERT_LT(secondLowestGenHit, thirdLowestGenHit);
  ASSERT_LT(lowestGenHit, thirdLowestGenHit);

}





