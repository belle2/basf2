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

#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>

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
  // Setup the DataStore and
  // create the default event number to be used for the DB parameter
  DataStore::Instance().reset();
  StoreObjPtr<EventMetaData> evtPtr;
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  evtPtr.construct(0, 0, 1);

  // Setup the Database
  Database::reset();

  // Load the default database including the remote db
  // Database::Instance();

  // Load only the local database
  std::string dbFilePath = FileSystem::findFile("data/framework/database.txt");
  LocalDatabase::createInstance(dbFilePath, "", true, LogConfig::c_Error);
  DBStore::Instance().update();

  // DBObjPtr<BeamParameters> dbBeamParameters;
  // B2ASSERT("Beamparameters in the database", dbBeamParameters);
  // B2INFO("Successfully found beamparameters with energy "
  //   << dbBeamParameters->getEnergy());

  // CDCGeometry not uploaded yet - maybe useful later
  // DBObjPtr<CDCGeometry> dbGeometry;
  // if (not dbGeometry) {
  //   B2FATAL("No configuration for CDC found.");
  // }
  // CDCGeometry& cdcGeometry = *dbGeometry;

  // Setup the gearbox
  TestHelpers::TestWithGearbox::SetUpTestCase();
  GearDir cdcGearDir = Gearbox::getInstance().getDetectorComponent("CDC");

  // Unpack the cdc geometry, which currently uses both the db and the gearbox
  CDCGeometry cdcGeometry;
  cdcGeometry.read(cdcGearDir);
  CDC::CDCGeometryPar::Instance(&cdcGeometry);

  // Prepare the wires for the cdc track finders.
  CDCWireTopology::getInstance();
}

void TrackFindingCDCTestWithTopology::TearDownTestCase()
{
  // Close the gearbox and reset the global objects
  TestHelpers::TestWithGearbox::TearDownTestCase();
  Database::reset();
  DataStore::Instance().reset();
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
