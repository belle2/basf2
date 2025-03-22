/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackingUtilities/testFixtures/TrackingUtilitiesTestWithTopology.h>

#include <tracking/trackingUtilities/topology/CDCWireTopology.h>
#include <tracking/trackingUtilities/topology/CDCGeometryLoader.h>

#include <cmath>

using namespace Belle2;
using namespace TrackingUtilities;


void TrackingUtilitiesTestWithTopology::SetUpTestCase()
{
  /// Manually open the database and prepare the cdc geometry parameters.
  CDCGeometryLoader::loadDatabase();

  // Prepare the wires for the cdc track finders.
  CDCWireTopology::getInstance();
}

void TrackingUtilitiesTestWithTopology::TearDownTestCase()
{
  CDCGeometryLoader::closeDatabase();
}



template<>
bool Belle2::TestHelpers::allNear<TrackingUtilities::Vector3D>(const TrackingUtilities::Vector3D& expected,
    const TrackingUtilities::Vector3D& actual,
    double tolerance)
{
  bool xNear = std::fabs(expected.x() - actual.x()) < tolerance;
  bool yNear = std::fabs(expected.y() - actual.y()) < tolerance;
  bool zNear = std::fabs(expected.z() - actual.z()) < tolerance;
  return xNear and yNear and zNear;
}
