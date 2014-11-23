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

    // add foil thickness //
    const double foilthickness = 0.0100 * CLHEP::cm; /**< Crystal wrapping foil 100 um */
    const double avoidov = 1 + 1E-6; /**< foil inside is a little bit lager than crystal to avoid overlap */
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
      // Instanciate CsI Envelope
      // To do: get rid of that, and make an envelope per crystal box, fill it with
      // all possible  combinaitions of  crystals, and then copy it to all positions.
      ///////////////////////////////////////////////////////////////////////////////

      double eclWorld_I[6] = {452, 452, 1250, 1250, 395, 395};//unit:mm
      double eclWorld_O[6] = {1640, 1640, 1640, 1640, 1640, 1640};//unit:mm
      double eclWorld_Z[6] = { -1450, -1010, -1010, 1960, 1960, 2400};//unit:mm
      G4Polycone* eclWorld = new G4Polycone("eclWorld", 0, 2 * PI, 6, eclWorld_Z, eclWorld_I, eclWorld_O);

      /** The CsI Logical Volume */
      G4LogicalVolume* logical_ecl = new G4LogicalVolume(eclWorld, geometry::Materials::get("G4_Galactic"), "logical_ecl");
      logical_ecl->SetVisAttributes(G4VisAttributes::GetInvisible());


      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logical_ecl, "physicalECL", &topVolume, false, 0);

      //
      // Load materials
      // Note to AB: someday change this
      // to use materials from the .xml file
      ///////////////////////////////////////
      G4Material* medCsI = geometry::Materials::get("CsI");
      G4Material* medAlTeflon = geometry::Materials::get("AlTeflon");



      // Create sub-assebmlies
      // NOTE: the order is important for getting the right names for
      // the physical volumes. Crystals first!!
      G4AssemblyVolume* assemblyFwCrystals = new G4AssemblyVolume();
      G4AssemblyVolume* assemblyFwFoils = new G4AssemblyVolume();
      ///////////////////////////////////////////////////////////



      //
      //  "END-CAP:-LIKE  CRYSTALS
      ///////////////////////////////
      //double h1, h2, bl1, bl2, tl1, tl2, alpha1, alpha2, Rphi1, Rphi2, Rtheta, Pr, Ptheta, Pphi, halflength;

      for (int iCry = 1 ; iCry <= 4 ; ++iCry) {


        GearDir counter(content);
        counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

        double h1 = counter.getLength("K_h1") * CLHEP::cm;
        double h2 = counter.getLength("K_h2") * CLHEP::cm;
        double bl1 = counter.getLength("K_bl1") * CLHEP::cm;
        double bl2 = counter.getLength("K_bl2") * CLHEP::cm;
        double tl1 = counter.getLength("K_tl1") * CLHEP::cm;
        double tl2 = counter.getLength("K_tl2") * CLHEP::cm;
        double alpha1 = counter.getAngle("K_alpha1");
        double alpha2 = counter.getAngle("K_alpha2");
        double Rphi1 = counter.getAngle("K_Rphi1") ;
        double Rtheta = counter.getAngle("K_Rtheta") ;
        double Rphi2 = counter.getAngle("K_Rphi2")  ;
        double Pr = counter.getLength("K_Pr") * CLHEP::cm;
        double Ptheta = counter.getAngle("K_Ptheta") ;
        double Pphi = counter.getAngle("K_Pphi") ;
        double halflength = 15.0 * CLHEP::cm;


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

        G4VisAttributes* FwCrystalVisAtt = new G4VisAttributes(G4Colour(18.0 / 256, 230.0 / 256, 3.0 / 256, 1.0));
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


        //Hide the foils for now...
        FwFoil->SetVisAttributes(G4VisAttributes::GetInvisible());

        assemblyFwFoils->AddPlacedVolume(FwFoil, Tr);
        //////////////////////////////////////////////////////////////////////////////////
      }//end forward endcap crystals


      int nSector = 4; //16
      for (int iSector = 0; iSector < nSector; ++iSector) {//total nSector
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / nSector * CLHEP::deg);
        assemblyFwCrystals->MakeImprint(logical_ecl, BrR);
        assemblyFwFoils->MakeImprint(logical_ecl, BrR);  // foil
      }//nSector sector

      // Diags
      B2INFO("Volume names for the CsI crystals");
      unsigned int i = 0;

      // Show cell IDs and volume names
      for (std::vector<G4VPhysicalVolume*>::iterator it = assemblyFwCrystals->GetVolumesIterator();
           i !=  assemblyFwCrystals->TotalImprintedVolumes();
           ++it, ++i) {

        G4VPhysicalVolume* MyVolume = *it;
        B2INFO("   Crystal Name " << MyVolume->GetName());
        CsiGeometryPar* eclp = CsiGeometryPar::Instance();
        int cid = eclp->CsiVolNameToCellID(MyVolume->GetName());
        B2INFO("   Crystal Number " << cid);
      } // for all physical volumes in the assembly
    }// create
  } // csi namespace
} // Belle2 namespace
