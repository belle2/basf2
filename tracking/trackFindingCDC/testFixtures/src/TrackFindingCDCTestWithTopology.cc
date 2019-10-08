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
#include <tracking/trackFindingCDC/topology/CDCGeometryLoader.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;


void TrackFindingCDCTestWithTopology::SetUpTestCase()
{
  /// Manually open the database and prepare the cdc geometry parameters.
  CDCGeometryLoader::loadDatabase();

  // Prepare the wires for the cdc track finders.
  CDCWireTopology::getInstance();
}

void TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  CDCGeometryLoader::closeDatabase();
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
