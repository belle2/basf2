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
#include <framework/database/IntervalOfValidity.h>
#include <geometry/GeometryManager.h>
#include <geometry/Materials.h>
#include <geometry/CreatorManager.h>
#include <geometry/CreatorBase.h>
#include <geometry/utilities.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/geometry/BFieldManager.h>
#include <geometry/bfieldmap/BFieldFrameworkInterface.h>
#include <geometry/dbobjects/GeoConfiguration.h>

#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4RunManager.hh"
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
#include "VGM/volumes/IPlacement.h"
#include "TGeoManager.h"

#include <memory>

using namespace std;

namespace Belle2 {
  using namespace gearbox;

  namespace geometry {

    GeometryManager& GeometryManager::getInstance()
    {
      static unique_ptr<GeometryManager> instance(new GeometryManager());
      return *instance;
    }

    void GeometryManager::clear()
    {
      B2DEBUG(50, "Cleaning up Geometry");
      for (G4VisAttributes* visAttr : m_VisAttributes) delete visAttr;
      m_VisAttributes.clear();
      for (CreatorBase* creator : m_creators) delete creator;
      m_creators.clear();
      m_topVolume = 0;
      // empty magnetic field
      BFieldManager::getInstance().clearComponents();
      //Clean up existing Geometry
      G4GeometryManager::GetInstance()->OpenGeometry();
      G4PhysicalVolumeStore::Clean();
      G4LogicalVolumeStore::Clean();
      G4SolidStore::Clean();
      G4LogicalBorderSurface::CleanSurfaceTable();
      G4LogicalSkinSurface::CleanSurfaceTable();
      G4SurfaceProperty::CleanSurfacePropertyTable();
      //And finally clean up materials
      Materials::getInstance().clear();
    }

    GeoConfiguration GeometryManager::createGeometryConfig(const GearDir& detectorDir, const IntervalOfValidity& iov)
    {
      std::string detectorName;
      try {
        detectorName = detectorDir.getString("Name");
        B2INFO("Creating geometry for detector: " << detectorName);
      } catch (gearbox::PathEmptyError& e) {
        B2FATAL("Could not read detector name, make sure gearbox is connected and "
                << detectorDir.getPath() << " points to the geometry description");
      }

      const double width  = detectorDir.getLength("Global/width",  8 * Unit::m);
      const double height = detectorDir.getLength("Global/height", 8 * Unit::m);
      const double length = detectorDir.getLength("Global/length", 8 * Unit::m);
      const std::string material = detectorDir.getString("Global/material", "Air");

      GeoConfiguration config(detectorName, width, height, length, material);

      // Add materials
      Materials& materials = Materials::getInstance();
      for (const GearDir& matlist : detectorDir.getNodes("Materials")) {
        for (const GearDir& mat : matlist.getNodes("Material")) {
          GeoMaterial material = materials.createMaterialConfig(mat);
          config.addMaterial(material);
        }
      }

      std::set<std::string> componentNames = m_components;
      std::set<std::string> excludedNames = m_excluded;
      std::set<std::string> additionalNames = m_additional;
      if (!m_components.empty() && !m_additional.empty()) {
        B2WARNING("Additional components are ignored when a list of components is provided.")
      }

      //Now create all subcomponents
      for (const GearDir& component : detectorDir.getNodes("DetectorComponent")) {
        string name;
        string creatorName;
        try {
          name        = component.getString("@name");
          creatorName = component.getString("Creator");
        } catch (gearbox::PathEmptyError& e) {
          B2ERROR("Could not find required element Name or Creator for " << component.getPath());
          continue;
        }
        const bool isDefault = component.getBool("@isDefault", true);
        //Remove this name from the list of selected, excluded or additional components. At
        //the end there should be nothing left in these lists
        componentNames.erase(name);
        excludedNames.erase(name);
        additionalNames.erase(name);
        if (!m_components.empty() && m_components.count(name) == 0) {
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
        if (!m_excluded.empty() && m_excluded.count(name) > 0) {
          B2INFO("DetectorComponent " << name << " in list of excluded components, skipping");
          continue;
        }

        string libraryName = component.getString("Creator/@library", "");
        if (!iov.empty()) {
          CreatorBase* creator = CreatorManager::getCreator(creatorName, libraryName);
          if (creator) {
            creator->createPayloads(GearDir(component, "Content"), iov);
          } else {
            B2ERROR("Could not load creator");
          }
        }
        config.addComponent({name, creatorName, libraryName});
      }

      //If there are still names left in the componentNames, excludedNames or
      //additionalNames there is probably an typo in the respective component
      //list. Throw an error for each name left using a small lambda function
      auto checkRemaining = [](const std::string & type, const std::set<std::string> componentNames) {
        for (const std::string& name : componentNames) {
          B2ERROR("'" << name << "' is specified in list of "
                  << type << " but could not be found");
        }
      };

      checkRemaining("components", componentNames);
      checkRemaining("excluded components", excludedNames);
      checkRemaining("additional components", additionalNames);

      return config;
    }

    void GeometryManager::createGeometry(const GearDir& detectorDir, GeometryTypes type)
    {
      GeoConfiguration config = createGeometryConfig(detectorDir, IntervalOfValidity());
      createGeometry(config, type, false);
    }

    void GeometryManager::createGeometry(const GeoConfiguration& config, GeometryTypes type, bool useDB)
    {
      // remove the old geometry
      clear();

      //Let Geant4 know that we "modified" the geometry
      G4RunManager* runManager = G4RunManager::GetRunManager();
      if (runManager) runManager->ReinitializeGeometry(true, true);

      // create new geometry
      B2INFO("Creating geometry for detector: " << config.getName());

      // create Materials
      Materials& materials = Materials::getInstance();
      for (const GeoMaterial& mat : config.getMaterials()) {
        materials.createMaterial(mat);
      }

      //Interface the magnetic field
      BFieldManager::getInstance().addComponent(new BFieldFrameworkInterface());

      //Now set Top volume
      G4Material*      top_mat = Materials::get(config.getGlobalMaterial());
      G4Box*           top_box = new G4Box("Top", config.getGlobalWidth() / Unit::cm * CLHEP::cm,
                                           config.getGlobalHeight() / Unit::cm * CLHEP::cm,
                                           config.getGlobalLength() / Unit::cm * CLHEP::cm);
      G4LogicalVolume* top_log = new G4LogicalVolume(top_box, top_mat, "Top", 0, 0, 0);
      setVisibility(*top_log, false);
      m_topVolume = new G4PVPlacement(0, G4ThreeVector(), top_log, "Top", 0, false, 0);
      B2INFO("Created top volume with x= +-" << config.getGlobalWidth() << " cm, y= +-"
             << config.getGlobalHeight() << " cm, z= +-" << config.getGlobalLength() << " cm");

      for (const GeoComponent& component : config.getComponents()) {
        CreatorBase* creator = CreatorManager::getCreator(component.getCreator(), component.getLibrary());
        if (creator) {
          int oldSolids = G4SolidStore::GetInstance()->size();
          int oldLogical = G4LogicalVolumeStore::GetInstance()->size();
          int oldPhysical = G4PhysicalVolumeStore::GetInstance()->size();
          try {
            if (!useDB) throw CreatorBase::DBNotImplemented();
            creator->createFromDB(component.getName(), *top_log, type);
            B2INFO("called creator " << component.getCreator() << " to create component " << component.getName() << " from DB");

          } catch (CreatorBase::DBNotImplemented& e) {
            GearDir parameters = Gearbox::getInstance().getDetectorComponent(component.getName());
            creator->create(parameters, *top_log, type);
            B2INFO("called creator " << component.getCreator() << " to create component " << component.getName() << " from Gearbox");
          }
          int newSolids = G4SolidStore::GetInstance()->size() - oldSolids;
          int newLogical = G4LogicalVolumeStore::GetInstance()->size() - oldLogical;
          int newPhysical = G4PhysicalVolumeStore::GetInstance()->size() - oldPhysical;
          B2INFO("DetectorComponent " << component.getName() << " created " << newSolids
                 << " solids, " << newLogical << " logical volumes and "
                 << newPhysical << " physical volumes");
          m_creators.push_back(creator);
          if (m_assignRegions) {
            //Automatically assign a region with the creator name to all volumes
            G4Region* region {nullptr};
            //We loop over all children of the top volume and check whether they
            //have the correct region assigned
            for (int i = 0; i < top_log->GetNoDaughters(); ++i) {
              G4LogicalVolume* vol = top_log->GetDaughter(i)->GetLogicalVolume();
              //We only assign a region if there is not already one
              if (!vol->GetRegion()) {
                //Ok, new region, create or get one if not already done and assign it
                if (!region) region = G4RegionStore::GetInstance()->FindOrCreateRegion(component.getName());
                vol->SetRegion(region);
                //And propagate the region to all child volumes
                region->AddRootLogicalVolume(vol);
              }
            }
          }
        }
      }

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
      B2INFO("Optimizing geometry and creating lookup tables ...")
      B2INFO_MEASURE_TIME(
        "Optimizing geometry took ",
        G4GeometryManager::GetInstance()->CloseGeometry(true, LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME()));
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
      delete g4Factory.Top();
      delete rtFactory.Top();
    }
  }
} //Belle2 namespace
