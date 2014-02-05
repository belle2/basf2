/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include "cdcLocalTrackingTest.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCWireHitVector.h>
#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

// Tests if CDCGenHitVector can handle CDCWireHits
TEST_F(CDCLocalTrackingTest, CDCGenHitVector_WithCDCWireHits)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  const CDCWire& firstWire = theWireTopology.first();

  WireID wireID = WireID(firstWire);

  CDCWireHit wirehit(firstWire, 0.0);

  //CDCGenHitVector<CDCWireHit> collection;
  CDCWireHitVector collection;

  ASSERT_TRUE(collection.empty());

  collection.push_back(wirehit);

  ASSERT_FALSE(collection.empty());

}








