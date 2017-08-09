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
#include <svd/geometry/SensorInfo.h>
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
#include <G4Point3D.hh>
#include <G4ThreeVector.hh>
#include <G4PVPlacement.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4IntersectionSolid.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>


//#define MATERIAL_SCAN

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstruction of the TB environment */
  namespace TB {


    /** Register the creator */
    geometry::CreatorFactory<GeoTBCreator> GeoTBFactory("VXDTBCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    GeoTBCreator::GeoTBCreator()
    {}

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

      BOOST_FOREACH(TEL::SensitiveDetector * sensitive, m_sensitiveTEL) {
        delete sensitive;
      }
      m_sensitiveTEL.clear();
    }

    G4LogicalVolume* GeoTBCreator::getLogicalVolume(const GearDir& content)
    {
      std::string volName = content.getString("@name", "");

      //NOTE: This is for backward compatibility.
      // Some name must always be specified to force users to keep XML readable
      if (volName == "") volName = content.getString("Name");
      // (end of note)

      std::string volShape = content.getString("Shape", "Box");
      std::string volColor = content.getString("Color", "#000000");
      bool volVisibility = content.getBool("Visible", true);

      std::string volMaterial = content.getString("Material", m_worldMaterial);
      G4Material* volG4Material = geometry::Materials::get(volMaterial);
      G4LogicalVolume* g4vol = 0;

      if (volShape == "Box") {
        double volHalfX = content.getLength("HalfX") / Unit::mm;
        double volHalfY = content.getLength("HalfY") / Unit::mm;
        double volHalfZ = content.getLength("HalfZ") / Unit::mm;
        G4Box* volG4Box = new G4Box(string("TB_G4Box_") + volName, volHalfX, volHalfY, volHalfZ);
        g4vol = new G4LogicalVolume(volG4Box, volG4Material, volName);
      } else if (volShape == "Tube") {
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
      } else {
        B2FATAL("Unsupported volume shape");
      }

      // Subtraction of volumes
      std::vector<GearDir> boolVolumes = content.getNodes("Subtract");
      if (!boolVolumes.empty()) {
        // These volumes do not need material

        BOOST_FOREACH(const GearDir & sub_volume, boolVolumes) {
          G4LogicalVolume* toSubtract = getLogicalVolume(sub_volume);
          G4VSolid* subtracted = new G4SubtractionSolid(G4String(volName + "_subtracted_" + toSubtract->GetName()), g4vol->GetSolid(),
                                                        toSubtract->GetSolid(), getTransform(sub_volume));
          delete g4vol;
          delete toSubtract;
          g4vol = new G4LogicalVolume(subtracted, volG4Material, volName);
        }
      }
      // Union with volumes
      boolVolumes = content.getNodes("Union");
      if (!boolVolumes.empty()) {
        // These volumes do not need material

        BOOST_FOREACH(const GearDir & sub_volume, boolVolumes) {
          G4LogicalVolume* toUnion = getLogicalVolume(sub_volume);
          G4VSolid* united = new G4UnionSolid(G4String(volName + "_united_" + toUnion->GetName()), g4vol->GetSolid(), toUnion->GetSolid(),
                                              getTransform(sub_volume));
          delete g4vol;
          delete toUnion;
          g4vol = new G4LogicalVolume(united, volG4Material, volName);
        }
      }
      // Interesct with volumes
      boolVolumes = content.getNodes("Intersect");
      if (!boolVolumes.empty()) {
        // These volumes do not need material

        BOOST_FOREACH(const GearDir & sub_volume, boolVolumes) {
          G4LogicalVolume* toIntersect = getLogicalVolume(sub_volume);
          G4VSolid* intersected = new G4IntersectionSolid(G4String(volName + "_intersected_" + toIntersect->GetName()), g4vol->GetSolid(),
                                                          toIntersect->GetSolid(), getTransform(sub_volume));
          delete g4vol;
          delete toIntersect;
          g4vol = new G4LogicalVolume(intersected, volG4Material, volName);
        }
      }
      Belle2::geometry::setColor(*g4vol, volColor);
      Belle2::geometry::setVisibility(*g4vol, volVisibility);

      // All helper logical volumes created along the way by "new" are deleted.
      // Final result, which is returned, is the only remaining object on the heap
      return g4vol;
    }

    G4Transform3D GeoTBCreator::getTransform(const GearDir& content)
    {

      double volDX = content.getLength("DX", 0.) / Unit::mm;
      double volDY = content.getLength("DY", 0.) / Unit::mm;
      double volDZ = content.getLength("DZ", 0.) / Unit::mm;
      double volPhi = content.getAngle("Phi", 0.);
      double volTheta = content.getAngle("Theta", 0.);
      double volPsi = content.getAngle("Psi", 0.);

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
      if (parentContent.getString("Shape", "Box") != "Box")
        B2FATAL("Only 'Box' shape can host a sensitive volume (Active node) for supported detectors");

      // get size of the volume from parent node GearDir (these are half-sizes - multiply by 2)
      double aWidth = 2.0 * parentContent.getLength("HalfX");// Unit::mm/10.0;
      double aLength = 2.0 * parentContent.getLength("HalfY");// Unit::mm/10.0;
      double aHeight = 2.0 * parentContent.getLength("HalfZ");// Unit::mm/10.0;

      unsigned short sensorID = content.getInt("SensorID");
      unsigned short ladder = content.getInt("Ladder");
      unsigned short layer = content.getInt("Layer");
      if (sensorID == 0 || ladder == 0
          || layer == 0) B2FATAL("Do not use 0-based id/ladder/layer in your XML, it is incompatible with VXD geometry cache.");

      double stepSize = content.getLength("stepSize") ;// Unit::mm;
      string detectorType = content.getString("DetectorType", "Unknown");

      if (detectorType == "PXD") {
        VxdID xID(layer, ladder, sensorID);
        PXD::SensorInfo sensorInfo(xID,
                                   aWidth,
                                   aLength,
                                   aHeight,
                                   content.getInt("pixelsR"),//!!!exchanged
                                   content.getInt("pixelsZ[1]"),//!!!exchanged
                                   content.getLength("splitLength", 0),
                                   content.getInt("pixelsZ[2]", 0)
                                  );
        sensorInfo.setDEPFETParams(
          content.getDouble("BulkDoping") / (Unit::um * Unit::um * Unit::um),
          content.getWithUnit("BackVoltage"),
          content.getWithUnit("TopVoltage"),
          content.getLength("SourceBorderSmallPixel"),
          content.getLength("ClearBorderSmallPixel"),
          content.getLength("DrainBorderSmallPixel"),
          content.getLength("SourceBorderLargePixel"),
          content.getLength("ClearBorderLargePixel"),
          content.getLength("DrainBorderLargePixel"),
          content.getLength("GateDepth"),
          content.getBool("DoublePixel"),
          content.getDouble("ChargeThreshold"),
          content.getDouble("NoiseFraction")
        );
        sensorInfo.setIntegrationWindow(
          content.getTime("IntegrationStart"),
          content.getTime("IntegrationEnd")
        );

        PXD::SensorInfo* newInfo = new PXD::SensorInfo(sensorInfo);
        PXD::SensitiveDetector* sensitive = new PXD::SensitiveDetector(newInfo);
        sensitive->setOptions(m_seeNeutrons, m_onlyPrimaryTrueHits, m_distanceTolerance, m_electronTolerance, m_minimumElectrons);
        m_sensitivePXD.push_back(sensitive);

        volume->SetSensitiveDetector(sensitive);

      } else if (detectorType == "SVD") {
        VxdID xID(layer, ladder, sensorID);
        SVD::SensorInfo sensorInfo(xID,
                                   aWidth,
                                   aLength,
                                   aHeight,
                                   content.getInt("stripsU"),//!!! exchanged
                                   content.getInt("stripsV"),//!!! exchanged
                                   content.getLength("width2", 0));
        const double unit_pFcm = 1000 * Unit::fC / Unit::V / Unit::cm;
        sensorInfo.setSensorParams(
          content.getWithUnit("stripEdgeU"),
          content.getWithUnit("stripEdgeV"),
          content.getWithUnit("DepletionVoltage"),
          content.getWithUnit("BiasVoltage"),
          content.getDouble("BackplaneCapacitanceU") * unit_pFcm,
          content.getDouble("InterstripCapacitanceU") * unit_pFcm,
          content.getDouble("CouplingCapacitanceU") * unit_pFcm,
          content.getDouble("BackplaneCapacitanceV") * unit_pFcm,
          content.getDouble("InterstripCapacitanceV") * unit_pFcm,
          content.getDouble("CouplingCapacitanceV") * unit_pFcm,
          content.getWithUnit("ADUEquivalentU"),
          content.getWithUnit("ADUEquivalentV"),
          content.getWithUnit("ElectronicNoiseU"),
          content.getWithUnit("ElectronicNoiseV"),
          content.getWithUnit("ADUEquivalentSbwU", 0),
          content.getWithUnit("ADUEquivalentSbwV", 0),
          content.getWithUnit("ElectronicNoiseSbwU", 0),
          content.getWithUnit("ElectronicNoiseSbwV", 0)
        );

        SVD::SensorInfo* newInfo = new SVD::SensorInfo(sensorInfo);
        VXD::SensitiveDetector<SVDSimHit, SVDTrueHit>* sensitive = new VXD::SensitiveDetector<SVDSimHit, SVDTrueHit>(newInfo);
        sensitive->setOptions(m_seeNeutrons, m_onlyPrimaryTrueHits, m_distanceTolerance, m_electronTolerance, m_minimumElectrons);
        m_sensitiveSVD.push_back(sensitive);

        volume->SetSensitiveDetector(sensitive);
        // Supress false positive from cppcheck
// cppcheck-suppress memleak
      } else if (detectorType == "TEL") {
        VxdID xID(layer, ladder, sensorID);
        TEL::SensorInfo sensorInfo(xID,
                                   aWidth,
                                   aLength,
                                   aHeight,
                                   content.getInt("pixelsR"),
                                   content.getInt("pixelsZ[1]"),
                                   content.getLength("splitLength", 0),
                                   content.getInt("pixelsZ[2]", 0)
                                  );

        TEL::SensorInfo* newInfo = new TEL::SensorInfo(sensorInfo);
        TEL::SensitiveDetector* sensitive = new TEL::SensitiveDetector(newInfo);
        sensitive->setOptions(m_seeNeutrons, m_onlyPrimaryTrueHits, m_distanceTolerance, m_electronTolerance, m_minimumElectrons);
        m_sensitiveTEL.push_back(sensitive);

        volume->SetSensitiveDetector(sensitive);

      } else {
        B2FATAL("Unsupported or non-specified DetecorType for Active node");
      }

      // set the stepSize and update volume name with VXD ID
      volume->SetUserLimits(new G4UserLimits(stepSize));
      volume->SetName(volume->GetName().append((
                                                 boost::format(".Active_%1%")
                                                 %
                                                 (int)VxdID(layer, ladder, sensorID)
                                               ).str()));
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

    void GeoTBCreator::readAddVolumes(const GearDir& content, G4LogicalVolume* parentVolume, int level)
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
        }
        // check child Volume tags and read them recursively
        GearDir childVolumes(volume, "Volume");
        if (childVolumes) readAddVolumes(volume, g4vol, level + 1);
        if (level == 1) VXD::GeoCache::getInstance().findVolumes(g4PhysVol);
      }
    }

    void GeoTBCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      m_activeChips = content.getBool("ActiveChips", m_activeChips);
      m_seeNeutrons = content.getBool("SeeNeutrons", m_seeNeutrons);
      m_onlyPrimaryTrueHits = content.getBool("OnlyPrimaryTrueHits", m_onlyPrimaryTrueHits);
      m_distanceTolerance = (float)content.getLength("DistanceTolerance", m_distanceTolerance);
      m_electronTolerance = (float)content.getDouble("ElectronTolerance", m_electronTolerance);
      m_minimumElectrons = (float)content.getDouble("MinimumElectrons", m_minimumElectrons);

      m_worldMaterial = content.getString("DefaultMaterial", "Air");

      m_alignment = GearDir(content, "Alignment/");

      GearDir volumes(content, "Volumes");
      readAddVolumes(volumes, &topVolume);

    }

  } // namespace TB

} // namespace Belle2




