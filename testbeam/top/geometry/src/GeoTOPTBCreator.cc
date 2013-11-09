/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/top/geometry/GeoTOPTBCreator.h>
#include <testbeam/top/geometry/DetectorTypes.h>
#include <testbeam/top/simulation/SensitiveScintillator.h>


#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4Material.hh>

#include <G4TwoVector.hh>
#include <G4ThreeVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Colour.hh>

#include <boost/foreach.hpp>


using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace TOPTB {

    /** Register the creator */
    geometry::CreatorFactory<GeoTOPTBCreator> GeoTOPFactory("TOPTBCreator");


    GeoTOPTBCreator::GeoTOPTBCreator()
    {
    }


    GeoTOPTBCreator::~GeoTOPTBCreator()
    {
      for (unsigned i = 0; i < m_sensitiveScintillators.size(); i++) {
        delete m_sensitiveScintillators[i];
      }
    }


    void GeoTOPTBCreator::create(const GearDir& content,
                                 G4LogicalVolume& motherVolume,
                                 GeometryTypes)
    {

      GearDir elements(content, "elements");
      GearDir setups(content, "setups");
      GearDir placements(content, "placements");

      std::string placementName = content.getString("experimentType") + "exp" +
                                  content.getString("experimentNumber");

      cout << "GeoTOPTBCreator: creating geometry for " << placementName << endl;

      GearDir placementParameters(placements, placementName);
      if (!placementParameters) {
        B2ERROR("No positioning data found in <placements> for " << placementName);
        return;
      }

      std::string setupName = placementParameters.getString("setup");
      GearDir setupParameters(setups, setupName);
      if (!setupParameters) {
        B2ERROR("No setup data found in <setups> for " << setupName);
        return;
      }

      G4AssemblyVolume* setup = createSetup(setupParameters, elements);
      if (!setup) {
        B2ERROR("Geometry creation failed for: " << setupName);
        return;
      }
      G4Transform3D Tr = getTransformation(placementParameters);
      setup->MakeImprint(&motherVolume, Tr);

      cout << endl;

    }


    G4AssemblyVolume* GeoTOPTBCreator::createSetup(const GearDir& content,
                                                   const GearDir& elements)
    {
      G4AssemblyVolume* setup = new G4AssemblyVolume();

      BOOST_FOREACH(const GearDir & node, content.getNodes("element")) {
        std::string elementType = node.getString("type");
        std::string elementName = node.getString("name");
        int detectorID = node.getInt("detectorID", 0);
        G4RotationMatrix* rotation = getRotation(node);
        G4ThreeVector translation = getTranslation(node);
        G4LogicalVolume* element = createElement(elements, elementType,
                                                 elementName, detectorID);
        if (!element) continue;
        setup->AddPlacedVolume(element, translation, rotation);

        cout << elementType << " " << elementName << " ID=" << detectorID;
        cout << " " << translation.x();
        cout << " " << translation.y();
        cout << " " << translation.z();
        cout << " " << rotation->getPhi() / Unit::deg;
        cout << " " << rotation->getTheta() / Unit::deg;
        cout << " " << rotation->getPsi() / Unit::deg;
        cout << endl;

      }

      return setup;
    }


    G4LogicalVolume* GeoTOPTBCreator::createElement(const GearDir& elements,
                                                    std::string elementType,
                                                    std::string elementName,
                                                    int detectorID)
    {
      GearDir element(elements, elementType);
      if (!element) {
        B2ERROR("No element data found in <elements> for " << elementType);
        return NULL;
      }

      std::string creatorName = element.getString("creatorName");
      if (creatorName == "scintillatorCounter") {
        G4LogicalVolume* ele = scintillatorCounter(element, elementName, detectorID);
        return ele;
      }
      if (creatorName == "SciFiTracker") {
        G4LogicalVolume* ele = sciFiTracker(element, elementName, detectorID);
        return ele;
      }

      B2ERROR("Creator with the name " << creatorName << " doesn't exist");
      return NULL;

    }



    G4Transform3D GeoTOPTBCreator::getTransformation(const GearDir& content,
                                                     std::string methodName)
    {
      double x = 0;
      double y = 0;
      double z = 0;
      double phi = 0;
      double theta = 0;
      double psi = 0;

      if (methodName == "") methodName = content.getString("method", "Geant");
      if (methodName == "Geant") {
        x = content.getLength("x", 0.0) / Unit::mm;
        y = content.getLength("y", 0.0) / Unit::mm;
        z = content.getLength("z", 0.0) / Unit::mm;
        phi = content.getAngle("phi", 0.0);
        theta = content.getAngle("theta", 0.0);
        psi = content.getAngle("psi", 0.0);
      } else if (methodName == "AlongBeam") {
        double shift = content.getLength("upstreamShift") / Unit::mm;
        double xBeam = content.getLength("xBeam", 0.0) / Unit::mm;
        double yBeam = content.getLength("yBeam", 0.0) / Unit::mm;
        double zBeam = content.getLength("zBeam", 0.0) / Unit::mm;
        double thetaBeam = content.getAngle("thetaBeam", 90.0);
        double phiBeam = content.getAngle("phiBeam", 0.0);
        x = xBeam + shift * cos(phiBeam) * sin(thetaBeam);
        y = yBeam + shift * sin(phiBeam) * sin(thetaBeam);
        z = zBeam + shift * cos(thetaBeam);
        phi = M_PI / 2; // valid only for small phiBeam (TODO: to be generalized!)
        theta = thetaBeam;
        psi = M_PI / 2 - phiBeam;
      } else {
        B2ERROR("unknown transformation method: " << methodName);
      }

      G4ThreeVector transl(x, y, z);
      G4RotationMatrix rot(phi, theta, psi);
      G4Transform3D Tr(rot, transl);
      return Tr;
    }


    G4RotationMatrix* GeoTOPTBCreator::getRotation(const GearDir& content)
    {
      double phi = content.getAngle("phi", 0.0);
      double theta = content.getAngle("theta", 0.0);
      double psi = content.getAngle("psi", 0.0);

      G4RotationMatrix* rot = new G4RotationMatrix(phi, theta, psi);
      return rot;
    }


    G4ThreeVector GeoTOPTBCreator::getTranslation(const GearDir& content)
    {
      double x = content.getLength("x", 0.0) / Unit::mm;
      double y = content.getLength("y", 0.0) / Unit::mm;
      double z = content.getLength("z", 0.0) / Unit::mm;

      G4ThreeVector translation(x, y, z);
      return translation;
    }


    //------- creator functions for elements of TB geometry ---------------------

    G4LogicalVolume* GeoTOPTBCreator::scintillatorCounter(const GearDir& content,
                                                          std::string elementName,
                                                          int detectorID)
    {
      if (!content) return NULL;

      EDetectorType detectorType = c_scintillator;

      std::string Material = content.getString("Material");
      double width = content.getLength("width") / Unit::mm;
      double height = content.getLength("height") / Unit::mm;
      double thickness = content.getLength("thickness") / Unit::mm;

      G4Box* box = new G4Box("scintillator", width / 2, height / 2, thickness / 2);
      G4Material* material = Materials::get(Material);
      G4LogicalVolume* counter = new G4LogicalVolume(box, material, elementName);

      SensitiveScintillator* sensitive =
        new SensitiveScintillator(detectorID, detectorType);
      m_sensitiveScintillators.push_back(sensitive);
      counter->SetSensitiveDetector(sensitive);

      return counter;

    }


    G4LogicalVolume* GeoTOPTBCreator::sciFiTracker(const GearDir& content,
                                                   std::string elementName,
                                                   int detectorID)
    {
      if (!content) return NULL;

      EDetectorType detectorType = c_sciFi;

      // get data from DB
      std::string emptySpaceMaterial = content.getString("Material");
      double fullThickness = content.getLength("fullThickness", 0.0) / Unit::mm;

      GearDir fibers(content, "fibers");
      if (!fibers) {
        B2ERROR("SciFi: fibers not defined");
        return NULL;
      }
      std::string fiberMaterial = fibers.getString("Material");
      std::string coatingMaterial = fibers.getString("coatingMaterial");
      double fiberDiameter = fibers.getLength("fiberDiameter") / Unit::mm;
      double activeDiameter = fibers.getLength("activeDiameter") / Unit::mm;
      double fiberLength = fibers.getLength("fiberLength") / Unit::mm;
      int numFibers = fibers.getInt("numberOfFibers");
      std::string firstFiber = fibers.getString("firstFiber", "z-");
      double row2rowDistance = fibers.getLength("row2rowDistance", 0.0) / Unit::mm;

      GearDir coverPlate(content, "coverPlate");
      if (!coverPlate) {
        B2ERROR("SciFi: cover plate not defined");
        return NULL;
      }
      std::string coverMaterial = coverPlate.getString("Material");
      double width = coverPlate.getLength("width") / Unit::mm;
      double height = coverPlate.getLength("height") / Unit::mm;
      double thickness = coverPlate.getLength("thickness") / Unit::mm;

      double ccDistance = sqrt(3.) / 2 * fiberDiameter;
      if (row2rowDistance < ccDistance) row2rowDistance = ccDistance;
      double minThickness = 2 * thickness + row2rowDistance + fiberDiameter;
      if (fullThickness < minThickness) fullThickness = minThickness;

      // detector volume
      G4Box* box = new G4Box("sciFiTracker", width / 2, height / 2, fullThickness / 2);
      G4Material* material = Materials::get(emptySpaceMaterial);
      G4LogicalVolume* counter = new G4LogicalVolume(box, material, elementName);

      // cover plate
      G4Box* cover = new G4Box("coverPlate", width / 2, height / 2, thickness / 2);
      material = Materials::get(coverMaterial);
      G4LogicalVolume* coverPlateLV = new G4LogicalVolume(cover, material, "coverPlate");

      // placement of cover plates
      double zCover = -(fullThickness - thickness) / 2.0;
      for (int i = 0; i < 2; i++) {
        G4Transform3D Tr = G4Translate3D(0, 0, zCover);
        new G4PVPlacement(Tr, coverPlateLV, "coverPlate", counter, false, i);
        zCover = -zCover;
      }

      // fiber core
      G4Tubs* core = new G4Tubs("core", 0., activeDiameter / 2, fiberLength / 2,
                                0., 2 * M_PI);
      material = Materials::get(fiberMaterial);
      G4LogicalVolume* fiberCore = new G4LogicalVolume(core, material, "fiberCore");
      SensitiveScintillator* sensitive =
        new SensitiveScintillator(detectorID, detectorType);
      m_sensitiveScintillators.push_back(sensitive);
      fiberCore->SetSensitiveDetector(sensitive);

      // fiber coating
      G4Tubs* coat = new G4Tubs("coat", activeDiameter / 2, fiberDiameter / 2,
                                fiberLength / 2, 0., 2 * M_PI);
      material = Materials::get(coatingMaterial);
      G4LogicalVolume* fiberCoat = new G4LogicalVolume(coat, material, "fiberCoat");

      // placement of fibers
      G4RotationMatrix* rot = new G4RotationMatrix(0, M_PI / 2, 0);
      double x = -fiberDiameter * (numFibers - 1) / 4.0;
      double dx = fiberDiameter / 2.0;
      double z = -row2rowDistance / 2.0;
      if (firstFiber == "z+") z = -z;
      for (int i = 0; i < numFibers; i++) {
        G4ThreeVector transl(x, 0, z);
        int channelID = i + 1;
        new G4PVPlacement(rot, transl, fiberCore, "fiberCore", counter, false, channelID);
        new G4PVPlacement(rot, transl, fiberCoat, "fiberCoat", counter, false, channelID);
        x += dx;
        z = -z;
      }

      return counter;
    }


    G4LogicalVolume* GeoTOPTBCreator::vetoCounter(const GearDir& content,
                                                  std::string elementName,
                                                  int detectorID)
    {
      if (!content) return NULL;

      EDetectorType detectorType = c_veto;

      std::string Material             = content.getString("Material");
      std::string ShieldingMaterial    = content.getString("shieldingMaterial");
      std::string ESMaterial           = content.getString("ESMaterial");
      double width                     = content.getLength("width") / Unit::mm;
      double height                    = content.getLength("height") / Unit::mm;
      double thickness                 = content.getLength("thickness") / Unit::mm;
      double shieldingThickness        = content.getLength("shieldingThickness") / Unit::mm;
      double fullThickness             = thickness + 2 * shieldingThickness;

      //detector volume
      G4Box* box                       = new G4Box("vetoCounter", width / 2.0, height / 2.0, fullThickness / 2.0);
      G4Material* material             = Materials::get(ESMaterial);
      G4LogicalVolume* counter         = new G4LogicalVolume(box, material, elementName);

      G4Box* vetoCounterBox            = new G4Box("vetoCounterBox", width / 2.0, height / 2.0, thickness / 2.0);
      G4Material* vetoCounterMaterial  = Materials::get(Material);
      G4LogicalVolume* vetoCounterLV   = new G4LogicalVolume(vetoCounterBox, vetoCounterMaterial, "vetoCounter");

      G4Box* shieldingBox              = new G4Box("vetoCounterShieldingBox", width / 2.0, height / 2.0, shieldingThickness / 2.0);
      G4Material* shieldingMaterial    = Materials::get(ShieldingMaterial);
      G4LogicalVolume* shieldingLV     = new G4LogicalVolume(shieldingBox, shieldingMaterial, "vetoCounterShielding");

      //Placement of counter and shielding
      G4Transform3D Translation1       = G4Translate3D(0, 0, -thickness - shieldingThickness);
      G4Transform3D Translation2       = G4Translate3D(0, 0, thickness + shieldingThickness);
      new G4PVPlacement(G4Transform3D(), vetoCounterLV, "vetoCounter", counter, false, 0);
      new G4PVPlacement(Translation1, shieldingLV, "vetoCounterShielding", counter, false, 0);
      new G4PVPlacement(Translation2, shieldingLV, "vetoCounterShielding", counter, false, 1);

      SensitiveScintillator* sensitive = new SensitiveScintillator(detectorID, detectorType);
      m_sensitiveScintillators.push_back(sensitive);
      vetoCounterLV->SetSensitiveDetector(sensitive);

      return counter;

    }

  } // namespace TOPTB
} // namespace Belle2


