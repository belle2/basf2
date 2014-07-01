/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/utilities/Utils.h>
#include <geometry/GeometryManager.h>
#include <geometry/Materials.h>
#include <geometry/CreatorManager.h>
#include <geometry/CreatorBase.h>
#include <geometry/utilities.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4RegionStore.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

//VGM stuff
#include "Geant4GM/volumes/Factory.h"
#include "RootGM/volumes/Factory.h"
#include "TGeoManager.h"

#include <memory>

using namespace std;

namespace Belle2 {
  using namespace gearbox;

  namespace geometry {

    GeometryManager& GeometryManager::getInstance()
    {
      static auto_ptr<GeometryManager> instance(new GeometryManager());
      return *instance;
    }

    void GeometryManager::clear()
    {
      for (CreatorBase * creator : m_creators) delete creator;
      m_creators.clear();
      m_topVolume = 0;
      //FIXME: Geometry is now run independent, don't delete anything, let Geant4 care about freeing stuff
      return;
      //Clean up existing Geometry
      G4GeometryManager::GetInstance()->OpenGeometry();
      G4PhysicalVolumeStore::Clean();
      G4LogicalVolumeStore::Clean();
      G4SolidStore::Clean();
      G4LogicalBorderSurface::CleanSurfaceTable();
      G4LogicalSkinSurface::CleanSurfaceTable();
      //FIXME: The MaterialPropertyTables associated with the surfaces won't get deleted.
      G4SurfaceProperty::CleanSurfacePropertyTable();
      for (CreatorBase * creator : m_creators) delete creator;
      m_creators.clear();
      m_topVolume = 0;
    }

    void GeometryManager::createGeometry(const GearDir& detectorDir, GeometryTypes type)
    {
      clear();
      B2INFO("Creating Geometry");

      try {
        string detectorName = detectorDir.getString("Name");
        B2INFO("Creating geometry for detector: " << detectorName);
      } catch (gearbox::PathEmptyError e) {
        B2FATAL("Could not read detector name, make sure gearbox is connected and "
                << detectorDir.getPath() << " points to the geometry description");
      }

      Materials& materials = Materials::getInstance();
      //Set up Materials first since we possibly need them for the top volume
      for (const GearDir & matlist : detectorDir.getNodes("Materials")) {
        for (const GearDir & mat : matlist.getNodes("Material")) {
          materials.createMaterial(mat);
        }
      }

      //Now set Top volume
      double width  = detectorDir.getLength("Global/width",  8 * Unit::m) / Unit::mm;
      double height = detectorDir.getLength("Global/height", 8 * Unit::m) / Unit::mm;
      double length = detectorDir.getLength("Global/length", 8 * Unit::m) / Unit::mm;
      string material = detectorDir.getString("Global/material", "Air");
      G4Material*      top_mat = Materials::get(material);
      G4Box*           top_box = new G4Box("Top", width, height, length);
      G4LogicalVolume* top_log = new G4LogicalVolume(top_box, top_mat, "Top", 0, 0, 0);
      setVisibility(*top_log, false);
      m_topVolume = new G4PVPlacement(0, G4ThreeVector(), top_log, "Top", 0, false, 0);
      //Make a copy of the names for all selected or excluded components to
      //check if all of those names are actually known components
      std::set<std::string> componentNames = m_components;
      std::set<std::string> excludedNames = m_excluded;
      std::set<std::string> additionalNames = m_additional;
      if (!m_components.empty() && !m_additional.empty()) {
        B2WARNING("Additional components are ignored when a list of components is provided.")
      }

      //Now create all subcomponents
      for (const GearDir & component : detectorDir.getNodes("DetectorComponent")) {
        string name;
        string creatorName;
        try {
          name        = component.getString("@name");
          creatorName = component.getString("Creator");
        } catch (gearbox::PathEmptyError e) {
          B2ERROR("Could not find required element Name or Creator for " << component.getPath());
          continue;
        }
        const bool isDefault = component.getBool("@isDefault", true);
        //Remove this name from the list of selected, excluded or additional components. At
        //the end there should be nothing left in these lists
        componentNames.erase(name);
        excludedNames.erase(name);
        additionalNames.erase(name);
        if (m_components.size() > 0 && m_components.count(name) == 0) {
          B2INFO("DetectorComponent " << name << " not in list of components, skipping");
          continue;
        }
        if (m_components.empty() && !isDefault) {
          if (m_additional.count(name) == 0) {
            B2INFO("DectorComponent " << name << " is not enabled by default, skipping");
            continue;
          } else {
            B2INFO("DectorComponent " << name << " is enabled in addition to the default components");
          }
        }
        if (m_excluded.size() > 0 && m_excluded.count(name) > 0) {
          B2INFO("DetectorComponent " << name << " in list of excluded components, skipping");
          continue;
        }

        string libraryName = component.getString("Creator/@library", "");

        CreatorBase* creator = CreatorManager::getCreator(creatorName, libraryName);
        if (creator) {
          int oldSolids = G4SolidStore::GetInstance()->size();
          int oldLogical = G4LogicalVolumeStore::GetInstance()->size();
          int oldPhysical = G4PhysicalVolumeStore::GetInstance()->size();
          B2INFO_MEASURE_TIME(
            "Calling creator " << creatorName << " to create component " << name << " took ",
            creator->create(GearDir(component, "Content"), *top_log, type)
          );
          int newSolids = G4SolidStore::GetInstance()->size() - oldSolids;
          int newLogical = G4LogicalVolumeStore::GetInstance()->size() - oldLogical;
          int newPhysical = G4PhysicalVolumeStore::GetInstance()->size() - oldPhysical;
          B2INFO("DetectorComponent " << name << " created " << newSolids
                 << " solids, " << newLogical << " logical volumes and "
                 << newPhysical << " physical volumes");
          m_creators.push_back(creator);
        }
      }

      //If there are still names left in the componentNames, excludedNames or
      //additionalNames there is probably an typo in the respective component
      //list. Throw an error for each name left using a small lambda function
      auto checkRemaining = [](const std::string & type, const std::set<std::string> componentNames) {
        for (const std::string & name : componentNames) {
          B2ERROR("'" << name << "' is specified in list of "
                  << type << " but could not be found");
        }
      };

      checkRemaining("components", componentNames);
      checkRemaining("excluded components", excludedNames);
      checkRemaining("additional components", additionalNames);

      int newSolids = G4SolidStore::GetInstance()->size();
      int newLogical = G4LogicalVolumeStore::GetInstance()->size();
      int newPhysical = G4PhysicalVolumeStore::GetInstance()->size();
      B2INFO("Created a total of " << newSolids << " solids, " << newLogical
             << " logical volumes and " << newPhysical << " physical volumes");

      B2INFO("Initializing magnetic field if present ...");
      B2INFO_MEASURE_TIME(
        "Initialization of magnetic field took ",
        BFieldMap::Instance().getBField(TVector3(0, 0, 0));
      );
    }

    void GeometryManager::createTGeoRepresentation()
    {
      if (!m_topVolume) {
        B2ERROR("No Geometry found, please create a geometry before converting it to ROOT::TGeo");
        return;
      }

      Geant4GM::Factory g4Factory;
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) {
        g4Factory.SetDebug(1);
      }
      g4Factory.Import(m_topVolume);
      RootGM::Factory rtFactory;
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) {
        rtFactory.SetDebug(1);
      }
      g4Factory.Export(&rtFactory);
      gGeoManager->CloseGeometry();
    }
  }
} //Belle2 namespace
