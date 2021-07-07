/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
