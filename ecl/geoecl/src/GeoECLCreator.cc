/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>

#include <ecl/geoecl/GeoECLCreator.h>
#include <ecl/geoecl/ECLGeometryPar.h>
#include <ecl/simecl/ECLSensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <cmath>
#include <boost/format.hpp>

#include <G4Material.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4Trap.hh>
#include <G4Cons.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>

#include <iostream>


#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>


#include <TVector3.h>



using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace ecl {
//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    geometry::CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    GeoECLCreator::GeoECLCreator()
    {
      m_sensitive = new ECLSensitiveDetector("ECLSensitiveDetector", (2*24)*eV, 10*MeV);
      logical_ecl = 0;
      physical_ecl = 0;
    }


    GeoECLCreator::~GeoECLCreator()
    {

    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {

      string CsI  = content.getString("CsI");
      G4Material* medCsI = geometry::Materials::get(CsI.c_str());
      G4Material* medAir = geometry::Materials::get("Air");
      string Si  = content.getString("Si");
      G4Material* medSi = geometry::Materials::get(Si.c_str());

      double eclWorld_I[6] = {452, 452, 1250, 1250, 395, 395};//unit:mm
      double eclWorld_O[6] = {1640, 1640, 1640, 1640, 1640, 1640};//unit:mm
      double eclWorld_Z[6] = { -145, -102, -102, 196, 196, 240};


      //define geometry
      G4Polycone* eclWorld = new G4Polycone("eclWorld", 0, 2*M_PI, 6, eclWorld_Z, eclWorld_I, eclWorld_O);
      G4LogicalVolume *logical_ecl = new G4LogicalVolume(eclWorld, medAir, "logical_ecl");
      physical_ecl = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logical_ecl, "physicalECL", &topVolume, false, 0);

      double DiodeWidth = content.getLength("k_diodewidth") * cm;
      double DiodeLength = content.getLength("k_diodelength") * cm;
      double DiodeHeight = content.getLength("k_diodeheight") * cm;

      G4Box* SensorDiode = new G4Box("diode", DiodeWidth / 2, DiodeLength / 2, DiodeHeight / 2);

      double k_BLL;
      double k_Ba;
      double k_Bb;
      double k_BA;
      double k_Bh;
      double k_BH;
      double k_z_TILTED;
      double k_phi_TILTED;
      double k_perpC;
      double k_phiC;
      double k_zC;
      double zsign = 1.;


      G4AssemblyVolume* assemblyBrCrystals = new G4AssemblyVolume();
      int nBarrelCrystal = content.getNumberNodes("BarrelCrystals/BarrelCrystal");

      for (int iBrCry = 1 ; iBrCry <= nBarrelCrystal ; ++iBrCry) {//46=29+17
        GearDir layerContent(content);
        layerContent.append((format("/BarrelCrystals/BarrelCrystal[%1%]/") % (iBrCry)).str());


        k_BLL = layerContent.getLength("K_BLL") * cm;
        k_Ba = layerContent.getLength("K_Ba")   * cm;
        k_Bb = layerContent.getLength("K_Bb")   * cm;
        k_Bh = layerContent.getLength("K_Bh")   * cm;
        k_BA = layerContent.getLength("K_BA")   * cm;
        k_BH = layerContent.getLength("K_BH")   * cm;
        k_z_TILTED = layerContent.getAngle("K_z_TILTED") ;
        k_phi_TILTED = layerContent.getAngle("K_phi_TILTED") ;
        k_perpC = layerContent.getLength("K_perpC") * cm ;
        k_phiC = layerContent.getAngle("K_phiC") ;
        k_zC = layerContent.getLength("K_zC")    * cm;

        double cDx1 = k_Ba / 2;
        double cDx2 = k_Bb / 2;
        double cDy1 = k_Bh / 2;
        double cDx3 = k_BA / 2;
        double cDx4 = k_Bb / 2 * k_BA / k_Ba;
        double cDy2 = k_BH / 2;
        double cDz = k_BLL / 2;
        if (iBrCry >= 30) {zsign = -1;}
        G4Transform3D r00 = G4RotateZ3D(90.*zsign * deg);
        G4Transform3D tilt_z = G4RotateY3D(k_z_TILTED);
        G4Transform3D tilt_phi = G4RotateZ3D(k_phi_TILTED);
        G4Transform3D position = G4Translate3D(k_perpC, 0, k_zC);  // Move over to the left...
        G4Transform3D pos_phi = G4RotateZ3D(k_phiC);
        G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * r00;

        G4Trap* BrCrysralShape = new G4Trap((format("solidEclBrCrystal_%1%") % iBrCry).str().c_str(),
                                            cDz , 0 , 0, cDy1, cDx2, cDx1, 0, cDy2 , cDx4, cDx3, 0);
        G4LogicalVolume* BrCrysral = new G4LogicalVolume(BrCrysralShape, medCsI, (format("logicalEclBrCrystal_%1%") % iBrCry).str().c_str(), 0, 0, 0);


        G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclDiode_%1%") % iBrCry).str().c_str(), 0, 0, 0);

        assemblyBrCrystals->AddPlacedVolume(BrCrysral, Tr);

        G4Transform3D DiodePosition = G4Translate3D(0, 0, (k_BLL + DiodeHeight) / 2);  // Move over to the left...
        G4Transform3D TrD = pos_phi * position * tilt_phi * tilt_z * r00 * DiodePosition ;
        assemblyBrCrystals->AddPlacedVolume(Sensor, TrD);




      }//iBrCry
      for (int iSector = 0; iSector < 72; ++iSector) {//total 72
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / 72 * deg);
        G4Transform3D BrRR = G4RotateZ3D((360.*iSector / 72 - 2.494688) * deg);

        assemblyBrCrystals->MakeImprint(logical_ecl, BrRR);
        assemblyBrCrystals->MakeImprint(logical_ecl, BrR);
      }//iSector

      double h1, h2, bl1, bl2, tl1, tl2, alpha1, alpha2, Rphi1, Rphi2, Rtheta, Pr, Ptheta, Pphi, halflength;

      G4AssemblyVolume* assemblyFwCrystals = new G4AssemblyVolume();
      for (int iCry = 1 ; iCry <= 72 ; ++iCry) {
        GearDir counter(content);
        counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

        h1 = counter.getLength("K_h1") * cm;
        h2 = counter.getLength("K_h2") * cm;
        bl1 = counter.getLength("K_bl1") * cm;
        bl2 = counter.getLength("K_bl2") * cm;
        tl1 = counter.getLength("K_tl1") * cm;
        tl2 = counter.getLength("K_tl2") * cm;
        alpha1 = counter.getAngle("K_alpha1");
        alpha2 = counter.getAngle("K_alpha2");
        Rphi1 = counter.getAngle("K_Rphi1") ;
        Rtheta = counter.getAngle("K_Ptheta") ;
        Rphi2 = counter.getAngle("K_Rphi2")  ;
        Pr = counter.getLength("K_Pr") * cm;
        Ptheta = counter.getAngle("K_Ptheta") ;
        Pphi = counter.getAngle("K_Pphi") ;
        halflength = 15.0;
        G4Transform3D m1 = G4RotateZ3D(Rphi1);
        G4Transform3D m2 = G4RotateY3D(Rtheta);
        G4Transform3D m3 = G4RotateZ3D(Rphi2);

        G4Transform3D position =
          G4Translate3D(Pr * sin(Ptheta) * cos(Pphi),
                        Pr * sin(Ptheta) * sin(Pphi),
                        Pr * cos(Ptheta));  // Move over to the left...
        G4Transform3D Tr = position * m3 * m2 * m1;
        G4Trap* FwCrysralShape = new G4Trap((format("solidEclFwCrystal_%1%") % iCry).str().c_str(),
                                            halflength , 0 , 0, h1 ,   bl1, tl1 , alpha1 , h2   , bl2, tl2, alpha2);
        G4LogicalVolume* FwCrysral = new G4LogicalVolume(FwCrysralShape, medCsI, (format("logicalEclFwCrystal_%1%") % iCry).str().c_str(), 0, 0, 0);
        G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclDiode_%1%") % iCry).str().c_str(), 0, 0, 0);


        assemblyFwCrystals->AddPlacedVolume(FwCrysral, Tr);

        G4Transform3D DiodePosition = G4Translate3D(0, 0, halflength + (DiodeHeight) / 2); // Move over to the left...
        G4Transform3D TrD =  position * m3 * m2 * m1 * DiodePosition ;
        assemblyFwCrystals->AddPlacedVolume(Sensor, TrD);


      }//forward endcap crystals

      G4AssemblyVolume* assemblyBwCrystals = new G4AssemblyVolume();
      for (int iCry = 73 ; iCry <= 132 ; ++iCry) {
        GearDir counter(content);
        counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

        h1 = counter.getLength("K_h1") * cm;
        h2 = counter.getLength("K_h2") * cm;
        bl1 = counter.getLength("K_bl1") * cm;
        bl2 = counter.getLength("K_bl2") * cm;
        tl1 = counter.getLength("K_tl1") * cm;
        tl2 = counter.getLength("K_tl2") * cm;
        alpha1 = counter.getAngle("K_alpha1");
        alpha2 = counter.getAngle("K_alpha2");
        Rphi1 = counter.getAngle("K_Rphi1") ;
        Rtheta = counter.getAngle("K_Ptheta") ;
        Rphi2 = counter.getAngle("K_Rphi2")  ;
        Pr = counter.getLength("K_Pr") * cm;
        Ptheta = counter.getAngle("K_Ptheta") ;
        Pphi = counter.getAngle("K_Pphi") ;
        halflength = 15.0;
        G4Transform3D m1 = G4RotateZ3D(Rphi1);
        G4Transform3D m2 = G4RotateY3D(Rtheta);
        G4Transform3D m3 = G4RotateZ3D(Rphi2);

        G4Transform3D position =
          G4Translate3D(Pr * sin(Ptheta) * cos(Pphi),
                        Pr * sin(Ptheta) * sin(Pphi),
                        Pr * cos(Ptheta));  // Move over to the left...
        G4Transform3D Tr = position * m3 * m2 * m1;
        G4Trap* BwCrysralShape = new G4Trap((format("solidEclBwCrystal_%1%") % iCry).str().c_str(),
                                            halflength , 0 , 0, h1 ,   bl1, tl1 , alpha1 , h2   , bl2, tl2, alpha2);
        G4LogicalVolume* BwCrysral = new G4LogicalVolume(BwCrysralShape, medCsI, (format("logicalEclBwCrystal_%1%") % iCry).str().c_str(), 0, 0, 0);
        G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclDiode_%1%") % iCry).str().c_str(), 0, 0, 0);

        assemblyBwCrystals->AddPlacedVolume(BwCrysral, Tr);

        G4Transform3D DiodePosition = G4Translate3D(0, 0, halflength + (DiodeHeight) / 2); // Move over to the left...
        G4Transform3D TrD =  position * m3 * m2 * m1 * DiodePosition ;
        assemblyBwCrystals->AddPlacedVolume(Sensor, TrD);

      }//backward endcap crystals


      for (int iSector = 0; iSector < 16; ++iSector) {//total 72
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / 16 * deg);
        assemblyFwCrystals->MakeImprint(logical_ecl, BrR);
        assemblyBwCrystals->MakeImprint(logical_ecl, BrR);
      }//16 sectior



    }//create
  }//ecl
}//belle2
