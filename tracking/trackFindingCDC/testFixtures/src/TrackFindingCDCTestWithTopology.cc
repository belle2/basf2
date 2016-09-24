/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;


void TrackFindingCDCTestWithTopology::SetUpTestCase()
{
  //Setup the gearbox
  TestHelpers::TestWithGearbox::SetUpTestCase();

  //Also preload the CDCGeometry
  const CDCWireTopology& wireTopology __attribute__((unused)) = CDCWireTopology::getInstance();

  CDCWireHitTopology::initialize();
}

void TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  //Close the gearbox
  TestHelpers::TestWithGearbox::TearDownTestCase();
}



template<>
bool Belle2::TestHelpers::allNear<Vector3D>(const Vector3D& expected,
                                            const Vector3D& actual,
                                            double tolerance)
{
  bool xNear = std::fabs(expected.x() - actual.x()) < tolerance;
  bool yNear = std::fabs(expected.y() - actual.y()) < tolerance;
  bool zNear = std::fabs(expected.z() - actual.z()) < tolerance;
  return xNear and yNear and zNear;
}
