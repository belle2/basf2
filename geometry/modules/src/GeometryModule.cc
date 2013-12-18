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
  setPropertyFlags(c_InitializeInMain);
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

  //TODO: Deprecated parameters, remove in future
  addParam("Components", m_componentsOld,
           "For Backward compatibility.", m_components);
  addParam("ExcludedComponents", m_excludedOld,
           "For Backward compatibility.", m_excluded);
}

void GeometryModule::initialize()
{
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.setDetectorComponents(m_components);
  geoManager.setExcludedComponents(m_excluded);
  geoManager.setAdditionalComponents(m_additional);

  //TODO: Check deprecated parameters, remove in future
  if (getParam<std::vector<std::string>>("Components").isSetInSteering()) {
    B2WARNING("Parameter 'Components' is deprecated, please use 'components' instead");
    if (getParam<std::vector<std::string>>("components").isSetInSteering()) {
      B2FATAL("Parameters 'Components' and 'components' cannot be given at the same time");
    } else {
      geoManager.setDetectorComponents(m_componentsOld);
    }
  }
  if (getParam<std::vector<std::string>>("ExcludedComponents").isSetInSteering()) {
    B2WARNING("Parameter 'ExcludedComponents' is deprecated, please use 'excludedComponents' instead");
    if (getParam<std::vector<std::string>>("excludedComponents").isSetInSteering()) {
      B2FATAL("Parameters 'ExcludedComponents' and 'excludedComponents' cannot be given at the same time");
    } else {
      geoManager.setExcludedComponents(m_excludedOld);
    }
  }

  geoManager.createGeometry(GearDir(m_geometryPath), geometry::FullGeometry);
}
