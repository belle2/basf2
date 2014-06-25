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

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST_F(CDCLocalTrackingTest, CDCObservations2D_centralize)
{
  CDCObservations2D observations;

  observations.append(Vector2D(0, 0));
  observations.append(Vector2D(1, 2));
  observations.append(Vector2D(2, 4));

  Vector2D centralPoint = observations.centralize();

  EXPECT_EQ(Vector2D(1, 2), centralPoint) <<
                                          "Central point of observation (0,0) , (1,1) and (2,2) is not (1,1)" ;

  EXPECT_EQ(-1, observations.getX(0));
  EXPECT_EQ(-2, observations.getY(0));

  EXPECT_EQ(0, observations.getX(1));
  EXPECT_EQ(0, observations.getY(1));

  EXPECT_EQ(1, observations.getX(2));
  EXPECT_EQ(2, observations.getY(2));
}


