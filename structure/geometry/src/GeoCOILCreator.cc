/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <structure/geometry/GeoCOILCreator.h>
#include <structure/dbobjects/COILGeometryPar.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>

#include <iostream>

/* Geant4 default unit
                millimeter              (mm)
                nanosecond              (ns)
                Mega electron Volt      (MeV)
                positron charge         (eplus)
                degree Kelvin           (kelvin)
                the amount of substance (mole)
                luminous intensity      (candela)
                radian                  (radian)
                steradian               (steradian)
*/


using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace coil {
    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoCOILCreator> GeoCOILFactory("COILCreator");
    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoCOILCreator::GeoCOILCreator()
    {
      m_VisAttributes.clear();
      m_VisAttributes.push_back(new G4VisAttributes(false)); // for "invisible"
    }


    GeoCOILCreator::~GeoCOILCreator()
    {
      for (G4VisAttributes* visAttr : m_VisAttributes) delete visAttr;
      m_VisAttributes.clear();
    }

    //Old way of creating geometry
    void GeoCOILCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {
      COILGeometryPar config = readConfiguration(content);
      createGeometry(config, topVolume, type);
    }

    //Create config obgects and store in database
    void GeoCOILCreator::createPayloads(const GearDir& content, const IntervalOfValidity& iov)
    {
      DBImportObjPtr<COILGeometryPar> importObj;
      importObj.construct(readConfiguration(content));
      importObj.import(iov);
    }

    //Create geometry from database
    void GeoCOILCreator::createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {
      DBObjPtr<COILGeometryPar> dbObj;
      if (!dbObj) {
        // Check that we found the object and if not report the problem
        B2FATAL("No configuration for " << name << " found.");
      }
      createGeometry(*dbObj, topVolume, type);
    }


    // Write COILGeometryPar object from GearBox content
    COILGeometryPar GeoCOILCreator::readConfiguration(const GearDir& content)
    {
      COILGeometryPar parameters;

      // Global parameters
      parameters.setGlobalRotAngle(content.getAngle("Rotation") / Unit::rad);
      parameters.setGlobalOffsetZ(content.getLength("OffsetZ") / Unit::mm);

      // Cryostat
      parameters.setCryoMaterial(content.getString("Cryostat/Material", "Air"));
      parameters.setCryoRmin(content.getLength("Cryostat/Rmin") / Unit::mm);
      parameters.setCryoRmax(content.getLength("Cryostat/Rmax") / Unit::mm);
      parameters.setCryoLength(content.getLength("Cryostat/HalfLength") / Unit::mm);

      //Cavity #1
      parameters.setCav1Material(content.getString("Cavity1/Material", "Air"));
      parameters.setCav1Rmin(content.getLength("Cavity1/Rmin") / Unit::mm);
      parameters.setCav1Rmax(content.getLength("Cavity1/Rmax") / Unit::mm);
      parameters.setCav1Length(content.getLength("Cavity1/HalfLength") / Unit::mm);

      // Radiation Shield
      parameters.setShieldMaterial(content.getString("RadShield/Material", "Air"));
      parameters.setShieldRmin(content.getLength("RadShield/Rmin") / Unit::mm);
      parameters.setShieldRmax(content.getLength("RadShield/Rmax") / Unit::mm);
      parameters.setShieldLength(content.getLength("RadShield/HalfLength") / Unit::mm);

      // Cavity #2
      parameters.setCav2Material(content.getString("Cavity2/Material", "Air"));
      parameters.setCav2Rmin(content.getLength("Cavity2/Rmin") / Unit::mm);
      parameters.setCav2Rmax(content.getLength("Cavity2/Rmax") / Unit::mm);
      parameters.setCav2Length(content.getLength("Cavity2/HalfLength") / Unit::mm);

      // Coil
      parameters.setCoilMaterial(content.getString("Coil/Material", "Air"));
      parameters.setCoilRmin(content.getLength("Coil/Rmin") / Unit::mm);
      parameters.setCoilRmax(content.getLength("Coil/Rmax") / Unit::mm);
      parameters.setCoilLength(content.getLength("Coil/HalfLength") / Unit::mm);

      return parameters;
    }

    // Create geometry from COILGeometryPar object
    void GeoCOILCreator::createGeometry(const COILGeometryPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {
      // Global parameters
      double GlobalRotAngle = parameters.getGlobalRotAngle();
      double GlobalOffsetZ = parameters.getGlobalOffsetZ();


      // Cryostat
      /////////////

      string strMatCryo = parameters.getCryoMaterial();
      double CryoRmin   = parameters.getCryoRmin();
      double CryoRmax   = parameters.getCryoRmax();
      double CryoLength = parameters.getCryoLength();

      G4Material* matCryostat = Materials::get(strMatCryo);
      G4Tubs* CryoTube =
        new G4Tubs("Cryostat", CryoRmin, CryoRmax, CryoLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* CryoLV =
        new G4LogicalVolume(CryoTube, matCryostat, "LVCryo", 0, 0, 0);
      new G4PVPlacement(G4TranslateZ3D(GlobalOffsetZ) * G4RotateZ3D(GlobalRotAngle),
                        CryoLV, "PVCryo", &topVolume, false, 0);


      // Cavity #1
      //////////////
      string strMatCav1 = parameters.getCav1Material();
      double Cav1Rmin   = parameters.getCav1Rmin();
      double Cav1Rmax   = parameters.getCav1Rmax();
      double Cav1Length = parameters.getCav1Length();

      G4Material* matCav1 = Materials::get(strMatCav1);

      G4Tubs* Cav1Tube =
        new G4Tubs("Cavity1", Cav1Rmin, Cav1Rmax, Cav1Length, 0.0, M_PI * 2.0);
      G4LogicalVolume* Cav1LV =
        new G4LogicalVolume(Cav1Tube, matCav1, "LVCav1", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(0., 1., 0.)));
      Cav1LV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), Cav1LV, "PVCav1", CryoLV, false, 0);


      // Rad Shield
      ///////////////
      string strMatShield = parameters.getShieldMaterial();
      double ShieldRmin   = parameters.getShieldRmin();
      double ShieldRmax   = parameters.getShieldRmax();
      double ShieldLength = parameters.getShieldLength();

      G4Material* matShield = Materials::get(strMatShield);

      G4Tubs* ShieldTube =
        new G4Tubs("RadShield", ShieldRmin, ShieldRmax,  ShieldLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* ShieldLV =
        new G4LogicalVolume(ShieldTube, matShield, "LVShield", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(1., 1., 0.)));
      ShieldLV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), ShieldLV, "PVShield", Cav1LV, false, 0);


      // Cavity #2
      /////////////
      string strMatCav2 = parameters.getCav2Material();
      double Cav2Rmin   = parameters.getCav2Rmin();
      double Cav2Rmax   = parameters.getCav2Rmax();
      double Cav2Length = parameters.getCav2Length();

      G4Material* matCav2 = Materials::get(strMatCav2);

      G4Tubs* Cav2Tube =
        new G4Tubs("Cavity2", Cav2Rmin, Cav2Rmax, Cav2Length, 0.0, M_PI * 2.0);
      G4LogicalVolume* Cav2LV =
        new G4LogicalVolume(Cav2Tube, matCav2, "LVCav2", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(1., 1., 1.)));
      Cav2LV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), Cav2LV, "PVCav2", ShieldLV, false, 0);


      // Coil
      /////////
      string strMatCoil = parameters.getCoilMaterial();
      double CoilRmin   = parameters.getCoilRmin();
      double CoilRmax   = parameters.getCoilRmax();
      double CoilLength = parameters.getCoilLength();

      G4Material* matCoil = Materials::get(strMatCoil);

      G4Tubs* CoilTube =
        new G4Tubs("Coil", CoilRmin, CoilRmax, CoilLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* CoilLV =
        new G4LogicalVolume(CoilTube, matCoil, "LVCoil", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(0., 1., 1.)));
      CoilLV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), CoilLV, "PVCoil", Cav2LV, false, 0);

    }
  }
}

