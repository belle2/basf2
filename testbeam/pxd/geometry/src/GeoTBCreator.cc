/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/pxd/geometry/GeoTBCreator.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <testbeam/vxd/geometry/SensorInfo.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>

//Sha pes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include <G4Point3D.hh>

#include <G4TessellatedSolid.hh>
#include <G4QuadrangularFacet.hh>
#include <G4TriangularFacet.hh>

//#define MATERIAL_SCAN

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the TB environment */
  namespace PXD {

    /** Register the creator */
    geometry::CreatorFactory<GeoTBCreator> GeoTBFactory("TBCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    GeoTBCreator::GeoTBCreator():
      m_activeChips(false), m_seeNeutrons(false), m_onlyPrimaryTrueHits(false),
      m_sensitiveThreshold(1.0)
    {}

    GeoTBCreator::~GeoTBCreator()
    {
      //Delete all sensitive detectors
      BOOST_FOREACH(PXD::SensitiveDetector * sensitive, m_sensitivePXD) {
        delete sensitive;
      }
      m_sensitivePXD.clear();

      BOOST_FOREACH(TEL::SensitiveDetector * sensitive, m_sensitiveTEL) {
        delete sensitive;
      }
      m_sensitiveTEL.clear();
    }

    G4LogicalVolume* GeoTBCreator::getLogicalVolume(const GearDir& content)
    {
      std::string volName = content.getString("Name");
      //std::string volShape = content.getString("Shape", "Box"); // Not used, keep anyway
      std::string volColor = content.getString("Color", "#000000");
      bool volVisibility = content.getBool("Visible", true);
      double volHalfX = content.getLength("HalfX") / Unit::mm;
      double volHalfY = content.getLength("HalfY") / Unit::mm;
      double volHalfZ = content.getLength("HalfZ") / Unit::mm;
      std::string volMaterial = content.getString("Material");
      G4Material* volG4Material = Materials::get(volMaterial);

      G4Box* volG4Box = new G4Box(string("TB_G4Box_") + volName, volHalfX, volHalfY, volHalfZ);
      G4LogicalVolume* g4vol = new G4LogicalVolume(volG4Box, volG4Material, volName);

      setColor(*g4vol, volColor);
      setVisibility(*g4vol, volVisibility);

      return g4vol;
    }

    G4Transform3D* GeoTBCreator::getTransform(const GearDir& content)
    {
      double volDX = content.getLength("DX") / Unit::mm;
      double volDY = content.getLength("DY") / Unit::mm;
      double volDZ = content.getLength("DZ") / Unit::mm;
      double volPhi = content.getAngle("Phi");
      double volTheta = content.getAngle("Theta");
      double volPsi = content.getAngle("Psi");

      G4RotationMatrix volRotation(volPhi, volTheta, volPsi);
      G4ThreeVector volTranslation(volDX, volDY, volDZ);

      return new G4Transform3D(volRotation, volTranslation);
    }

    void GeoTBCreator::setVolumeActive(const GearDir& content, G4LogicalVolume* volume, const GearDir& parentContent)
    {
      // get size of the volume from parent node GearDir (these are hals-sizes - multiply by 2)
      double aWidth = 2.0 * parentContent.getLength("HalfX") ;// Unit::mm/10.0;
      double aLength = 2.0 * parentContent.getLength("HalfY") ;// Unit::mm/10.0;
      double aHeight = 2.0 * parentContent.getLength("HalfZ") ;// Unit::mm/10.0;

      unsigned short sensorID = content.getInt("SensorID");
      if (sensorID == 0) B2ERROR("Do not use sensorID=0 in your XML, it is incompatible with PXD geometry cache and with the TB Analysis Module.");
      double stepSize = content.getLength("stepSize") ;// Unit::mm;
      string detectorType = content.getString("DetectorType", "");

      if (detectorType == "") B2FATAL("TB: Geometry XML Problem: No DetectorType provided in 'Active' node.");
      if (detectorType == "PXD") {
        if (sensorID == 0) B2FATAL("TB Geometry Creator: sensorID=0 in your XML is incompatible with PXD geometry cache. Use 1-based numbering of sensorID's.");
        VxdID xID(1, 1, sensorID);
        PXD::SensorInfo sensorInfo(xID, aWidth, aLength, aHeight, content.getInt("pixelsR"), content.getInt("pixelsZ[1]"), content.getLength("splitLength", 0), content.getInt("pixelsZ[2]", 0));
        sensorInfo.setDEPFETParams(
          content.getDouble("BulkDoping") / (Unit::um * Unit::um * Unit::um),
          content.getWithUnit("BackVoltage"), // Unit::V,
          content.getWithUnit("TopVoltage"), // Unit::V,
          content.getLength("SourceBorder"),
          content.getLength("ClearBorder"),
          content.getLength("DrainBorder"),
          content.getLength("GateDepth"),
          content.getBool("DoublePixel")
        );
        sensorInfo.setIntegrationWindow(
          content.getTime("IntegrationStart"),
          content.getTime("IntegrationEnd")
        );

        PXD::SensorInfo* newInfo = new PXD::SensorInfo(sensorInfo);
        SensitiveDetector* sensitive = new PXD::SensitiveDetector(newInfo, m_seeNeutrons, m_onlyPrimaryTrueHits, m_sensitiveThreshold);
        m_sensitivePXD.push_back(sensitive);

        volume->SetSensitiveDetector(sensitive);
      }
      if (detectorType == "TEL") {
        TEL::SensorInfo sensorInfo(sensorID, aWidth, aLength, aHeight, content.getInt("pixelsR"), content.getInt("pixelsZ[1]"), content.getLength("splitLength", 0), content.getInt("pixelsZ[2]", 0));

        TEL::SensorInfo* newInfo = new TEL::SensorInfo(sensorInfo);
        TEL::SensitiveDetector* sensitive = new TEL::SensitiveDetector(newInfo, m_seeNeutrons, m_onlyPrimaryTrueHits, m_sensitiveThreshold);
        m_sensitiveTEL.push_back(sensitive);

        volume->SetSensitiveDetector(sensitive);
      }
      // set the stepSize and update volume name
      volume->SetUserLimits(new G4UserLimits(stepSize));
      volume->SetName(volume->GetName() + ".Active");
    }

    void GeoTBCreator::readAddVolumes(const GearDir& content, G4LogicalVolume* parentVolume, int level = 1)
    {
      // loop through all Volume nodes
      BOOST_FOREACH(const GearDir & volume, content.getNodes("Volume")) {
        // use our function to construct G4 volume from GearDir parameters (name, size, material, color, visibility)
        G4LogicalVolume* g4vol = getLogicalVolume(volume);
        // create the physical volume - load transform from params and use same name as for the logical volume
        G4VPhysicalVolume* g4PhysVol = new G4PVPlacement(*getTransform(volume), g4vol, g4vol->GetName(), parentVolume, false, 1);
        // get content of Active node
        GearDir activePar(volume, "Active");
        if (activePar != 0) {
          // if Active node is found, read its params and make it active
          setVolumeActive(activePar, g4vol, volume);
          // only in case we create PXD type sensor (DUT), let the GeoCache to search it through
          //if (activePar.getString("DetectorType") == "PXD") VXD::GeoCache::getInstance().findVolumes(g4PhysVol);
        }
        // check child Volume tags and read them recursively
        GearDir childVolumes(volume, "Volume");
        if (childVolumes != 0) readAddVolumes(volume, g4vol, level + 1);
        if (level == 1) VXD::GeoCache::getInstance().findVolumes(g4PhysVol);
      }
    }

    void GeoTBCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {
      m_activeChips = content.getBool("ActiveChips", false);
      m_seeNeutrons = content.getBool("SeeNeutrons", false);
      m_onlyPrimaryTrueHits = content.getBool("OnlyPrimaryTrueHits", false);
      m_sensitiveThreshold = content.getWithUnit("SensitiveThreshold", 1.0 * Unit::eV);

      GearDir volumes(content, "Volumes");
      readAddVolumes(volumes, &topVolume);
    }

  }
}
