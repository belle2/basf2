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

#include <cdc/geometry/CDCGeometryPar.h>

// In case we need to get the parameters from the database here are some hints
// #include <framework/database/Database.h>
// #include <framework/database/DBStore.h>
// #include <framework/dbobjects/BeamParameters.h>
// #include <framework/datastore/StoreObjPtr.h>
// #include <framework/datastore/DataStore.h>
// #include <framework/dataobjects/EventMetaData.h>

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
  // In case we need to get the parameters from the database here are some hints
  // StoreObjPtr<EventMetaData> evtPtr;
  // DataStore::Instance().setInitializeActive(true);
  // evtPtr.registerInDataStore();
  // DataStore::Instance().setInitializeActive(false);
  // evtPtr.construct(0, 0, 1);

  // Database::Instance();
  // DBObjPtr<CDCGeometry> dbGeometry;
  // DBObjPtr<BeamParameters> dbBeamParameters;
  // DBStore::Instance().update();

  // B2ASSERT("Beamparameters in the database", dbBeamParameters);
  // B2INFO("Successfully found beamparameters with energy "
  //   << dbBeamParameters->getEnergy());

  // if (not dbGeometry) {
  //   B2FATAL("No configuration for CDC found.");
  // }

  //Setup the gearbox
  TestHelpers::TestWithGearbox::SetUpTestCase();
  GearDir cdcGearDir = Gearbox::getInstance().getDetectorComponent("CDC");
  CDCGeometry cdcGeometry;
  cdcGeometry.read(cdcGearDir);
  CDC::CDCGeometryPar::Instance(&cdcGeometry);

  const CDCWireTopology& wireTopology __attribute__((unused)) = CDCWireTopology::getInstance();

  CDCWireHitTopology::initialize();
}

void TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  // In case we need to get the parameters from the database here are some hints
  // Database::reset();
  // DataStore::Instance().reset();
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
