/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/GeometryModule.h>
#include <geometry/GeometryManager.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/DBImportObjPtr.h>

#include <algorithm>
#include <iterator>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Geometry);

GeometryModule::GeometryModule()
{
  setDescription("Setup geometry description");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("geometryPath", m_geometryPath,
           "Path where the parameters for the Geometry can be found", string("/Detector"));
  addParam("geometryType", m_geometryType,
           "Type of geometry to build. Valid values: Full, Tracking, Display", 0);
  addParam("components", m_components,
           "Name of the components to be created. If not empty, all other components found "
           "in the parameter file will be ignored", m_components);
  addParam("excludedComponents", m_excluded,
           "Name of the components to excluded from creation", m_excluded);
  addParam("additionalComponents", m_additional,
           "Name of components to be created in addition to the default parameters", m_additional);
  addParam("assignRegions", m_assignRegions,
           "If true, automatically assign a Geant4 Region with the name of the "
           "creator to all volumes created by that creator", m_assignRegions);
  addParam("ignoreIfPresent", m_ignoreIfPresent,
           "If true this module will silently ignore if the geometry is already "
           "present and do nothing in that case. If false a B2FATAL will be "
           "if the geometry was already created before", m_ignoreIfPresent);
  addParam("useDB", m_useDB, "If true load the Geometry from the database instead of the gearbox", m_useDB);
  addParam("payloadIov", m_payloadIov, "Payload IoV when creating a geometry configuration", m_payloadIov);
  addParam("createPayloads", m_createGeometryPayload, "If true create a "
           "Geometry payload with the given configuration", m_createGeometryPayload);
}

void GeometryModule::initialize()
{
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  if (geoManager.getTopVolume()) {
    if (m_ignoreIfPresent) {
      B2DEBUG(10, "Geometry already created, skipping");
      return;
    } else {
      B2FATAL("Geometry already created, more than one Geometry module present?");
    }
  }

  // filter out components which are not part of the load-able geometry
  vector<string> filteredComponents;
  copy_if(m_components.begin(), m_components.end(),
  std::back_inserter(filteredComponents), [](std::string component) { return component != "TRG"; });

  geoManager.setDetectorComponents(filteredComponents);
  geoManager.setExcludedComponents(m_excluded);
  geoManager.setAdditionalComponents(m_additional);
  geoManager.setAssignRegions(m_assignRegions);

  if (m_createGeometryPayload) {
    B2INFO("Creating Database configuration.");
    if (m_payloadIov.size() != 4) {
      B2ERROR("Geometry: payloadIov must be ecactly 4 values: [first experiment, first run, final experiment, final run]");
      return;
    }
    DBImportObjPtr<GeoConfiguration> import;
    IntervalOfValidity iov(m_payloadIov[0], m_payloadIov[1], m_payloadIov[2], m_payloadIov[3]);
    if (iov.empty()) {
      B2ERROR("Cannot create payloads for an empty iov");
      return;
    }
    import.construct(geoManager.createGeometryConfig(GearDir(m_geometryPath), iov));
    import.import(iov);
    return;
  }

  if (m_useDB) {
    if (getParam<std::string>("geometryPath").isSetInSteering()) {
      B2WARNING("Loading Geometry from Database: parameter 'geometryPath' is ignored");
    }
    for (auto par : {"components", "additionalComponents", "excludedComponents"}) {
      if (getParam<std::vector<std::string>>(par).isSetInSteering()) {
        B2WARNING("Loading Geometry from Database: parameter '" << par << "' is ignored");
      }
    }
    m_geometryConfig = new DBObjPtr<GeoConfiguration>();
    if (!m_geometryConfig->isValid()) {
      B2ERROR("Cannot create Geometry from Database: no configuration found");
      return;
    }
    // Make sure that we abort as soon as the geometry changes
    m_geometryConfig->addCallback([]() {B2FATAL("Geometry cannot change during processing, aborting");});
    geoManager.createGeometry(**m_geometryConfig);
  } else {
    geoManager.createGeometry(GearDir(m_geometryPath), geometry::FullGeometry);
  }
}

void GeometryModule::terminate()
{
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.clear();
  delete m_geometryConfig;
}
