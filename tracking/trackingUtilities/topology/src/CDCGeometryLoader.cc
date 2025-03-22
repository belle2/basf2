/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/CDCGeometryLoader.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void CDCGeometryLoader::loadDatabase()
{
  // Setup the DataStore and
  // create the default event number to be used for the DB parameter
  DataStore::Instance().reset();
  StoreObjPtr<EventMetaData> evtPtr;
  DataStore::Instance().setInitializeActive(true);
  evtPtr.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  // use experiment 0 for MC-only events
  evtPtr.construct(1, 0, 0);

  Database::reset();
  // load database content for IoV in EventMetaData
  DBStore::Instance().update();

  DBObjPtr<CDCGeometry> cdcGeometry;
  if (!cdcGeometry) {
    B2FATAL("No CDC configuration can be loaded!");
  } else {
    B2INFO("CDC Geometry loaded from DB");
  }

  CDC::CDCGeometryPar::Instance(&(*cdcGeometry));
}

void CDCGeometryLoader::closeDatabase()
{
  Database::reset();
  DataStore::Instance().reset();
}
