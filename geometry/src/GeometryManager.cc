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
#include <geometry/GeometryManager.h>
#include <geometry/Materials.h>
#include <geometry/CreatorManager.h>
#include <geometry/CreatorBase.h>
#include <geometry/utilities.h>

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

#include <boost/foreach.hpp>
#include <memory>
#include <framework/core/utilities.h>

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
      BOOST_FOREACH(CreatorBase * creator, m_creators) delete creator;
      m_creators.clear();
      m_topVolume = 0;
    }

    void GeometryManager::createGeometry(const GearDir& detectorDir, GeometryTypes type)
    {
      clear();

      try {
        string detectorName = detectorDir.getString("Name");
        B2INFO("Creating geometry for detector: " << detectorName);
      } catch (gearbox::PathEmptyError e) {
        B2FATAL("Could not read detector name, make sure gearbox is connected and " << detectorDir.getPath() << " points to the geometry description");
      }

      Materials& materials = Materials::getInstance();
      //Set up Materials first since we possibly need them for the top volume
      BOOST_FOREACH(const GearDir & matlist, detectorDir.getNodes("Materials")) {
        BOOST_FOREACH(const GearDir & mat, matlist.getNodes("Material")) {
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

      //Now create all subcomponents
      BOOST_FOREACH(const GearDir & component, detectorDir.getNodes("DetectorComponent")) {
        string name;
        string creatorName;
        try {
          name        = component.getString("@name");
          creatorName = component.getString("Creator");
        } catch (gearbox::PathEmptyError e) {
          B2ERROR("Could not find required element Name or Creator for " << component.getPath());
          continue;
        }
        if (m_components.size() > 0 && m_components.count(name) == 0) continue;
        if (m_excluded.size() > 0 && m_excluded.count(name) > 0) continue;

        string libraryName = component.getString("Creator/@library", "");

        CreatorBase* creator = CreatorManager::getCreator(creatorName, libraryName);
        if (creator) {
          B2INFO_MEASURE_TIME("Calling creator " << creatorName << " to create component " << name << " took ",
                              creator->create(GearDir(component, "Content"), *top_log, type));
          m_creators.push_back(creator);
        }
      }
    }

    void GeometryManager::createTGeoRepresentation()
    {
      if (!m_topVolume) {
        B2ERROR("No Geometry found, please create a geometry before converting it to ROOT::TGeo");
        return;
      }

      Geant4GM::Factory g4Factory;
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME())) {
        g4Factory.SetDebug(1);
      }
      g4Factory.Import(m_topVolume);
      RootGM::Factory rtFactory;
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME())) {
        rtFactory.SetDebug(1);
      }
      g4Factory.Export(&rtFactory);
      gGeoManager->CloseGeometry();
    }
  }
} //Belle2 namespace
