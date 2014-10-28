/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/geometry/CsiCreator.h>
#include <beast/csi/simulation/SensitiveDetector.h>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <csi/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <G4AssemblyVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include <G4Tubs.hh>

//Visualization Attributes
#include <G4VisAttributes.hh>

#define PI 3.14159265358979323846

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CSI detector */
  namespace csi {

    // Register the creator
    /** Creator creates the CSI geometry */
    geometry::CreatorFactory<CsiCreator> CsiFactory("CSICreator");


    const double k_CLR(0.01   * CLHEP::cm);                 // clearance around outside
    const double k_backConFarZ(-145.5 * CLHEP::cm);        // Z of back container back
    const double k_backConNearZ(-102.0 * CLHEP::cm);       // Z of back container front
    const double k_forwConFarZ(239.8 * CLHEP::cm);         // Z of forw container back
    const double k_forwConNearZ(196.0 * CLHEP::cm);        // Z of forw container front
    const double k_backConNearInnerR(45.0705 * CLHEP::cm); // inner R of back cont (near Z)
    const double k_backConFarInnerR(67.9125 * CLHEP::cm);  // inner R of back cont (far  Z)
    const double k_forwConNearInnerR(40.93  * CLHEP::cm);  // inner R of forw cont (near Z)
    const double k_forwConFarInnerR(51.1387 * CLHEP::cm);  // inner R of forw cont (far  Z)

    const double k_endConInnerThick(2.0   * CLHEP::cm);    // thickness of inner cone
    const double k_endConSiWallThick(0.16 * CLHEP::cm);    // thickness of outer side wall
    const double k_endConOuterThick(2.0  * CLHEP::cm);     // thickness of outer tube
    const double k_backConNearOuterR(119.015 * CLHEP::cm); // outer R of cont at near Z
    const double k_backConFarOuterR(149.6 * CLHEP::cm);    // outer R of cont at far Z
    const double k_backConHflat(20.267 * CLHEP::cm);       // Zlength of "flat" outer radius
    const double k_forwConNearOuterR(120.04 * CLHEP::cm);  // outer R of cont at near Z
    const double k_forwConFarOuterR(141.5  * CLHEP::cm);   // outer R of cont at far Z
    const double k_forwConHflat(10.72 * CLHEP::cm);        // Zlength of "flat" outer radius
    const double k_endConNearThick(0.3   * CLHEP::cm);     // thickness of front of con
    const double k_endConFarThick(0.1   * CLHEP::cm);      // thickness of back of con
    const double k_endConFinThick(0.05  * CLHEP::cm);      // ec's phi fin thickness


    const double k_barConBackRThick(10.5 * CLHEP::cm);     // thickness of sup ring in back
    const double k_barConBackZOut(-122.50 * CLHEP::cm);    // Z of container back at outer R
    const double k_barConEndRngThick(0.2 * CLHEP::cm);     // endring thickess, both ends

    const double k_barConForwZOut(229.0 * CLHEP::cm);      // Z of container forw at outer R
    const double k_barConForwRThick(9.0 * CLHEP::cm);      // thickness of sup ring in forw
    const double k_barConInnerR(125.01 * CLHEP::cm);       // inner R of barrel container
    const double k_barCryBackZOut(k_barConBackZOut + k_barConEndRngThick) ;
    const double k_barSupBackFarZ(k_barConBackZOut - k_barConBackRThick) ;
    const double k_barSupForwFarZ(k_barConForwZOut + k_barConForwRThick) ;
    const double k_c1z3(k_barSupForwFarZ + 31.*CLHEP::cm);
    const double k_c2z1(k_c1z3 - 444.0 * CLHEP::cm);
    const double k_c2z2(k_c2z1 + 1.0 * CLHEP::cm);

    const double k_c1r1(167.0 * CLHEP::cm);
    const double k_l1r4(k_c1r1 - 0.2 * CLHEP::cm);
    const double k_l2r3(k_l1r4);


    // add foil thickness //
    const double foilthickness = 0.0100 * CLHEP::cm; // crystal wrapping foil 100 um
    const double thinfoilthickness = foilthickness * 0.8; // thin crystal wrapping foil 80 um
    const double avoidov = 1 + 1E-6; // foil inside is a little bit lager than crystal to avoid overlap
    ///////////////////////



    CsiCreator::CsiCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    CsiCreator::~CsiCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void CsiCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {

      if (type) {}

      //
      // Load materials
      // Note to AB: someday change this
      // to use materials from the .xml file
      //
      ///////////////////////////////////////
      G4Material* medAir = geometry::Materials::get("ColdAir");
      G4Material* medCsI = geometry::Materials::get("CsI");
      G4Material* medSi = geometry::Materials::get("Si");
      G4Material* medAlTeflon = geometry::Materials::get("AlTeflon");
      G4Material* medAlTeflon_thin = geometry::Materials::get("AlTeflon_thin");


      // Diodes. Obsolete, but maybe relevent in the future..
      ///////////////////////////////////////////////////////
      double DiodeWidth = content.getLength("k_diodewidth") * CLHEP::cm;
      double DiodeLength = content.getLength("k_diodelength") * CLHEP::cm;
      double DiodeHeight = content.getLength("k_diodeheight") * CLHEP::cm;
      G4Box* SensorDiode = new G4Box("diode", DiodeWidth / 2, DiodeLength / 2, DiodeHeight / 2);


      double h1, h2, bl1, bl2, tl1, tl2, alpha1, alpha2, Rphi1, Rphi2, Rtheta, Pr, Ptheta, Pphi, halflength;


      //
      // Define CsI Envelope
      // To o: get rid of that, and make an envelope per box,
      // fill it with all possible  combinaitions of crystals, and
      ////////////////////////

      double eclWorld_I[6] = {452, 452, 1250, 1250, 395, 395};//unit:mm
      double eclWorld_O[6] = {1640, 1640, 1640, 1640, 1640, 1640};//unit:mm
      double eclWorld_Z[6] = { -1450, -1010, -1010, 1960, 1960, 2400};//unit:mm
      G4Polycone* eclWorld = new G4Polycone("eclWorld", 0, 2 * PI, 6, eclWorld_Z, eclWorld_I, eclWorld_O);
      logical_ecl = new G4LogicalVolume(eclWorld, geometry::Materials::get("G4_Galactic"), "logical_ecl");
      physical_ecl = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logical_ecl, "physicalECL", &topVolume, false, 0);

      /*
      // Instantiation of a set of visualization attributes with white 100% transparent colour.
      G4VisAttributes * csiWorldVisAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.0));
      // Set the forced wireframe style
      csiWorldVisAtt->SetForceWireframe(true);
      */
      // Assignment of the visualization attributes to the logical volume
      logical_ecl->SetVisAttributes(G4VisAttributes::GetInvisible());


      // Create sub-assebmlies
      // NOTE: the order is important for getting the right names for
      // the physical volumes. Crystals first!!

      G4AssemblyVolume* assemblyFwCrystals = new G4AssemblyVolume();
      G4AssemblyVolume* assemblyFwFoils = new G4AssemblyVolume();
      ///////////////////////////////////////////////////////////



      //
      //  FORWARD END-CAP CRYSTALS
      //  (does backward too now...)
      ///////////////////////////////

      int iRing = 0;
      int nRing = 0;
      int iPhi = 0;

      //      for (int iCry = 1 ; iCry <= 72 ; ++iCry) {
      for (int iCry = 1 ; iCry <= 4 ; ++iCry) {


        GearDir counter(content);
        counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

        /*
              //for quick tests!
              if (iCry > 2) {
          iCry = iCry - 2 + 100;
              }
        */

        h1 = counter.getLength("K_h1") * CLHEP::cm;
        h2 = counter.getLength("K_h2") * CLHEP::cm;
        bl1 = counter.getLength("K_bl1") * CLHEP::cm;
        bl2 = counter.getLength("K_bl2") * CLHEP::cm;
        tl1 = counter.getLength("K_tl1") * CLHEP::cm;
        tl2 = counter.getLength("K_tl2") * CLHEP::cm;
        alpha1 = counter.getAngle("K_alpha1");
        alpha2 = counter.getAngle("K_alpha2");
        Rphi1 = counter.getAngle("K_Rphi1") ;
        Rtheta = counter.getAngle("K_Rtheta") ;
        Rphi2 = counter.getAngle("K_Rphi2")  ;
        Pr = counter.getLength("K_Pr") * CLHEP::cm;
        Ptheta = counter.getAngle("K_Ptheta") ;
        Pphi = counter.getAngle("K_Pphi") ;
        halflength = 15.0 * CLHEP::cm;

        double fwfoilthickness = foilthickness;

        // add forward foil dimensions ////////////////////////////
        double fwtrapangle1 = atan(2 * h1 / (bl1 - tl1)); // the smaller angle of the trap
        double fwtrapangle2 = atan(2 * h2 / (bl2 - tl2));
        double fwfoilh1 = h1 + fwfoilthickness;
        double fwfoilh2 = h2 + fwfoilthickness;
        double fwfoiltl1 = tl1 + fwfoilthickness * tan(fwtrapangle1 / 2);
        double fwfoilbl1 = bl1 + fwfoilthickness / tan(fwtrapangle1 / 2);
        double fwfoiltl2 = tl2 + fwfoilthickness * tan(fwtrapangle2 / 2);
        double fwfoilbl2 = fwfoiltl2 + (fwfoilbl1 - fwfoiltl1) * fwfoilh2 / fwfoilh1;
        double fwfoilhalflength = halflength + fwfoilthickness;
        ///////////////////////////////////////////////////////////


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

        G4VisAttributes* FwCrystalVisAtt = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0));
        FwCrysral->SetVisAttributes(FwCrystalVisAtt);
        FwCrysral->SetSensitiveDetector(m_sensitive);


        assemblyFwCrystals->AddPlacedVolume(FwCrysral, Tr);

        /////////////////  add forward foil ///////////////////////////////////////

        G4Trap* FwFoilout = new G4Trap((format("FwFoilout_%1%") % iCry).str().c_str(),
                                       fwfoilhalflength , 0 , 0, fwfoilh1,  fwfoilbl1,
                                       fwfoiltl1, alpha1 , fwfoilh2, fwfoilbl2,
                                       fwfoiltl2, alpha2);

        G4Trap* FwFoilin = new G4Trap((format("solidEclFwCrystal_%1%") % iCry).str().c_str(),
                                      halflength * avoidov , 0 , 0, h1 * avoidov , bl1 * avoidov, tl1 * avoidov , alpha1 , h2 * avoidov   , bl2 * avoidov, tl2 * avoidov, alpha2);
        G4SubtractionSolid* FwFoilShape = new G4SubtractionSolid((format("FwFoil_%1%") % iCry).str().c_str(), FwFoilout, FwFoilin);

        G4LogicalVolume* FwFoil = new G4LogicalVolume(FwFoilShape, medAlTeflon, (format("logicalFwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
        G4VisAttributes* FwFoilVisAtt = new G4VisAttributes(G4Colour(0.4, 0.4, 0.8, 0.5));
        FwFoil->SetVisAttributes(FwFoilVisAtt);

        assemblyFwFoils->AddPlacedVolume(FwFoil, Tr);
        //////////////////////////////////////////////////////////////////////////////////
      }//end forward endcap crystals


      int nSector = 4; //16
      for (int iSector = 0; iSector < nSector; ++iSector) {//total nSector
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / nSector * CLHEP::deg);
        assemblyFwCrystals->MakeImprint(logical_ecl, BrR);
        assemblyFwFoils->MakeImprint(logical_ecl, BrR);  // foil
      }//nSector sectior

      // Diags
      B2INFO("Volumes names for the CsI crystals");
      B2INFO("  Forward: ");
      unsigned int i = 0;
      G4VPhysicalVolume* MyVolume;

      // Show cell IDs and volume names
      int cid;
      for (std::vector<G4VPhysicalVolume*>::iterator it = assemblyFwCrystals->GetVolumesIterator();
           i !=  assemblyFwCrystals->TotalImprintedVolumes();
           ++it, ++i) {

        MyVolume = *it;
        B2INFO("   Crystal Name " << MyVolume->GetName());
        CsiGeometryPar* eclp = CsiGeometryPar::Instance();
        cid = eclp->CsiVolNameToCellID(MyVolume->GetName());
        B2INFO("   Crystal Number " << cid);
      }


    }//create

  } // csi namespace
} // Belle2 namespace
