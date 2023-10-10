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


void TrackFindingCDC::TrackFindingCDCTestWithTopology::SetUpTestCase()
{
  /// Manually open the database and prepare the cdc geometry parameters.
  TrackFindingCDC::CDCGeometryLoader::loadDatabase();

  // Prepare the wires for the cdc track finders.
  TrackFindingCDC::CDCWireTopology::getInstance();
}

void TrackFindingCDC::TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  TrackFindingCDC::CDCGeometryLoader::closeDatabase();
}



template<>
bool Belle2::TestHelpers::allNear<TrackFindingCDC::Vector3D>(const TrackFindingCDC::Vector3D& expected,
    const TrackFindingCDC::Vector3D& actual,
    double tolerance)
{
  bool xNear = std::fabs(expected.x() - actual.x()) < tolerance;
  bool yNear = std::fabs(expected.y() - actual.y()) < tolerance;
  bool zNear = std::fabs(expected.z() - actual.z()) < tolerance;
  return xNear and yNear and zNear;
}
