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
}

void GeometryModule::initialize()
{
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.setDetectorComponents(m_components);
  geoManager.setExcludedComponents(m_excluded);
  geoManager.setAdditionalComponents(m_additional);
  geoManager.setAssignRegions(m_assignRegions);

  geoManager.createGeometry(GearDir(m_geometryPath), geometry::FullGeometry);
}
