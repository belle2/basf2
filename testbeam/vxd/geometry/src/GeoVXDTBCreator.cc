/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/geometry/GeoVXDTBCreator.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>


#include <testbeam/vxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>

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
#include <G4ThreeVector.hh>


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
//TODO: no namespace decided yet

  /** Register the creator */
  geometry::CreatorFactory<GeoTBCreator> GeoTBFactory("VXDTBCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  GeoTBCreator::GeoTBCreator()
  {
  }

  GeoTBCreator::~GeoTBCreator()
  {

    //Delete all sensitive detectors
    BOOST_FOREACH(PXD::SensitiveDetector * sensitive, m_sensitivePXD) {
      delete sensitive;
    }
    m_sensitivePXD.clear();

    BOOST_FOREACH(SVD::SensitiveDetector * sensitive, m_sensitiveSVD) {
      delete sensitive;
    }
    m_sensitiveSVD.clear();

    BOOST_FOREACH(TB::SensitiveDetector * sensitive, m_sensitiveTB) {
      delete sensitive;
    }
    m_sensitiveTB.clear();
  }

  G4LogicalVolume* GeoTBCreator::getLogicalVolume(const GearDir& content)
  {
    std::string volName = content.getString("Name");
    std::string volShape = content.getString("Shape", "Box");
    std::string volColor = content.getString("Color", "#000000");
    bool volVisibility = content.getBool("Visible", true);

    std::string volMaterial = content.getString("Material");
    G4Material* volG4Material = Materials::get(volMaterial);
    G4LogicalVolume* g4vol = 0;

    if (volShape == "Box") {
      double volHalfX = content.getLength("HalfX") / Unit::mm;
      double volHalfY = content.getLength("HalfY") / Unit::mm;
      double volHalfZ = content.getLength("HalfZ") / Unit::mm;
      G4Box* volG4Box = new G4Box(string("TB_G4Box_") + volName, volHalfX, volHalfY, volHalfZ);
      g4vol = new G4LogicalVolume(volG4Box, volG4Material, volName);
    }
    if (volShape == "EmptyBox") {
      double volHalfX = content.getLength("HalfX") / Unit::mm;
      double volHalfY = content.getLength("HalfY") / Unit::mm;
      double volHalfZ = content.getLength("HalfZ") / Unit::mm;
      double volHalfXIn = content.getLength("HalfXIn") / Unit::mm;
      double volHalfYIn = content.getLength("HalfYIn") / Unit::mm;
      double volHalfZIn = content.getLength("HalfZIn") / Unit::mm;
      G4Box* volG4Box = new G4Box(string("TB_G4Box_") + volName, volHalfX, volHalfY, volHalfZ);
      G4Box* volG4BoxIn = new G4Box(string("TB_G4Box_inside_") + volName, volHalfXIn, volHalfYIn, volHalfZIn);
      G4VSolid* subtract = new G4SubtractionSolid("TB_EmptyBox_" + volName, volG4Box, volG4BoxIn);
      g4vol = new G4LogicalVolume(subtract, volG4Material, volName);
    }
    if (volShape == "Tube") {
      double innerRadius = content.getLength("InnerRadius") / Unit::mm;
      double outerRadius = content.getLength("OuterRadius") / Unit::mm;
      double hz = content.getLength("Hz") / Unit::mm;
      double startAngle = content.getAngle("StartAngle", 0);
      double spanningAngle = content.getAngle("SpanningAngle", 360);

      G4Tubs* volG4Tube = new G4Tubs(string("TB_G4Tube_") + volName, innerRadius,
                                     outerRadius,
                                     hz,
                                     startAngle,
                                     spanningAngle);
      g4vol = new G4LogicalVolume(volG4Tube, volG4Material, volName);
    }
    setColor(*g4vol, volColor);
    setVisibility(*g4vol, volVisibility);

    return g4vol;
  }

  G4Transform3D GeoTBCreator::getTransform(const GearDir& content)
  {

    double volDX = content.getLength("DX") / Unit::mm;
    double volDY = content.getLength("DY") / Unit::mm;
    double volDZ = content.getLength("DZ") / Unit::mm;
    double volPhi = content.getAngle("Phi");
    double volTheta = content.getAngle("Theta");
    double volPsi = content.getAngle("Psi");

    G4RotationMatrix volRotation(volPhi, volTheta, volPsi);
    G4ThreeVector volTranslation(volDX, volDY, volDZ);

    G4Transform3D placement = G4Transform3D(volRotation, volTranslation);

    if (content.getNumberNodes("AlignComponent")) {
      string component = content.getString("AlignComponent");
      G4Transform3D align = getAlignment(component);

      placement = placement * align;
    }

    return placement;
  }

  void GeoTBCreator::setVolumeActive(const GearDir& content, G4LogicalVolume* volume, const GearDir& parentContent)
  {
    B2INFO("GeoVXDTBCreator: Setting active volume...")
    // get size of the volume from parent node GearDir (these are half-sizes - multiply by 2)
    double aWidth = 2.0 * parentContent.getLength("HalfX") ;// Unit::mm/10.0;
    double aLength = 2.0 * parentContent.getLength("HalfY") ;// Unit::mm/10.0;
    double aHeight = 2.0 * parentContent.getLength("HalfZ") ;// Unit::mm/10.0;

    unsigned short sensorID = content.getInt("SensorID");
    unsigned short ladder = content.getInt("Ladder");
    unsigned short layer = content.getInt("Layer");
    if (sensorID == 0) B2ERROR("Do not use sensorID=0 in your XML, it is incompatible with PXD geometry cache and with the TB Analysis Module.");
    double stepSize = content.getLength("stepSize") ;// Unit::mm;
    string detectorType = content.getString("DetectorType", "");

    if (detectorType == "") B2FATAL("TB: Geometry XML Problem: No DetectorType provided in 'Active' node.");
    if (detectorType == "PXD") {
      if (sensorID == 0) B2FATAL("TB Geometry Creator: sensorID=0 in your XML is incompatible with PXD geometry cache. Use 1-based numbering of sensorID's.");
      VxdID xID(layer, ladder, sensorID);
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
      PXD::SensitiveDetector* sensitive = new PXD::SensitiveDetector(newInfo, m_seeNeutrons, m_onlyPrimaryTrueHits, m_sensitiveThreshold);
      m_sensitivePXD.push_back(sensitive);

      volume->SetSensitiveDetector(sensitive);
    }
    if (detectorType == "SVD") {
      if (sensorID == 0) B2FATAL("TB Geometry Creator: sensorID=0 in your XML is incompatible with VXD geometry cache. Use 1-based numbering of sensorID's.");
      VxdID xID(layer, ladder, sensorID);
      SVD::SensorInfo sensorInfo(xID, aWidth, aLength, aHeight,
                                 content.getInt("stripsU"), content.getInt("stripsV"), content.getLength("width2", 0));
      const double unit_pF = 1000 * Unit::fC / Unit::V; // picofarad
      sensorInfo.setSensorParams(
        content.getWithUnit("DepletionVoltage"),
        content.getWithUnit("BiasVoltage"), // Unit::V,
        content.getDouble("BackplaneCapacitance")* unit_pF,
        content.getDouble("InterstripCapacitance")* unit_pF,
        content.getDouble("CouplingCapacitance")* unit_pF
      );

      SVD::SensorInfo* newInfo = new SVD::SensorInfo(sensorInfo);
      VXD::SensitiveDetector<SVDSimHit, SVDTrueHit>* sensitive = new VXD::SensitiveDetector<SVDSimHit, SVDTrueHit>(newInfo, m_seeNeutrons, m_onlyPrimaryTrueHits, m_sensitiveThreshold);
      m_sensitiveSVD.push_back(sensitive);

      volume->SetSensitiveDetector(sensitive);
    }
    if (detectorType == "TEL") {
      TB::SensorInfo sensorInfo(sensorID, aWidth, aLength, aHeight, content.getInt("pixelsR"), content.getInt("pixelsZ[1]"), content.getLength("splitLength", 0), content.getInt("pixelsZ[2]", 0));

      TB::SensorInfo* newInfo = new TB::SensorInfo(sensorInfo);
      TB::SensitiveDetector* sensitive = new TB::SensitiveDetector(newInfo, m_seeNeutrons, m_onlyPrimaryTrueHits, m_sensitiveThreshold);
      m_sensitiveTB.push_back(sensitive);

      volume->SetSensitiveDetector(sensitive);
    }
    // set the stepSize and update volume name
    volume->SetUserLimits(new G4UserLimits(stepSize));
    volume->SetName(volume->GetName() + ".Active");
  }

  G4Transform3D GeoTBCreator::getAlignment(const string& component)
  {
    string path = (boost::format("Align[@component='%1%']/") % component).str();
    GearDir params(m_alignment, path);
    if (!params) {
      B2WARNING("Could not find alignment parameters for component " << component);
      return G4Transform3D();
    }
    double dU = params.getLength("du") / Unit::mm;
    double dV = params.getLength("dv") / Unit::mm;
    double dW = params.getLength("dw") / Unit::mm;
    double alpha = params.getAngle("alpha");
    double beta  = params.getAngle("beta");
    double gamma = params.getAngle("gamma");
    G4RotationMatrix rotation(alpha, beta, gamma);
    G4ThreeVector translation(dU, dV, dW);
    return G4Transform3D(rotation, translation);
  }

  void GeoTBCreator::readAddVolumes(const GearDir& content, G4LogicalVolume* parentVolume, int level = 1)
  {
    // loop through all Volume nodes
    BOOST_FOREACH(const GearDir & volume, content.getNodes("Volume")) {
      // use our function to construct G4 volume from GearDir parameters (name, size, material, color, visibility)
      G4LogicalVolume* g4vol = getLogicalVolume(volume);
      // create the physical volume - load transform from params and use same name as for the logical volume
      G4VPhysicalVolume* g4PhysVol = new G4PVPlacement(getTransform(volume), g4vol, g4vol->GetName(), parentVolume, false, 1);
      // get content of Active node
      GearDir activePar(volume, "Active");
      if (activePar != 0) {
        // if Active node is found, read its params and make it active
        setVolumeActive(activePar, g4vol, volume);
        // only in case we create PXD/SVD/TEL type sensor, let the GeoCache to search it through
        VXD::GeoCache::getInstance().findVolumes(g4PhysVol);
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
    m_alignment = GearDir(content, "Alignment/");

    GearDir volumes(content, "Volumes");
    readAddVolumes(volumes, &topVolume);
  }


}
