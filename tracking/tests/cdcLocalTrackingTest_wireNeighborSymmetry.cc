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

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CCWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CCWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CCW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(CDCLocalTrackingTest, WireNeighborSymmetry_CW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire & wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}


