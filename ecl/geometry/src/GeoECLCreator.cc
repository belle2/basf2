/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>


#include <CLHEP/Geometry/Transform3D.h>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>


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

// add include subtraction /////
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>

#include <G4VisAttributes.hh>
////////////////////////////////

#include <iostream>



#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>

#define PI 3.14159265358979323846


using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace ECL {
//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    geometry::CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    G4Transform3D Global_offset;
    const int
    k_barNPhiSegs(144) ,
                  k_forwNPhiSegs(16) ,
                  k_backNPhiSegs(16) ;

    const EclIdentifier
    k_barNRings(46) ,
                k_forwNRings(13) ,
                k_backNRings(10) ;

    const double k_CLR(0.01   * cm);                 // clearance around outside
//const double k_barCryPullOut    (  0.5   *cm ); // Barrel Crystal pullout
    const double k_barFinThick(0.05  * cm);         // fin thickness in barrel
    const double k_barConOuterR(166.0 * cm);        // outer R of barrel container
    const double k_barConOuterThick(0.8 * cm);      // thickness (eff) of outer skin
    const double k_barConInnerR(125.01 * cm);       // inner R of barrel container
    const double k_barConInnerThick(0.15 * cm);     // thickness of inner skin
    const double k_barConBackZOut(-122.50 * cm);    // Z of container back at outer R
    const double k_barConBackZIn(-100.58 * cm);     // Z of container back at inner R
    const double k_barConForwZIn(199.1 * cm);       // Z of container forw at inner R
    const double k_barConForwZOut(229.0 * cm);      // Z of container forw at outer R
    const double k_barConEndConThick(0.4 * cm);     // endcone thickess, both ends
    const double k_barConEndRngThick(0.2 * cm);     // endring thickess, both ends
    const double k_barConBackRThick(10.5 * cm);     // thickness of sup ring in back
    const double k_barConForwRThick(9.0 * cm);      // thickness of sup ring in forw
    const double k_backConFarZ(-145.5 * cm);        // Z of back container back
    const double k_backConNearZ(-102.0 * cm);       // Z of back container front
    const double k_forwConFarZ(239.8 * cm);         // Z of forw container back
    const double k_forwConNearZ(196.0 * cm);        // Z of forw container front
    const double k_backConNearInnerR(45.0705 * cm); // inner R of back cont (near Z)
    const double k_backConFarInnerR(67.9125 * cm);  // inner R of back cont (far  Z)
    const double k_forwConNearInnerR(40.93  * cm);  // inner R of forw cont (near Z)
    const double k_forwConFarInnerR(51.1387 * cm);  // inner R of forw cont (far  Z)

    const double k_endConInnerThick(2.0   * cm);    // thickness of inner cone
    const double k_endConSiWallThick(0.16 * cm);    // thickness of outer side wall
    const double k_endConOuterThick(2.0  * cm);     // thickness of outer tube
    const double k_backConNearOuterR(119.015 * cm); // outer R of cont at near Z
    const double k_backConFarOuterR(149.6 * cm);    // outer R of cont at far Z
    const double k_backConHflat(20.267 * cm);       // Zlength of "flat" outer radius
    const double k_forwConNearOuterR(120.04 * cm);  // outer R of cont at near Z
    const double k_forwConFarOuterR(141.5  * cm);   // outer R of cont at far Z
    const double k_forwConHflat(10.72 * cm);        // Zlength of "flat" outer radius
    const double k_endConNearThick(0.3   * cm);     // thickness of front of con
    const double k_endConFarThick(0.1   * cm);      // thickness of back of con
    const double k_endConFinThick(0.05  * cm);      // ec's phi fin thickness
//const double k_endConMetThick   (  1.34  *cm ) ; // ec's metal fitting thickness

//  const double k_barCryPhiAng  (  2.500*deg ) ; // crystal azimuthal width
//  const double k_barPhiTiltAng (  1.250*deg ) ; // crystal tilt in phi
    const double k_barBackAng(52.902 * deg) ;     // theta at container back
    const double k_barForwAng(32.987 * deg) ;     // theta at container front

    const double k_barBackSin(sin(k_barBackAng)) ;
    const double k_barBackCos(cos(k_barBackAng)) ;
    const double k_barBackTan(tan(k_barBackAng)) ;
    const double k_barForwSin(sin(k_barForwAng)) ;
    const double k_barForwCos(cos(k_barForwAng)) ;
    const double k_barForwTan(tan(k_barForwAng)) ;

    const double k_barCryOuterR(k_barConOuterR - k_barConOuterThick) ;

    const double k_barCryInnerR(k_barConInnerR + k_barConInnerThick) ;

    const double k_barCryH(32.0 * cm) ;  // rough
    const double k_barCryMargin(.8 * cm) ;  // rough, work
    const double k_barCryOuterQ(k_barCryInnerR + k_barCryH) ;


    const double k_barCryBackZOut(k_barConBackZOut + k_barConEndRngThick) ;
    const double k_barCryForwZOut(k_barConForwZOut - k_barConEndRngThick) ;
    const double k_barSupBackFarZ(k_barConBackZOut - k_barConBackRThick) ;
    const double k_barSupForwFarZ(k_barConForwZOut + k_barConForwRThick) ;
    const double k_barSupForwFarIR(k_barConInnerR +
                                   (k_barSupForwFarZ - k_barConForwZIn)*
                                   k_barForwTan) ;
//  const double k_barCryBackZIn  ( k_barConBackZIn + k_barFinThick/k_barBackSin );
//  const double k_barCryForwZIn  ( k_barConForwZIn - k_barFinThick/k_barForwSin );
    const double k_barConBackR5(k_barCryInnerR + k_barFinThick / k_barBackCos) ;
    const double k_barConForwR5(k_barCryInnerR + k_barFinThick / k_barForwCos) ;
    const double k_barConTFC(k_barFinThick + k_barConEndConThick) ;
    const double k_barConBackRg(k_barConTFC / k_barBackCos) ;
    const double k_barConBackR4(k_barConInnerR + k_barConBackRg) ;
    const double k_barConForwRg(k_barConTFC / k_barForwCos) ;
    const double k_barConForwR4(k_barConInnerR + k_barConForwRg) ;
    const double k_barConBackZ3(k_barConBackZIn -
                                (k_barConTFC *
                                 (k_barBackSin + k_barBackCos / k_barBackTan) -
                                 (k_barFinThick / k_barBackCos +
                                  k_barConInnerThick) / k_barBackTan)) ;
    const double k_barConForwZ3(k_barConForwZIn +
                                (k_barConTFC *
                                 (k_barForwSin + k_barForwCos / k_barForwTan) -
                                 (k_barFinThick / k_barForwCos +
                                  k_barConInnerThick) / k_barForwTan)) ;
    const double k_barConBackR2(k_barCryInnerR + k_barBackTan *
                                (k_barConBackZ3 - k_barCryBackZOut)) ;
    const double k_barConForwR2(k_barCryInnerR - k_barForwTan *
                                (k_barConForwZ3 - k_barCryForwZOut)) ;
    const double k_barConBackR3(k_barConBackR2 + k_barConBackRg) ;
    const double k_barConForwR3(k_barConForwR2 + k_barConForwRg) ;
    const double k_barConBackR1(k_barConBackR2 +
                                k_barConEndRngThick* k_barBackTan) ;
    const double k_barConForwR1(k_barConForwR2 +
                                k_barConEndRngThick* k_barForwTan) ;



    const double k_c1z3(k_barSupForwFarZ + 31.*cm);
    const double k_c1z2(k_c1z3 - 1.*cm);
    const double k_c1z1(k_c1z3 - 75.*cm);
    const double k_c1r1(167.0 * cm);
    const double k_c1r2(k_c1r1 + 2.0 * cm);
    const double k_c1r3(k_c1r1 + 32.0 * cm);

    const double k_c2z1(k_c1z3 - 444.0 * cm);
    const double k_c2z2(k_c2z1 + 1.0 * cm);
    const double k_c2z3(k_c2z1 + 75.*cm);
    const double k_c2r1(k_c1r1);
    const double k_c2r2(k_c1r2);
    const double k_c2r3(k_c1r3);

    const double k_l1z1(k_barSupForwFarZ);
    const double k_l1z2(k_l1z1 + 3.5 * cm);
    const double k_l1z3(k_l1z1 + 10.0 * cm);
    const double k_l1r4(k_c1r1 - 0.2 * cm);
    const double k_l1r3(k_l1r4 - 3.5 * cm);
    const double k_l1r1(k_l1r4 - 15.8 * cm);
    const double k_l1r2(k_l1r1 + 2.45 * cm);
    const double k_l1ang(6.3 * cm / ((k_l1r1 + k_l1r4) / 2.0));

    const double k_l2z3(k_barSupBackFarZ);
    const double k_l2z2(k_l2z3 - 3.5 * cm);
    const double k_l2z1(k_l2z3 - 12.0 * cm);
    const double k_l2r3(k_l1r4);
    const double k_l2r2(k_l2r3 - 3.5 * cm);
    const double k_l2r1(k_l2r3 - 11.8 * cm);
    const double k_l2ang(9.0 * cm / ((k_l2r1 + k_l2r3) / 2.0));

    // add foil thickness //
    const double foilthickness = 0.0100 * cm; // crystal wrapping foil 100 um
    const double thinfoilthickness = foilthickness * 0.8; // thin crystal wrapping foil 80 um
    const double thinpentafoilthickness = foilthickness * 0.2; // pentagon crystal foil
    const double brthetafinthickness = 0.0500 * cm; // barrel theta fin 500 um
    const double brphifinthickness = 0.0500 * cm; // barrel phi fin 500 um
    const double avoidov = 1 + 1E-6; // foil inside is a little bit lager than crystal to avoid overlap
    ///////////////////////

    GeoECLCreator::GeoECLCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector("SensitiveDetector", (2 * 24)*eV, 10 * MeV);
      logical_ecl = 0;
      physical_ecl = 0;
      physical_ECLBarrelCylinder = 0;
    }


    GeoECLCreator::~GeoECLCreator()
    {
      delete m_sensitive;

    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {

      if (type) {}

      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");
      string CsI  = content.getString("CsI");
      G4Material* medCsI = geometry::Materials::get(CsI.c_str());
      G4Material* medAir = geometry::Materials::get("Air");
      string Si  = content.getString("Si");
      G4Material* medSi = geometry::Materials::get(Si.c_str());
      // add AlTeflon //////////////////////////////
      string AlTeflon  = content.getString("AlTeflon");
      G4Material* medAlTeflon = geometry::Materials::get(AlTeflon.c_str());
      string AlTeflon_thin  = content.getString("AlTeflon_thin");
      G4Material* medAlTeflon_thin = geometry::Materials::get(AlTeflon_thin.c_str());
      string AlTeflon_thinpenta  = content.getString("AlTeflon_thinpenta");
      G4Material* medAlTeflon_thinpenta = geometry::Materials::get(AlTeflon_thinpenta.c_str());
      //////////////////////////////////////////////
      // add AlTeflon //////////////////////////////
      string Al  = content.getString("Al");
      G4Material* medAl = geometry::Materials::get(Al.c_str());
      //////////////////////////////////////////////

//      double eclWorld_I[6] = {452, 452, 1250, 1250, 395, 395};//unit:mm
//      double eclWorld_O[6] = {1640, 1640, 1640, 1640, 1640, 1640};//unit:mm
//      double eclWorld_Z[6] = { -1450, -1010, -1010, 1960, 1960, 2400};//unit:mm



      double eclWorld_I[12] ;//unit:mm
      double eclWorld_O[12] ;//unit:mm
      double eclWorld_Z[12] ;//unit:mm

//for outer support
      eclWorld_Z[0] = k_c2z1 + k_CLR; // reverse clearance
      eclWorld_I[0] = k_l2r3 - 36.0 * cm  - k_CLR;
      eclWorld_O[0] = k_l2r3 + k_CLR;
      eclWorld_Z[1] = k_backConFarZ - k_CLR;
      eclWorld_I[1] = k_l2r3 - 36.0 * cm  - k_CLR;
      eclWorld_O[1] = k_l2r3 + k_CLR;
//45
      eclWorld_Z[2] = k_backConFarZ - k_CLR;
      eclWorld_I[2] = k_backConFarInnerR  - k_CLR;
      eclWorld_O[2] = k_l2r3 + k_CLR  ;
//67
      eclWorld_Z[3] = k_backConNearZ + k_CLR;
      eclWorld_I[3] = k_backConNearInnerR - k_CLR;
      eclWorld_O[3] = k_l2r3 + k_CLR  ;
      eclWorld_Z[4] = k_backConNearZ + k_CLR;
      eclWorld_I[4] = k_barConInnerR      - k_CLR;
      eclWorld_O[4] = k_l2r3 + k_CLR  ;
//89
      eclWorld_Z[5] = k_forwConNearZ - k_CLR;
      eclWorld_I[5] = k_barConInnerR      - k_CLR;
      eclWorld_O[5] = k_l2r3 + k_CLR  ;
      eclWorld_Z[6] = k_forwConNearZ - k_CLR;
      eclWorld_I[6] = k_forwConNearInnerR - k_CLR;
      eclWorld_O[6] = k_l2r3 + k_CLR  ;
//1011
      eclWorld_Z[7] = k_forwConFarZ + k_CLR;
      eclWorld_I[7] = k_forwConFarInnerR  - k_CLR;
      eclWorld_O[7] = k_l2r3 + k_CLR ;
//for outer support
      eclWorld_Z[8] = k_forwConFarZ + k_CLR;
      eclWorld_I[8] = k_l2r3  - 42.0 * cm - k_CLR;
      eclWorld_O[8] = k_l2r3 + k_CLR;
      eclWorld_Z[9] = k_forwConFarZ + 6.0 * cm + k_CLR;
      eclWorld_I[9] = k_l2r3  - 42.0 * cm - k_CLR;
      eclWorld_O[9] = k_l2r3 + k_CLR;
      eclWorld_Z[10] = k_forwConFarZ + 6.0 * cm + k_CLR;
      eclWorld_I[10] = k_l2r3  - 42.0 * cm + 3.0 * cm - k_CLR;
      eclWorld_O[10] = k_l2r3 + k_CLR;
      eclWorld_Z[11] = k_c1z3 - k_CLR; // reverse clearance
      eclWorld_I[11] = k_l2r3  - 42.0 * cm + 3.0 * cm - k_CLR;
      eclWorld_O[11] = k_l2r3 + k_CLR;



      //define geometry
      G4Polycone* eclWorld = new G4Polycone("eclWorld", 0, 2 * PI, 12, eclWorld_Z, eclWorld_I, eclWorld_O);
      logical_ecl = new G4LogicalVolume(eclWorld, medAir, "logical_ecl");
      physical_ecl = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logical_ecl, "physicalECL", &topVolume, false, 0);

      double DiodeWidth = content.getLength("k_diodewidth") * cm;
      double DiodeLength = content.getLength("k_diodelength") * cm;
      double DiodeHeight = content.getLength("k_diodeheight") * cm;

      G4Box* SensorDiode = new G4Box("diode", DiodeWidth / 2, DiodeLength / 2, DiodeHeight / 2);


      /*
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
      double k_phi_init; // add correction
      */
      double zsign = 1.;

      // add barrel fin parameters and previous crystal parameters/////////
      double brfink_BLL = 30 * cm;
      double brfink_Ba = 5.387 * cm;
      double brfink_Bb = 5.5868 * cm;
      double brfink_BA = 6.0997 * cm;
      double brfink_Bh = 5.4504 * cm;
      double brfink_z_TILTED = 33.667039 * deg;
      double brfink_phi_TILTED = 1.149806 * deg;
      double brfink_perpC = 136.08246 * cm;
      double brfink_phiC = 1.344548 * cm;
      double brfink_zC = 210.1509 * cm;
      double brfink_phi_init = 0.0094779 * deg; // add correction


      double  prevk_Ba = 5.387 * cm;
      double  prevk_BA =  6.0997 * cm;
      double  prevk_Bh =  5.4504 * cm;
      double  prevk_BH = 6.0997 * cm;;
      double  prevk_z_TILTED =  33.667039 * deg;
      double  prevk_phi_TILTED = 1.149806 * deg;
      double  prevk_perpC = 136.08246 * cm;
      double  prevk_phiC = 1.344548 * cm;
      double  prevk_zC = 210.1509 * cm; ;
      double  prevk_phi_init =  0.0094779 * deg ;
      //////////////////////////////////////////////////////////////////////


      G4AssemblyVolume* assemblyBrCrystals = new G4AssemblyVolume();
      G4AssemblyVolume* assemblyBrDiodes = new G4AssemblyVolume();
      // add assembly barrel foil ///////////////////////////////////
      G4AssemblyVolume* assemblyBrFoils = new G4AssemblyVolume();
      ////////////////////////////////////////////////////////
      // add assembly barrel fin ///////////////////////////////////
      G4AssemblyVolume* assemblyBrFins = new G4AssemblyVolume(); // fins in different theta
      G4AssemblyVolume* assemblyBrPhiFins = new G4AssemblyVolume(); // big fins in different phi with same theta range
      ////////////////////////////////////////////////////////

      int nBarrelCrystal = content.getNumberNodes("BarrelCrystals/BarrelCrystal");

      for (int iBrCry = 1 ; iBrCry <= nBarrelCrystal ; ++iBrCry) {
        {
          //46=29+17

          GearDir layerContent(content);
          layerContent.append((format("/BarrelCrystals/BarrelCrystal[%1%]/") % (iBrCry)).str());


          double k_BLL = layerContent.getLength("K_BLL") * cm;
          double k_Ba = layerContent.getLength("K_Ba")   * cm;
          double k_Bb = layerContent.getLength("K_Bb")   * cm;
          double k_Bh = layerContent.getLength("K_Bh")   * cm;
          double k_BA = layerContent.getLength("K_BA")   * cm;
          double k_BH = layerContent.getLength("K_BH")   * cm;
          double k_z_TILTED = layerContent.getAngle("K_z_TILTED") ;
          double k_phi_TILTED = layerContent.getAngle("K_phi_TILTED") ;
          double k_perpC = layerContent.getLength("K_perpC") * cm ;
          double k_phiC = layerContent.getAngle("K_phiC") ;
          double k_zC = layerContent.getLength("K_zC") * cm;
          double k_phi_init = layerContent.getAngle("K_phi_init") ; // add correction


          double cDx1 = k_Ba / 2;
          double cDx2 = k_Bb / 2;
          double cDy1 = k_Bh / 2;
          double cDx3 = k_BA / 2;
          double cDy2 = k_BH / 2;
          double cDz = k_BLL / 2;
          double cDx4 = (cDx3 * cDy1 + (cDx2 - cDx1) * cDy2) / cDy1; // replaced by coplanarity

          /*// add barrel foil dimensions ////////////////////////
          double brfratio = 1 + foilthickness / cDx4;
          double brfoilcDx1 = cDx1 * brfratio;
          double brfoilcDx2 = cDx2 * brfratio;
          double brfoilcDy1 = cDy1 * brfratio;
          double brfoilcDx3 = cDx3 * brfratio;
          double brfoilcDy2 = cDy2 * brfratio;
          double brfoilcDx4 = cDx4 * brfratio;
          *////////////////////////////////////////////////

          // add barrel foil dimensions a///////////////////////
          double brtrapangle1 = atan(2 * cDy1 / (cDx2 - cDx1)); // the smaller angle of the trap
          double brtrapangle2 = atan(2 * cDy2 / (cDx4 - cDx3));
          double brfoilcDz = cDz + foilthickness;
          double brfoilcDx1 = cDx1 + foilthickness * tan(brtrapangle1 / 2);
          double brfoilcDx2 = cDx2 + foilthickness / tan(brtrapangle1 / 2);
          double brfoilcDy1 = cDy1 + foilthickness;
          double brfoilcDx3 = cDx3 + foilthickness * tan(brtrapangle2 / 2);
          double brfoilcDy2 = cDy2 + foilthickness;
          double brfoilcDx4 = brfoilcDx3 + (brfoilcDx2 - brfoilcDx1) * brfoilcDy2 / brfoilcDy1;
          ///////////////////////////////////////////////

          if (iBrCry >= 30) {zsign = -1;}
          G4Transform3D r00 = G4RotateZ3D(90.*zsign * deg);
          G4Transform3D phi_init = G4RotateZ3D(k_phi_init);
          G4Transform3D tilt_z = G4RotateY3D(k_z_TILTED);
          G4Transform3D tilt_phi = G4RotateZ3D(k_phi_TILTED);
          G4Transform3D position = G4Translate3D(k_perpC, 0, k_zC);  // Move over to the left...
          G4Transform3D pos_phi = G4RotateZ3D(k_phiC);
          G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * r00 * phi_init;

          /*
                      double vtx[15];
                      vtx[0] = cDx1; vtx[1] = cDy1; G4Point3D tmp3( cDx1, cDy1,-cDz);
                      vtx[2] = cDx2; vtx[3] = -cDy1; G4Point3D tmp2( cDx2,-cDy1,-cDz);
                      vtx[4] = -cDx2; vtx[5] = -cDy1; G4Point3D tmp1(-cDx2,-cDy1,-cDz);
                      vtx[6] = -cDx1; vtx[7] = cDy1; G4Point3D tmp0(-cDx1, cDy1,-cDz);
                      vtx[8] = cDx3; vtx[9] = cDy2; G4Point3D tmp7( cDx3, cDy2, cDz);
                      vtx[10] = cDx4; vtx[11] = -cDy2; G4Point3D tmp6( cDx4,-cDy2, cDz);
                      vtx[12] = -cDx4; vtx[13] = -cDy2; G4Point3D tmp5(-cDx4,-cDy2, cDz);
                      vtx[14] = -cDx3; vtx[15] = cDy2; G4Point3D tmp4(-cDx3, cDy2, cDz);
          */

          G4Trap* BrCrysralShape = new G4Trap((format("solidEclBrCrystal_%1%") % iBrCry).str().c_str(),
                                              cDz , 0 , 0, cDy1, cDx2, cDx1, 0, cDy2 , cDx4, cDx3, 0);
          G4LogicalVolume* BrCrysral = new G4LogicalVolume(BrCrysralShape, medCsI, (format("logicalEclBrCrystal_%1%") % iBrCry).str().c_str(), 0, 0, 0);
          BrCrysral->SetSensitiveDetector(m_sensitive);
          assemblyBrCrystals->AddPlacedVolume(BrCrysral, Tr);


          //  add barrel foil /////////////////////////////////////////////

          G4Trap* BrFoilout = new G4Trap((format("BrFoilout_%1%") % iBrCry).str().c_str(),
                                         brfoilcDz , 0 , 0, brfoilcDy1, brfoilcDx2, brfoilcDx1, 0,
                                         brfoilcDy2, brfoilcDx4, brfoilcDx3, 0);

          G4SubtractionSolid* BrFoilShape = new G4SubtractionSolid((format("BrFoil_%1%") % iBrCry).str().c_str(), BrFoilout, BrCrysralShape);

          G4LogicalVolume* BrFoil = new G4LogicalVolume(BrFoilShape, medAlTeflon, (format("logicalBrFoil_%1%") % iBrCry).str().c_str(), 0, 0, 0);
          //BrFoil->SetSensitiveDetector(m_sensitive);
          assemblyBrFoils->AddPlacedVolume(BrFoil, Tr);

          /////////////////////////////////////////////////////////////////////////


          // add barrel theta fin //////////////////////////////////////////////////////////////////////////


          if (iBrCry % 4 == 1) { // store the parameters of crystals before the fin (previous)
            prevk_Ba = k_Ba;
            prevk_BA = k_BA;
            prevk_Bh = k_Bh;
            prevk_BH = k_BH;
            prevk_z_TILTED = k_z_TILTED;
            prevk_phi_TILTED = k_phi_TILTED;
            prevk_perpC = k_perpC;
            prevk_phiC = k_phiC;
            prevk_zC = k_zC;
            prevk_phi_init = k_phi_init;
          }

          if (iBrCry % 4 == 2 && (iBrCry != 2 && iBrCry != 46)) { // barrel fin between every  4 crystals (iBrCry%4==2 means after the fin)

            double smalldev = 0.01;
            if (iBrCry < 29) { // choose the lower one
              brfink_Ba = k_Ba * (1 - smalldev);
              brfink_Bb = k_Ba * (1 - smalldev);
              brfink_BA = k_BA * (1 - smalldev); // / sin((prevk_z_TILTED + k_z_TILTED) / 2);
            } else {
              brfink_Ba = prevk_Ba * (1 - smalldev);
              brfink_Bb = prevk_Ba * (1 - smalldev);
              brfink_BA = prevk_BA * (1 - smalldev); // / sin((prevk_z_TILTED + k_z_TILTED) / 2);
            }


            brfink_Bh = brthetafinthickness;
            brfink_z_TILTED = ((prevk_z_TILTED + atan((prevk_BH - prevk_Bh) / k_BLL / 2)) + (k_z_TILTED - atan((k_BH - k_Bh) / k_BLL / 2))) / 2; // average angle of 2 crystal sides
            //cout << (prevk_z_TILTED + atan((prevk_BH-prevk_Bh)/k_BLL/2)) << " " << (k_z_TILTED - atan((k_BH-k_Bh)/k_BLL/2)) << endl;
            brfink_BLL = k_BLL; // (30 cm)
            brfink_phi_TILTED = (prevk_phi_TILTED + k_phi_TILTED) / 2; // average
            brfink_perpC = (prevk_perpC + k_perpC + (brfink_BLL - k_BLL - smalldev * fabs(prevk_zC + k_zC)) * sin(brfink_z_TILTED)) / 2; // average and move more for fin longer than crystal
            brfink_phiC = (prevk_phiC + k_phiC) / 2; // average
            brfink_zC = (prevk_zC + k_zC  - (prevk_BH - prevk_Bh) / 2 / sin(prevk_z_TILTED) + (k_BH - k_Bh) / 2 / sin(k_z_TILTED) + (brfink_BLL - k_BLL - smalldev * fabs(prevk_zC + k_zC)) * cos(brfink_z_TILTED)) / 2; // average and move more for fin longer than crystal
            brfink_phi_init = (prevk_phi_init + k_phi_init) / 2; // average


            double brfincDx1 = brfink_Ba / 2;
            double brfincDx2 = brfink_Bb / 2;
            double brfincDy = brfink_Bh / 2;
            double brfincDx3 = brfink_BA / 2;
            double brfincDz = brfink_BLL / 2;
            double brfincDx4 = brfincDx3 + (brfincDx2 - brfincDx1); // coplanarity


            if (iBrCry >= 30) {zsign = -1;}
            G4Transform3D r00 = G4RotateZ3D(90.*zsign * deg);
            G4Transform3D phi_init = G4RotateZ3D(brfink_phi_init);
            G4Transform3D tilt_z = G4RotateY3D(brfink_z_TILTED);
            G4Transform3D tilt_phi = G4RotateZ3D(brfink_phi_TILTED);
            G4Transform3D position = G4Translate3D(brfink_perpC, 0, brfink_zC);  // Move over to the left...
            G4Transform3D pos_phi = G4RotateZ3D(brfink_phiC);
            G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * r00 * phi_init;

            G4Trap* BrFinShape = new G4Trap((format("solidEclBrFin_%1%") % iBrCry).str().c_str(),
                                            brfincDz , 0 , 0, brfincDy, brfincDx2, brfincDx1, 0, brfincDy , brfincDx4, brfincDx3, 0);
            G4LogicalVolume* BrFin = new G4LogicalVolume(BrFinShape, medAl, (format("logicalEclBrFin_%1%") % iBrCry).str().c_str(), 0, 0, 0);
            //BrFin->SetSensitiveDetector(m_sensitive);
            assemblyBrFins->AddPlacedVolume(BrFin, Tr);

          } // end if barrel fin between every  4 crystals
          /////////////////////////////////////////////////////////////////////////////////////


          G4Transform3D DiodePosition = G4Translate3D(0, 0, (k_BLL + DiodeHeight) / 2 + 0.1);  // Move over to the left...
          G4Transform3D TrD = pos_phi * position * tilt_phi * tilt_z * r00 * DiodePosition ;

          if (isBeamBkgStudy) {
            for (int iSector = 0; iSector < 72; ++iSector) {//total 72 for Bareel Diode
              G4Transform3D SectorRot = G4RotateZ3D(360.*iSector / 72 * deg);
              G4Transform3D SectorRRot = G4RotateZ3D((360.*iSector / 72 - 2.488555) * deg); // replace the 2.494688
              G4Transform3D BrR = SectorRot * TrD;
              G4Transform3D BrRR = SectorRRot * TrD;
              int DiodeId = (iBrCry - 1) * 144 + iSector * 2 + 1152;
              G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclBrDiode_%1%") % DiodeId).str().c_str(), 0, 0, 0);
              Sensor->SetSensitiveDetector(new BkgSensitiveDetector("ECL", DiodeId));
              assemblyBrDiodes->AddPlacedVolume(Sensor, BrR);


              int DiodeId1 = (iBrCry - 1) * 144 + iSector * 2 - 1 + 1152;
              if (iSector == 0)DiodeId1 = (iBrCry) * 144 + iSector * 2 - 1 + 1152;
              G4LogicalVolume* Sensor1 = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclBrDiode_%1%") % DiodeId1).str().c_str(), 0, 0, 0);
              Sensor1->SetSensitiveDetector(new BkgSensitiveDetector("ECL", DiodeId1));
              assemblyBrDiodes->AddPlacedVolume(Sensor1, BrRR);
//         cout<<(format("logicalEclBrDiode_%1%") % DiodeId1).str().c_str()<<endl;
//         cout<<(format("logicalEclBrDiode_%1%") % DiodeId).str().c_str()<<endl;
            }//iSector
          }


        }
      }//iBrCry


      // add barrel phi fin ///////////////////////////////////////////////////////////////////////////////


      double brforwangle = 33.003 * deg; // same as Belle I
      double brbackangle = 52.879 * deg;

      double brphifina = 298.14 * cm / 2; // shorter base
      double brphifinL = 19.49 * cm / 2; // height

      double f_xForw = brphifinL / tan(brforwangle);
      double f_xBack = brphifinL / tan(brbackangle);
      double f_dx = (f_xForw - f_xBack) / 2;

      double brphifinA = brphifina + f_xBack + f_xForw ;
      double brphifinh = brphifinthickness / 2; // thickness
      double brphifintheta = atan(f_dx / brphifinL);

      double brphifin_shorterbaseleft = 100.08 * cm;
      double cposix = 140.594699 * cm;
      double cposiy = 6.478866 * cm;
      double brphitiltphi = 3.7579644 * deg;
      double brphirmoveout = - 3.50 * cm;
      double brphizmove =  - 0.0 * cm;
      double brphimoveperp =   0.0 * cm;

      G4Transform3D r00 = G4RotateY3D(-90. * deg);
      G4Transform3D tilt_z = G4RotateY3D(0.);
      G4Transform3D central = G4Translate3D(0, 0, (brphifina - brphifin_shorterbaseleft + f_dx));
      G4Transform3D tilt_psi = G4RotateX3D(0E-6 * deg);
      G4Transform3D tilt_phi = G4RotateZ3D(brphitiltphi);
      G4Transform3D cposition = G4Translate3D(cposix + brphirmoveout * cos(brphitiltphi) - brphimoveperp * sin(brphitiltphi), cposiy + brphirmoveout * sin(brphitiltphi) + brphimoveperp * cos(brphitiltphi), brphizmove);
      //G4Transform3D pos_phi = G4RotateZ3D(brphifin_centercrysphiC + (2.5114444*deg/2) );
      //G4Transform3D Tr =  cposition * tilt_phi * tilt_psi * central * tilt_z * r00;
      G4Transform3D Tr =  cposition * tilt_phi * tilt_psi * central * tilt_z * r00;

      G4Trap* BrPhiFinShape1 = new G4Trap(format("solidEclBrPhiFinShape1").str().c_str(),
                                          brphifinL, brphifintheta, 180. * deg,
                                          brphifinh, brphifinA, brphifinA, 0.,
                                          brphifinh, brphifina, brphifina, 0.);

      double brphifinL2 = 9.46 * cm / 2; // height
      double f_xForw2 = 0.;
      double f_xBack2 = brphifinL2 / tan(brbackangle);
      double f_dx2 = (f_xForw2 - f_xBack2) / 2;
      double brphifinA2 = brphifinA + f_xForw2 + f_xBack2;
      double brphifintheta2 = atan(f_dx2 / brphifinL2);

      G4Trap* BrPhiFinShape2 = new G4Trap(format("solidEclBrPhiFinShape2").str().c_str(),
                                          brphifinL2, brphifintheta2, 180. * deg,
                                          brphifinh, brphifinA2, brphifinA2, 0.,
                                          brphifinh, brphifinA, brphifinA, 0.);


      double brphifinL3 = 2.94 * cm / 2; // height

      G4Trap* BrPhiFinShape3 = new G4Trap(format("solidEclBrPhiFinShape3").str().c_str(),
                                          brphifinL3, 0, 180. * deg,
                                          brphifinh, brphifinA2, brphifinA2, 0.,
                                          brphifinh, brphifinA2, brphifinA2, 0.);

      G4Transform3D shape2position = G4Translate3D((brphifinA + f_dx2) - (brphifina + f_dx) - f_xBack * 2 , 0 , -(brphifinL + brphifinL2));   // move shape 2
      G4Transform3D shape3position = G4Translate3D(brphifinA2 - (brphifina + f_dx) - f_xBack * 2 - f_xBack2 * 2 , 0 , -(brphifinL + brphifinL2 * 2 + brphifinL3)); // move shape 3

      G4UnionSolid* BrPhiFinShape12 = new G4UnionSolid(format("solidEclBrPhiFinShape12").str().c_str(), BrPhiFinShape1, BrPhiFinShape2, shape2position);
      G4UnionSolid* BrPhiFinShape = new G4UnionSolid(format("solidEclBrPhiFin").str().c_str(), BrPhiFinShape12, BrPhiFinShape3, shape3position);



      G4LogicalVolume* BrPhiFin = new G4LogicalVolume(BrPhiFinShape, medAl, format("logicalEclBrPhiFin").str().c_str(), 0, 0, 0);
      assemblyBrPhiFins->AddPlacedVolume(BrPhiFin, Tr);



      /////////////////////////////////////////////////////////////////////////////////////////////////////


      double h1, h2, bl1, bl2, tl1, tl2, alpha1, alpha2, Rphi1, Rphi2, Rtheta, Pr, Ptheta, Pphi, halflength;
      int k_forwMPerRing[] = { 3, 3, 4, 4, 4, 6, 6, 6, 6, 6, 6, 9, 9 };
      int k_backMPerRing[] = { 9, 9, 6, 6, 6, 6, 6, 4, 4, 4 } ;

      int iRing = 0;
      int nRing = 0;
      int iPhi = 0;

      G4AssemblyVolume* assemblyFwCrystals = new G4AssemblyVolume();
      G4AssemblyVolume* assemblyFwDiodes = new G4AssemblyVolume();
      // add assembly forward foil ///////////////////////////////
      G4AssemblyVolume* assemblyFwFoils = new G4AssemblyVolume();
      ///////////////////////////////////////////////////////////

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
        Rtheta = counter.getAngle("K_Rtheta") ;
        Rphi2 = counter.getAngle("K_Rphi2")  ;
        Pr = counter.getLength("K_Pr") * cm;
        Ptheta = counter.getAngle("K_Ptheta") ;
        Pphi = counter.getAngle("K_Pphi") ;
        halflength = 15.0 * cm;

        double fwfoilthickness;
        if (iCry == 5) {
          fwfoilthickness = thinpentafoilthickness;
        } else if (1 <= iCry && iCry <= 6) {
          fwfoilthickness = thinfoilthickness;
        } else {
          fwfoilthickness = foilthickness;
        }

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


        if (iCry == 5) { // Pentagon!!

          double smalls = 1E-6 * cm; // small size (near zero) of the tip
          double trih1 = 0.166375 * cm; // from Belle I code
          double tritl1 = smalls;
          double trih2 = (bl2 - smalls) * trih1 / (bl1 - tritl1);
          double tribl1 = bl1;
          double tribl2 = bl2;
          double tritl2 = tribl2 - (tribl1 - tritl1) * trih2 / trih1;
          double tritheta =  - (atan((h2 - h1) / halflength / 2) + atan((trih2 - trih1) / halflength / 2)); //z-incline of tri

          G4Transform3D Tr = position * m3 * m2 * m1;

          G4Trap* FwCrysralShapeTrap = new G4Trap((format("solidEclFwCrystalTrap_%1%") % iCry).str().c_str(), halflength , 0 , 0, h1 , bl1, tl1 , alpha1 , h2 , bl2, tl2, alpha2);
          G4Trap* FwCrysralShapeTrigon = new G4Trap((format("solidEclFwCrystalTrigon_%1%") % iCry).str().c_str(), halflength , tritheta, 90.*deg , trih1 , tritl1, tribl1 , alpha1 , trih2 , tritl2, tribl2, alpha2);
          G4Transform3D trigonposition = G4Translate3D(0 , -(h1 + h2 + trih1 + trih2 - smalls) / 2 , 0); // move trigon
          G4UnionSolid* FwCrystalShape = new G4UnionSolid((format("solidEclFwCrystal_%1%") % iCry).str().c_str(), FwCrysralShapeTrap, FwCrysralShapeTrigon, trigonposition);
          G4LogicalVolume* FwCrystal = new G4LogicalVolume(FwCrystalShape, medCsI, (format("logicalEclFwCrystal_%1%") % iCry).str().c_str(), 0, 0, 0);
          FwCrystal->SetSensitiveDetector(m_sensitive);

          assemblyFwCrystals->AddPlacedVolume(FwCrystal, Tr);

          /////////////////  add forward foil -- pentagon //////////////////////

          double triangle1 = atan(2 * trih1 / (tribl1 - tritl1)); // the smaller angle of the trigon
          double triangle2 = atan(2 * trih2 / (tribl2 - tritl2));
          double trifoilh1 = trih1 + fwfoilthickness / cos(triangle1);
          double trifoiltl1 = smalls;
          double trifoilbl1 = tribl1 * (trifoilh1 / trih1);
          double trifoilh2 = trih2 + fwfoilthickness / cos(triangle2);
          double trifoilbl2 = tribl2 * (trifoilh2 / trih2);
          double trifoiltl2 = trifoilbl2 - (trifoilbl1 - trifoiltl1) * trifoilh2 / trifoilh1;
          double trifoilq = fwfoilthickness * ((1 / sin(triangle1) - 1 / sin(fwtrapangle1)) / (1 / tan(triangle1) + 1 / tan(fwtrapangle1)) + (1 / sin(triangle2) - 1 / sin(fwtrapangle2)) / (1 / tan(triangle2) + 1 / tan(fwtrapangle2))) / 2; // The distance between unfoiled trap bl to segment of foiled pentagon (position of cut plane)
          double cutboxangle = atan((h2 - h1) / halflength / 2); //

          G4Trap* FwFoilouttrap = new G4Trap((format("FwFoilouttrap_%1%") % iCry).str().c_str(), fwfoilhalflength , 0 , 0, fwfoilh1,  fwfoilbl1, fwfoiltl1, alpha1 , fwfoilh2, fwfoilbl2, fwfoiltl2, alpha2);
          G4Box* CutBox = new G4Box("CutBox", 15.*cm, 2.*cm, 50.*cm); // to cut off foil in the attaching side
          G4Transform3D cutboardtr1 = G4Translate3D(0, -(2.*cm / cos(cutboxangle)) - (h1 + h2) / 2 + trifoilq, 0) * G4RotateY3D(-cutboxangle);
          G4SubtractionSolid* FwFoilouttrapcut = new G4SubtractionSolid((format(" FwFoilouttrapcut_%1%") % iCry).str().c_str(), FwFoilouttrap, CutBox, cutboardtr1);
          G4Trap* FwFoilouttrigon = new G4Trap((format("FwFoilouttrigon_%1%") % iCry).str().c_str(), halflength , tritheta, 90.*deg , trifoilh1 , trifoiltl1, trifoilbl1 , alpha1 , trifoilh2 , trifoiltl2, trifoilbl2, alpha2);

          G4Transform3D cutboardtr2 = G4Translate3D(0, (2.*cm / cos(cutboxangle)) + (trih1 + trih2) / 2 - trifoilq, 0) * G4RotateY3D(-cutboxangle);
          G4SubtractionSolid* FwFoilouttrigoncut = new G4SubtractionSolid((format(" FwFoilouttrigoncut_%1%") % iCry).str().c_str(), FwFoilouttrigon, CutBox, cutboardtr2);
          G4UnionSolid* FwFoiloutpenta = new G4UnionSolid((format("FwFoiloutpenta_%1%") % iCry).str().c_str(), FwFoilouttrapcut, FwFoilouttrigoncut, trigonposition);

          G4Trap* FwFoilintrap = new G4Trap((format("FwFoilintrap_%1%") % iCry).str().c_str(), halflength * avoidov , 0 , 0, h1 * avoidov , bl1 * avoidov, tl1 * avoidov , alpha1 , h2 * avoidov , bl2 * avoidov, tl2 * avoidov, alpha2);
          G4Trap* FwFoilintrigon = new G4Trap((format("FwFoilintrigon_%1%") % iCry).str().c_str(), halflength * avoidov , tritheta, 90.*deg , trih1 * avoidov , tritl1 * avoidov, tribl1 * avoidov , alpha1 , trih2 * avoidov , tritl2 * avoidov, tribl2 * avoidov, alpha2);
          G4UnionSolid* FwFoilinpenta = new G4UnionSolid((format("FwFoilinpenta_%1%") % iCry).str().c_str(), FwFoilintrap, FwFoilintrigon, trigonposition);

          G4SubtractionSolid* FwFoilShape = new G4SubtractionSolid((format("FwFoil_%1%") % iCry).str().c_str(), FwFoiloutpenta, FwFoilinpenta);

          G4LogicalVolume* FwFoil = new G4LogicalVolume(FwFoilShape, medAlTeflon_thinpenta, (format("logicalFwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
          assemblyFwFoils->AddPlacedVolume(FwFoil, Tr);

        } // End of Pentagon
        else { // Trapezoids

          G4Transform3D Tr = position * m3 * m2 * m1;
          G4Trap* FwCrysralShape = new G4Trap((format("solidEclFwCrystal_%1%") % iCry).str().c_str(),
                                              halflength , 0 , 0, h1 ,   bl1, tl1 , alpha1 , h2   , bl2, tl2, alpha2);
          G4LogicalVolume* FwCrysral = new G4LogicalVolume(FwCrysralShape, medCsI, (format("logicalEclFwCrystal_%1%") % iCry).str().c_str(), 0, 0, 0);
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

          if (1 <= iCry && iCry <= 6) { // but not pentagon
            G4LogicalVolume* FwFoil = new G4LogicalVolume(FwFoilShape, medAlTeflon_thin, (format("logicalFwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
            assemblyFwFoils->AddPlacedVolume(FwFoil, Tr);
          } else {
            G4LogicalVolume* FwFoil = new G4LogicalVolume(FwFoilShape, medAlTeflon, (format("logicalFwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
            assemblyFwFoils->AddPlacedVolume(FwFoil, Tr);
          }

        } // End of Trapezoids
        //////////////////////////////////////////////////////////////////////////////////


        G4Transform3D DiodePosition = G4Translate3D(0, 0, halflength + (DiodeHeight) / 2 + 0.1); // Move over to the left...
        G4Transform3D TrD =  position * m3 * m2 * m1 * DiodePosition ;


        if (isBeamBkgStudy) {
          for (int iSector = 0; iSector < 16; ++iSector) {//total 16 for EndcapDiode
            G4Transform3D SectorRot = G4RotateZ3D(360.*iSector / 16 * deg);
            G4Transform3D FwR = SectorRot * TrD;
            iPhi = iCry - nRing - 1;
            int DiodeId = nRing * 16 + iSector * k_forwMPerRing[iRing] + iPhi ;
            G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclFwDiode_%1%") % DiodeId).str().c_str(), 0, 0, 0);
            Sensor->SetSensitiveDetector(new BkgSensitiveDetector("ECL", DiodeId));
            assemblyFwDiodes->AddPlacedVolume(Sensor, FwR);
          }
        }
        if (iPhi == (k_forwMPerRing[iRing] - 1)) {nRing = nRing + k_forwMPerRing[iRing]; iRing++;}

      }//forward endcap crystals

      iRing = 0;
      nRing = 0;
      iPhi = 0;
      G4AssemblyVolume* assemblyBwCrystals = new G4AssemblyVolume();
      G4AssemblyVolume* assemblyBwDiodes = new G4AssemblyVolume();
      // add assembly backward foil ///////////////////////////////
      G4AssemblyVolume* assemblyBwFoils = new G4AssemblyVolume();
      ///////////////////////////////////////////////////////////

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
        Rtheta = counter.getAngle("K_Rtheta") ;
        Rphi2 = counter.getAngle("K_Rphi2")  ;
        Pr = counter.getLength("K_Pr") * cm;
        Ptheta = counter.getAngle("K_Ptheta") ;
        Pphi = counter.getAngle("K_Pphi") ;
        halflength = 15.0 * cm;

        double bwfoilthickness;
        if ((iCry == 110 || iCry == 113 || iCry == 116 || iCry == 119) || (121 <= iCry && iCry <= 128)) {
          bwfoilthickness = thinfoilthickness;
        } else {
          bwfoilthickness = foilthickness;
        }

        // add backward foil dimensions ////////////////////////////
        double bwtrapangle1 = atan(2 * h1 / (bl1 - tl1)); // the smaller angle of the trap
        double bwtrapangle2 = atan(2 * h2 / (bl2 - tl2));
        double bwfoilh1 = h1 + bwfoilthickness;
        double bwfoilh2 = h2 + bwfoilthickness;
        double bwfoiltl1 = tl1 + bwfoilthickness * tan(bwtrapangle1 / 2);
        double bwfoilbl1 = bl1 + bwfoilthickness / tan(bwtrapangle1 / 2);
        double bwfoiltl2 = tl2 + bwfoilthickness * tan(bwtrapangle2 / 2);
        double bwfoilbl2 = bwfoiltl2 + (bwfoilbl1 - bwfoiltl1) * bwfoilh2 / bwfoilh1;
        double bwfoilhalflength = halflength + bwfoilthickness;
        ///////////////////////////////////////////////////////////

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
        BwCrysral->SetSensitiveDetector(m_sensitive);
        assemblyBwCrystals->AddPlacedVolume(BwCrysral, Tr);

        /////////////////  add backward foil ///////////////////////////////////////

        G4Trap* BwFoilout = new G4Trap((format("BwFoilout_%1%") % iCry).str().c_str(),
                                       bwfoilhalflength , 0 , 0, bwfoilh1,  bwfoilbl1,
                                       bwfoiltl1, alpha1 , bwfoilh2, bwfoilbl2,
                                       bwfoiltl2, alpha2);

        G4Trap* BwFoilin = new G4Trap((format("solidEclBwCrystal_%1%") % iCry).str().c_str(),
                                      halflength * avoidov , 0 , 0, h1 * avoidov , bl1 * avoidov, tl1 * avoidov , alpha1 , h2 * avoidov   , bl2 * avoidov, tl2 * avoidov, alpha2);
        G4SubtractionSolid* BwFoilShape = new G4SubtractionSolid((format("BwFoil_%1%") % iCry).str().c_str(), BwFoilout, BwFoilin);

        if ((iCry == 110 || iCry == 113 || iCry == 116 || iCry == 119) || (121 <= iCry && iCry <= 128)) {
          G4LogicalVolume* BwFoil = new G4LogicalVolume(BwFoilShape, medAlTeflon_thin, (format("logicalBwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
          assemblyBwFoils->AddPlacedVolume(BwFoil, Tr);
        } else {
          G4LogicalVolume* BwFoil = new G4LogicalVolume(BwFoilShape, medAlTeflon, (format("logicalBwFoil_%1%") % iCry).str().c_str(), 0, 0, 0);
          assemblyBwFoils->AddPlacedVolume(BwFoil, Tr);
        }

        //////////////////////////////////////////////////////////////////////////////////


        G4Transform3D DiodePosition = G4Translate3D(0, 0, halflength + (DiodeHeight) / 2 + 0.1); // Move over to the left...
        G4Transform3D TrD =  position * m3 * m2 * m1 * DiodePosition ;

        if (isBeamBkgStudy) {
          for (int iSector = 0; iSector < 16; ++iSector) {//total 16 for EndcapDiode
            G4Transform3D SectorRot = G4RotateZ3D(360.*iSector / 16 * deg);
            G4Transform3D BwR = SectorRot * TrD;
            iPhi = (iCry - 72) - nRing - 1;
            int DiodeId = nRing * 16 + iSector * k_backMPerRing[iRing] + iPhi + 7776 ;
            G4LogicalVolume* Sensor = new G4LogicalVolume(SensorDiode, medSi, (format("logicalEclBwDiode_%1%") % DiodeId).str().c_str(), 0, 0, 0);
            Sensor->SetSensitiveDetector(new BkgSensitiveDetector("ECL", DiodeId));
            assemblyBwDiodes->AddPlacedVolume(Sensor, BwR);
//         cout<<(format("logicalEclBwDiode_%1%") % DiodeId).str().c_str()<<endl;
          }
          if (iPhi == (k_backMPerRing[iRing] - 1)) {nRing = nRing + k_backMPerRing[iRing]; iRing++;}
        }
      }//backward endcap crystals




      for (int iSector = 0; iSector < 16; ++iSector) {//total 16
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / 16 * deg);
        assemblyFwCrystals->MakeImprint(logical_ecl, BrR);
        assemblyFwFoils->MakeImprint(logical_ecl, BrR);  // foil
      }//16 sectior
      for (int iSector = 0; iSector < 72; ++iSector) {//total 72
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / 72 * deg);
        G4Transform3D BrRR = G4RotateZ3D((360.*iSector / 72 - 2.488555) * deg); // replace 2.494688
        assemblyBrCrystals->MakeImprint(logical_ecl, BrRR);
        assemblyBrCrystals->MakeImprint(logical_ecl, BrR);
        assemblyBrFoils->MakeImprint(logical_ecl, BrRR);  // foil
        assemblyBrFoils->MakeImprint(logical_ecl, BrR);  // foil
        assemblyBrFins->MakeImprint(logical_ecl, BrRR);  // theta fin
        assemblyBrFins->MakeImprint(logical_ecl, BrR);  // theta fin
        assemblyBrPhiFins->MakeImprint(logical_ecl, BrR);  // phi fin
      }//iSector
      for (int iSector = 0; iSector < 16; ++iSector) {//total 16
        G4Transform3D BrR = G4RotateZ3D(360.*iSector / 16 * deg);
        assemblyBwCrystals->MakeImprint(logical_ecl, BrR);
        assemblyBwFoils->MakeImprint(logical_ecl, BrR);  // foil
      }//16 sectior


      if (isBeamBkgStudy) {
        assemblyFwDiodes->MakeImprint(logical_ecl, Global_offset);
        assemblyBrDiodes->MakeImprint(logical_ecl, Global_offset);
        assemblyBwDiodes->MakeImprint(logical_ecl, Global_offset);
      }

      double BarrelCylinderWZ[6]    = {k_c2z1, k_c2z2, k_c2z2, k_c1z2, k_c1z2, k_c1z3};
      double BarrelCylinderWRin[6]  = {k_c2r1, k_c2r1, k_c2r1, k_c1r1, k_c1r1, k_c1r1};
      double BarrelCylinderWRout[6] = {k_c2r3, k_c2r3, k_c2r2, k_c1r2, k_c1r3, k_c1r3};


      G4Polycone* BarrelCylinderWorld = new G4Polycone("BarrelCylinderWorld", 0, 2 * PI, 6, BarrelCylinderWZ, BarrelCylinderWRin, BarrelCylinderWRout);
      G4LogicalVolume* logical_BarrelCylinder = new G4LogicalVolume(BarrelCylinderWorld, medAir, "logical_BarrelCylinderWorld");
      physical_ECLBarrelCylinder = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logical_BarrelCylinder, "physicalBarrelCylinder", &topVolume, false, 0);
      G4AssemblyVolume* assemblyBarrelCylinderSupport = new G4AssemblyVolume();

      double BarrelCylinder1Z[4] = {k_c1z1, k_c1z2, k_c1z2, k_c1z3};
      double BarrelCylinder1Rin[4] = {k_c1r1, k_c1r1, k_c1r1, k_c1r1};
      double BarrelCylinder1Rout[4] = {k_c1r2, k_c1r2, k_c1r3, k_c1r3};

      G4Polycone* barCy1 =
        new G4Polycone("ECL Barrel Support Cylinder", 0 * deg, 360.*deg, 4, BarrelCylinder1Z, BarrelCylinder1Rin, BarrelCylinder1Rout);


      double BarrelCylinder2Z[4] = {k_c2z1, k_c2z2, k_c2z2, k_c2z3};
      double BarrelCylinder2Rin[4] = {k_c2r1, k_c2r1, k_c2r1, k_c2r1};
      double BarrelCylinder2Rout[4] = {k_c2r3, k_c2r3, k_c2r2, k_c1r2};

      G4Polycone* barCy2 =
        new G4Polycone("ECL Barrel Support Cylinder", 0 * deg, 360.*deg, 4, BarrelCylinder2Z, BarrelCylinder2Rin, BarrelCylinder2Rout);

      G4LogicalVolume* barCy1_logi = new G4LogicalVolume(barCy1, Materials::get("G4_Fe"), "ECLBarrelSupportCylinder1", 0, 0, 0);
      G4LogicalVolume* barCy2_logi = new G4LogicalVolume(barCy2, Materials::get("G4_Fe"), "ECLBarrelSupportCylinder2", 0, 0, 0);


      assemblyBarrelCylinderSupport->AddPlacedVolume(barCy1_logi, Global_offset);
      assemblyBarrelCylinderSupport->AddPlacedVolume(barCy2_logi, Global_offset);
      assemblyBarrelCylinderSupport->MakeImprint(logical_BarrelCylinder, Global_offset);


      makeEndcap(0);
      makeEndcap(1);
      makeSupport();


    }//create

    void GeoECLCreator::makeSupport()
    {
///////////////////////////
//////makeBarrelSuppor/////
///////////////////////////
      G4AssemblyVolume* assemblyInnerBarrelSupport = new G4AssemblyVolume();
      const EclRad InnerBarreloffset = (2.796 - 2.5) * deg;
      const int InnerBarrelnSectors = 144 / 2;
      const EclRad InnerBarreldPhi = 2 * PI / InnerBarrelnSectors;

      const EclCM InnerBarrelw1 = 5.05 / 2 * cm;
//   const EclCM h1 = 2.5;
      const EclCM InnerBarrelh1 = (2.5 + 0.3) * cm; // additional pusher
      const EclCM InnerBarrelz11 = k_barCryBackZOut;
      const EclCM InnerBarrelz12 = k_barCryForwZOut;
//   const EclCM InnerBarrelt1 = InnerBarrelz12 - InnerBarrelz11;
      const EclCM InnerBarrelro1 = k_barCryOuterR;
      const EclCM InnerBarrelri1 = InnerBarrelro1 - InnerBarrelh1;
      const EclRad InnerBarreldp1 = InnerBarrelw1 / ((InnerBarrelro1 + InnerBarrelri1) / 2.0);
//   const TripletF mPar1[]={ // PolyCONe parameters
//      TripletF(z11, ri1, ro1),
//      TripletF(z12, ri1, ro1)};
//   const vTripletF vPar1(mPar1, mPar1+2) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (ecs1Name, "ECL Inner Barrel 1st piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp1/k_degRad, vPar1 ) );
//   const HepTransform3D rot11 (HepTransform3D::Identity);
//   const HepRotate3D rot12 (HepRotateZ3D(-dp1 + dPhi));
//   barVS->addChild( G3WCopyPCON(*ecs1VS, 1, rot11) );
//   barVS->addChild( G3WCopyPCON(*ecs1VS, 2, rot12) );

      double InnerBarrel1Z[2] = {InnerBarrelz11, InnerBarrelz12};
      double InnerBarrel1Rin[2] = {InnerBarrelri1, InnerBarrelri1};
      double InnerBarrel1Rout[2] = {InnerBarrelro1, InnerBarrelro1};


      G4Polycone* InnerBarrel1 =
        new G4Polycone("ECL Inner Barrel 1st piece", 0, InnerBarreldp1 , 2, InnerBarrel1Z, InnerBarrel1Rin, InnerBarrel1Rout);
      G4LogicalVolume* InnerBarrel1_logi = new G4LogicalVolume(InnerBarrel1, Materials::get("G4_Fe"), "ECLInnerBarrel1st", 0, 0, 0);


      const EclCM InnerBarrelh2 = 1.6 * cm;
      const EclCM InnerBarrelz21 = k_barSupBackFarZ + 13.5 * cm;
      const EclCM InnerBarrelz22 = k_barSupForwFarZ - 12.0 * cm;
//   const EclCM InnerBarrelt2 = InnerBarrelz12 - InnerBarrelz11;
      const EclCM InnerBarrelro2 = k_barCryOuterR - 0.21 * cm;
      const EclCM InnerBarrelri2 = InnerBarrelro2 - InnerBarrelh2;
      const EclRad InnerBarreldp2 = InnerBarreldPhi - InnerBarreldp1 * 2;
//   const TripletF mPar2[]={ // PolyCONe parameters
//      TripletF(z21, ri2, ro2),
//      TripletF(z22, ri2, ro2)};
//   const vTripletF vPar2(mPar2, mPar2+2) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (ecs2Name, "ECL Inner Barrel 2nd piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp2/k_degRad, vPar2 ) );
//   G3WVolumeSpec* ecs2VS ( G3WVolumeSpec::findVol(ecs2Name, &aVolSpecs ));
//   const HepRotate3D rot2 (HepRotateZ3D(-dp2/2.0 + dPhi/2));
//   const HepRotate3D rot2 (HepRotateZ3D(-dp2/2.0 + dPhi/2));
//   barVS->addChild( G3WCopyPCON(*ecs2VS, 1, rot2) );

      double InnerBarrel2Z[2] = {InnerBarrelz21, InnerBarrelz22};
      double InnerBarrel2Rin[2] = {InnerBarrelri2, InnerBarrelri2};
      double InnerBarrel2Rout[2] = {InnerBarrelro2, InnerBarrelro2};



      G4Polycone* InnerBarrel2 =
        new G4Polycone("ECL Inner Barrel 2nd piece", 0, InnerBarreldp2 , 2, InnerBarrel2Z, InnerBarrel2Rin, InnerBarrel2Rout);
      G4LogicalVolume* InnerBarrel2_logi = new G4LogicalVolume(InnerBarrel2, Materials::get("G4_Fe"), "ECLInnerBarrel2nd", 0, 0, 0);

      const EclCM InnerBarrelgap4 = (0.1) * cm;
      const EclCM InnerBarrelw4 = 0.3 * cm;
      const EclCM InnerBarrelh4 = (InnerBarrelri1 - k_barCryOuterQ) - k_barCryMargin - InnerBarrelgap4;
      const EclCM InnerBarrelz41 = k_barSupBackFarZ + 10.5 * cm + 1.8 * cm;
      const EclCM InnerBarrelz42 = k_barSupForwFarZ - 9.0 * cm - 1.0 * cm;
//   const EclCM InnerBarrelt4 = InnerBarrelz42 - InnerBarrelz41;
      const EclCM InnerBarrelro4 = InnerBarrelri1;
      const EclCM InnerBarrelri4 = InnerBarrelro4 - InnerBarrelh4;
      const EclRad InnerBarreldp4 = InnerBarrelw4 / ((InnerBarrelro4 + InnerBarrelri4) / 2.0);
//   const TripletF mPar4[]={ // PolyCONe parameters
//      TripletF(z41, ri4, ro4),
//      TripletF(z42, ri4, ro4)};
//   const vTripletF vPar4(mPar4, mPar4+2) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (ecs4Name, "ECL Inner Barrel 4th piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp4/k_degRad, vPar4 ) );
//   G3WVolumeSpec* ecs4VS ( G3WVolumeSpec::findVol(ecs4Name, &aVolSpecs ));
//   const HepRotate3D rot41 (HepRotateZ3D(-dp4 +dp1));
//   const HepRotate3D rot42 (HepRotateZ3D(-dp1 + dPhi));
//   barVS->addChild( G3WCopyPCON(*ecs4VS, 1, rot41) );
//   barVS->addChild( G3WCopyPCON(*ecs4VS, 2, rot42) );

      double InnerBarrel4Z[2] = {InnerBarrelz41, InnerBarrelz42};
      double InnerBarrel4Rin[2] = {InnerBarrelri4, InnerBarrelri4};
      double InnerBarrel4Rout[2] = {InnerBarrelro4, InnerBarrelro4};

      G4Polycone* InnerBarrel4 =
        new G4Polycone("ECL Inner Barrel 4th piece", 0, InnerBarreldp4 , 2, InnerBarrel4Z, InnerBarrel4Rin, InnerBarrel4Rout);
      G4LogicalVolume* InnerBarrel4_logi = new G4LogicalVolume(InnerBarrel4, Materials::get("G4_Fe"), "ECLInnerBarrel4th", 0, 0, 0);


      const EclCM InnerBarrelt5 = 0.05 * cm;
      const EclCM InnerBarrell5 = (5.0 + 329.0) * cm;
      const EclCM InnerBarrelz51 = InnerBarrelz11;
      const EclCM InnerBarrelz52 = InnerBarrelz51 + InnerBarrell5;
      const EclCM InnerBarrelr52 = 1.2 / 2 * cm;
      const EclCM InnerBarrelr51 = InnerBarrelr52 - InnerBarrelt5;
      EclCM InnerBarrelr5 = InnerBarrelro1 - 1.25 * cm - 1.58 * cm;
      const EclRad InnerBarreldp5 = atan2(InnerBarrelr52, InnerBarrelr5);
      InnerBarrelr5 = sqrt(InnerBarrelr5 * InnerBarrelr5 + InnerBarrelr52 * InnerBarrelr52);
//   const TripletF mPar5[]={ // PolyCONe parameters
//      TripletF(z51, r51, r52),
//      TripletF(z52, r51, r52)};
//   const vTripletF vPar5(mPar5, mPar5+2) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (ecs5Name, "ECL Inner Barrel 5th piece",
//                         G3WMaterial::getCopper() ,
//                         0.0, 360.0, vPar5 ) );
//   G3WVolumeSpec* ecs5VS ( G3WVolumeSpec::findVol(ecs5Name, &aVolSpecs ));
//   const HepRotate3D rot51 (HepRotateZ3D(dp5 +dp1));
//   const HepRotate3D rot52 (HepRotateZ3D(-dp5-dp1 + dPhi));
//   const HepTransform3D tr51(rot51*HepTranslateX3D(r5));
//   barVS->addChild( G3WCopyPCON(*ecs5VS, 1, tr51) );
//   const HepTransform3D tr52(rot52*HepTranslateX3D(r5));
//   barVS->addChild( G3WCopyPCON(*ecs5VS, 2, tr52) );


      double InnerBarrel5Z[2] = {InnerBarrelz51, InnerBarrelz52};
      double InnerBarrel5Rin[2] = {InnerBarrelr51, InnerBarrelr51};
      double InnerBarrel5Rout[2] = {InnerBarrelr52, InnerBarrelr52};

      G4Polycone* InnerBarrel5 =
        new G4Polycone("ECL Inner Barrel 5th piece", 0 * deg, 360.*deg , 2, InnerBarrel5Z, InnerBarrel5Rin, InnerBarrel5Rout);
      G4LogicalVolume* InnerBarrel5_logi = new G4LogicalVolume(InnerBarrel5, Materials::get("G4_Fe") , "ECLInnerBarrel5th", 0, 0, 0);


      for (int i = 0; i < InnerBarrelnSectors; ++i) {
//   const HepTransform3D rot11 (HepTransform3D::Identity);
//   const HepRotate3D rot12 (HepRotateZ3D(-dp1 + dPhi));
//   const HepRotate3D rot2 (HepRotateZ3D(-dp2/2.0 + dPhi/2));
//   const HepRotate3D rot41 (HepRotateZ3D(-dp4 +dp1));
//   const HepRotate3D rot42 (HepRotateZ3D(-dp1 + dPhi));
//   const HepRotate3D rot51 (HepRotateZ3D(dp5 +dp1));
//   const HepRotate3D rot52 (HepRotateZ3D(-dp5-dp1 + dPhi));
//      const HepRotate3D rot (HepRotateZ3D(i*dPhi -dPhi/2 + offset));
        double iSectorsRot = i * InnerBarreldPhi - InnerBarreldPhi / 2 + InnerBarreloffset;
        G4Transform3D InnerBarrelrot11 = G4RotateZ3D(0 + iSectorsRot);
        G4Transform3D InnerBarrelrot12 = G4RotateZ3D(-InnerBarreldp1 + InnerBarreldPhi + iSectorsRot);
        G4Transform3D InnerBarrelrot2 = G4RotateZ3D(-InnerBarreldp2 / 2.0 + InnerBarreldPhi / 2 + iSectorsRot);

        G4Transform3D InnerBarrelrot41 = G4RotateZ3D(-InnerBarreldp4 + InnerBarreldp1  + iSectorsRot);
        G4Transform3D InnerBarrelrot42 = G4RotateZ3D(-InnerBarreldp1 + InnerBarreldPhi  + iSectorsRot);

        G4Transform3D InnerBarrelX5 = G4Translate3D(InnerBarrelr5, 0, 0);
        G4Transform3D InnerBarrelrot51 = G4RotateZ3D(InnerBarreldp5 + InnerBarreldp1  + iSectorsRot);
        G4Transform3D InnerBarrelrot52 = G4RotateZ3D(-InnerBarreldp5 - InnerBarreldp1 + InnerBarreldPhi  + iSectorsRot);
        G4Transform3D InnerBarreltr51 = InnerBarrelrot51 * InnerBarrelX5;
        G4Transform3D InnerBarreltr52 = InnerBarrelrot52 * InnerBarrelX5;

        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel1_logi, InnerBarrelrot11);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel1_logi, InnerBarrelrot12);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel2_logi, InnerBarrelrot2);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel4_logi, InnerBarrelrot42);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel4_logi, InnerBarrelrot41);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel5_logi, InnerBarreltr51);
        assemblyInnerBarrelSupport->AddPlacedVolume(InnerBarrel5_logi, InnerBarreltr52);
      }

      assemblyInnerBarrelSupport->MakeImprint(logical_ecl, Global_offset);



      G4AssemblyVolume* assemblyBarrelSupport = new G4AssemblyVolume();


// -------------------- Start Barrel Inner Support Spec ---------------------
//    TripletF( k_barConBackZOut, k_barConBackR1   , k_barCryOuterR  ) ,
//    TripletF( k_barCryBackZOut, k_barConBackR2   , k_barCryOuterR  ) ,
//    TripletF( k_barCryBackZOut, k_barConBackR2   , k_barConBackR3  ) ,
//    TripletF( k_barConBackZ3  , k_barConInnerR   , k_barConBackR4  ) ,
//    TripletF( k_barConBackZIn , k_barConInnerR   , k_barConBackR5  ) ,
//    TripletF( k_barConBackZIn , k_barConInnerR   , k_barCryInnerR  ) ,
//    TripletF( k_barConForwZIn , k_barConInnerR   , k_barCryInnerR  ) ,
//    TripletF( k_barConForwZIn , k_barConInnerR   , k_barConForwR5  ) ,
//    TripletF( k_barConForwZ3  , k_barConInnerR   , k_barConForwR4  ) ,
//    TripletF( k_barCryForwZOut, k_barConForwR2   , k_barConForwR3  ) ,
//    TripletF( k_barCryForwZOut, k_barConForwR2   , k_barCryOuterR  ) ,
//    TripletF( k_barConForwZOut, k_barConForwR1   , k_barCryOuterR  ) ;

      double BarrelInnerSupport1Z[12] = {
        k_barConBackZOut, k_barCryBackZOut, k_barCryBackZOut, k_barConBackZ3,
        k_barConBackZIn , k_barConBackZIn , k_barConForwZIn , k_barConForwZIn,
        k_barConForwZ3  , k_barCryForwZOut, k_barCryForwZOut, k_barConForwZOut
      };
      double BarrelInnerSupportRin[12] = {
        k_barConBackR1, k_barConBackR2, k_barConBackR2, k_barConInnerR,
        k_barConInnerR, k_barConInnerR, k_barConInnerR, k_barConInnerR,
        k_barConInnerR, k_barConForwR2, k_barConForwR2, k_barConForwR1
      };
      double BarrelInnerSupportRout[12] = {
        k_barCryOuterR, k_barCryOuterR, k_barConBackR3, k_barConBackR4,
        k_barConBackR5, k_barCryInnerR, k_barCryInnerR, k_barConForwR5,
        k_barConForwR4, k_barConForwR3, k_barCryOuterR, k_barCryOuterR
      };

      G4Polycone* barInnerSupport =
        new G4Polycone("ECL Barrel Inner Support", 0 * deg, 360.*deg, 12, BarrelInnerSupport1Z, BarrelInnerSupportRin, BarrelInnerSupportRout);

      // -------------------- Start Barrel Outer Skin Spec ---------------------
//  const TripletF osParA[]={  // PCON parameters
//    TripletF( k_barSupBackFarZ, k_barConBackR1   , k_barConOuterR  ) ,
//    TripletF( k_barConBackZOut, k_barConBackR1   , k_barConOuterR  ) ,
//    TripletF( k_barConBackZOut, k_barCryOuterR   , k_barConOuterR  ) ,
//    TripletF( k_barConForwZOut, k_barCryOuterR   , k_barConOuterR  ) ,
//    TripletF( k_barConForwZOut, k_barConForwR1   , k_barConOuterR  ) ,
//    TripletF( k_barSupForwFarZ, k_barSupForwFarIR, k_barConOuterR ) } ;

      // -------------------- Start Barrel Inner Support Spec ---------------------

      double BarrelOuterSkinZ[6] = {
        k_barSupBackFarZ, k_barConBackZOut, k_barConBackZOut,
        k_barConForwZOut, k_barConForwZOut, k_barSupForwFarZ
      };
      double BarrelOuterSkinRin[6] =  {k_barConBackR1, k_barConBackR1, k_barCryOuterR, k_barCryOuterR, k_barConForwR1, k_barSupForwFarIR};
      double BarrelOuterSkinRout[6] = {k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR};



      G4Polycone* barOuterSkin =
        new G4Polycone("ECL Barrel Outer Skin", 0 * deg, 360.*deg, 6, BarrelOuterSkinZ, BarrelOuterSkinRin, BarrelOuterSkinRout);


      G4LogicalVolume* barInnerSupport_logi = new G4LogicalVolume(barInnerSupport, Materials::get("G4_Al"), "ECLBarrelInnerSupport", 0, 0, 0);
      G4LogicalVolume* barOuterSkin_logi = new G4LogicalVolume(barOuterSkin, Materials::get("G4_Fe"), "ECLBarrelOuterSkin", 0, 0, 0);


      assemblyBarrelSupport->AddPlacedVolume(barInnerSupport_logi, Global_offset);
      assemblyBarrelSupport->AddPlacedVolume(barOuterSkin_logi, Global_offset);




      const double BASdelPhi = 2.*PI / 72;
      const double BASoffset = 2.796 * PI / 180.;
      const double BAScut = 0.5 * cm; // due to the PCON overlap


      double BarrelL1Z[4] = {k_l1z1, k_l1z2, k_l1z2, k_l1z3};
      double BarrelL1Rin[4] = {k_l1r1, k_l1r2, k_l1r3, k_l1r3};
      double BarrelL1Rout[4] = {k_l1r4 - BAScut, k_l1r4 - BAScut, k_l1r4 - BAScut, k_l1r4 - BAScut};






      G4Polycone* barL1 =
        new G4Polycone("ECL Barrel Support barrel-fwd L", BASdelPhi + (-k_l1ang / 2.), k_l1ang, 4, BarrelL1Z, BarrelL1Rin, BarrelL1Rout);




      double BarrelL2Z[4] = {k_l2z1, k_l2z2, k_l2z2, k_l2z3};
      double BarrelL2Rin[4] = {k_l2r2, k_l2r2, k_l2r1, k_l2r1};
      double BarrelL2Rout[4] = {k_l2r3 - BAScut, k_l2r3 - BAScut, k_l2r3 - BAScut, k_l2r3 - BAScut};

      G4Polycone* barL2 =
        new G4Polycone("ECL Barrel Support barrel-back L", BASdelPhi + (-k_l2ang / 2.), k_l2ang, 4, BarrelL2Z, BarrelL2Rin, BarrelL2Rout);

      G4LogicalVolume* barL1_logi = new G4LogicalVolume(barL1, Materials::get("G4_Fe"), "ECLBarrelSupport-fwdL", 0, 0, 0);
      G4LogicalVolume* barL2_logi = new G4LogicalVolume(barL2, Materials::get("G4_Fe"), "ECLBarrelSupport-bckL", 0, 0, 0);





      for (int i = 0; i < 72 ; ++i) {

        G4Transform3D BASrs00 = G4RotateZ3D(BASoffset + i * BASdelPhi);
        G4Transform3D BASrs01 = G4RotateZ3D(-BASoffset + i * BASdelPhi);
        assemblyBarrelSupport->AddPlacedVolume(barL1_logi, BASrs00);
        assemblyBarrelSupport->AddPlacedVolume(barL2_logi, BASrs01);

      }
      assemblyBarrelSupport->MakeImprint(logical_ecl, Global_offset);


///////////////////////////
//////makeEndCapSupport/////
///////////////////////////

      G4AssemblyVolume* assemblyBackwordEndcapSupport = new G4AssemblyVolume();

      const int ECSnSectors = 16;
      const double ECSdPhi = 2 * PI / ECSnSectors;
      const double ECSoffset = ECSdPhi;

      // In fact they're not polycone, but for simplicity...
      //

      //backward
      //
      double BwECSw1 = 13.0 * cm;
      double BwECSh1 = 18.5 * cm;
      double BwECSt1 = 4.0 * cm;
      double BwECSro1 = 149.6 * cm;
      double BwECSri1 = BwECSro1 - BwECSh1;
      double BwECSdp1 = BwECSw1 / ((BwECSro1 + BwECSri1) / 2.0);
      double BwECSdz1 = BwECSt1;
      double BwECSz12 = k_backConFarZ;
      double BwECSz11 = BwECSz12 - BwECSdz1;
//   const TripletF mPar1[]={ // PolyCONe parameters
//      TripletF(z11, ri1, ro1),
//      TripletF(z12, ri1, ro1)};
//   const vTripletF vPar1(mPar1, mPar1+2) ;
//   aVolSpBwECS.push_back( new G3WSpecPCON
//                        (k_boS1Name, "ECL Backward Endcap Support 1st piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp1/k_degRad, vPar1 ) );

      double BackwordEndcapSupport1Z[2] = {BwECSz11, BwECSz12};
      double BackwordEndcapSupport1Rin[2] = {BwECSri1, BwECSri1};
      double BackwordEndcapSupport1Rout[2] = {BwECSro1, BwECSro1};

      G4Polycone* BackEcapP1 =
        new G4Polycone("ECL Backward Endcap Support 1st piece", 0.0, BwECSdp1, 2, BackwordEndcapSupport1Z, BackwordEndcapSupport1Rin, BackwordEndcapSupport1Rout);

      G4LogicalVolume* BackEcapP1_logi = new G4LogicalVolume(BackEcapP1, Materials::get("G4_Fe"), "ECLBwECSupport1st", 0, 0, 0);


      double BwECSw2 = 6.0 * cm;
      double BwECSh2 = 13.5 * cm;
      double BwECSt2 = 21.2 * cm;
      double BwECSri21 = BwECSri1 + 3.5 * cm;
      double BwECScut2 = 3.0 * cm;  // guess, I could not find exact fig.
      double BwECSri22 = BwECSri21 + BwECScut2; //cut2*tan(M_PI/4.0);
      double BwECSro2 = BwECSri21 + BwECSh2;
      double BwECSdp2 = BwECSw2 / ((BwECSro2 + BwECSri21) / 2.0);
      double BwECSz25 = BwECSz11;
      double BwECSz24 = BwECSz25 - BwECSt2 + BwECScut2;  //  *tan(M_PI/4.0);
      double BwECSz23 = BwECSz25 - BwECSt2;
//   const TripletF mPar2[]={ // PolyCONe parameters
//      TripletF(z23, ri22, ro2),
//      TripletF(z24, ri21, ro2),
//      TripletF(z25, ri21, ro2)};
//   const vTripletF vPar2(mPar2, mPar2+3) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (k_boS2Name, "ECL Backward Endcap Support 2nd piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp2/k_degRad, vPar2 ) );

      double BackwordEndcapSupport2Z[3] = {BwECSz23, BwECSz24, BwECSz25};
      double BackwordEndcapSupport2Rin[3] = {BwECSri22, BwECSri21, BwECSri21};
      double BackwordEndcapSupport2Rout[3] = {BwECSro2, BwECSro2, BwECSro2};
      G4Polycone* BackEcapP2 =
        new G4Polycone("ECL Backward Endcap Support 2st piece", 0.0, BwECSdp2, 3, BackwordEndcapSupport2Z, BackwordEndcapSupport2Rin, BackwordEndcapSupport2Rout);

      G4LogicalVolume* BackEcapP2_logi = new G4LogicalVolume(BackEcapP2, Materials::get("G4_Fe"), "ECLBwECSupport2nd", 0, 0, 0);



      double BwECSw3 = 14.0 * cm;
      double BwECSh3 = k_l2r3 - BwECSro2; //18.9;
      double BwECSt3 = 16.0 * cm;
      double BwECSri3 = BwECSro2;
      double BwECSro3 = BwECSri3 + BwECSh3;
      double BwECSdp3 = BwECSw3 / ((BwECSro3 + BwECSri3) / 2.0);
      double BwECSz37 = BwECSz11 - 5.7 * cm;
      double BwECSz36 = BwECSz37 - BwECSt3;
//   const TripletF mPar3[]={ // PolyCONe parameters
//      TripletF(z36, ri3, ro3),
//      TripletF(z37, ri3, ro3)};
//   const vTripletF vPar3(mPar3, mPar3+2) ;
//   aVolSpecs.push_back( new G3WSpecPCON
//                        (k_boS3Name, "ECL Backward Endcap Support 3rd piece" ,
//                         G3WMaterial::getIron() ,
//                         0.0, dp3/k_degRad, vPar3 ) );

      double BackwordEndcapSupport3Z[2] = {BwECSz36, BwECSz37};
      double BackwordEndcapSupport3Rin[2] = {BwECSri3, BwECSri3};
      double BackwordEndcapSupport3Rout[2] = {BwECSro3, BwECSro3};

      G4Polycone* BackEcapP3 =
        new G4Polycone("ECL Backward Endcap Support 3rd piece", 0.0, BwECSdp3, 2, BackwordEndcapSupport3Z, BackwordEndcapSupport3Rin, BackwordEndcapSupport3Rout);

      G4LogicalVolume* BackEcapP3_logi = new G4LogicalVolume(BackEcapP3, Materials::get("G4_Fe"), "ECLBwECSupport3rd", 0, 0, 0);



//////forward support


      G4AssemblyVolume* assemblyForwordEndcapSupport = new G4AssemblyVolume();

      EclCM FwECSw_1 = 13.0 * cm;
      EclCM FwECSh_1 = 17.0 * cm;
      EclCM FwECSt_1 = 4.0 * cm;
      EclCM FwECSro_1 = 142.0 * cm;
      EclCM FwECSri_1 = FwECSro_1 - FwECSh_1;
      EclRad FwECSdp_1 = FwECSw_1 / ((FwECSro_1 + FwECSri_1) / 2.0);
      double FwECSz_11 = k_forwConFarZ;
      double FwECSz_12 = FwECSz_11 + FwECSt_1;
//      TripletF(z_11, ri_1, ro_1),
//      TripletF(z_12, ri_1, ro_1);
//   const vTripletF vPar_1(mPar_1, mPar_1+2) ;
      double ForwordEndcapSupport1Z[2] = {FwECSz_11, FwECSz_12};
      double ForwordEndcapSupport1Rin[2] = {FwECSri_1, FwECSri_1};
      double ForwordEndcapSupport1Rout[2] = {FwECSro_1, FwECSro_1};

      G4Polycone* FWEcapP1 =
        new G4Polycone("ECL forward Endcap Support 1st piece", 0.0, FwECSdp_1, 2, ForwordEndcapSupport1Z, ForwordEndcapSupport1Rin, ForwordEndcapSupport1Rout);

      G4LogicalVolume* FWEcapP1_logi = new G4LogicalVolume(FWEcapP1, Materials::get("G4_Fe"), "ECLFwECSupport1st", 0, 0, 0);

      EclCM FwECSw_2 = 6.0 * cm;
      EclCM FwECSh_2 = 13.0 * cm;
      EclCM FwECSt_2 = 13.7 * cm;
      EclCM FwECSri_2 = FwECSri_1 + 3.5 * cm;
      EclCM FwECSro_2 = FwECSri_2 + FwECSh_2;
      EclRad FwECSdp_2 = FwECSw_2 / ((FwECSro_2 + FwECSri_2) / 2.0);
      double FwECSz_21 = FwECSz_12;
      double FwECSz_22 = FwECSz_21 + FwECSt_2;
//      TripletF(z_21, ri_2, ro_2),
//      TripletF(z_22, ri_2, ro_2);
      double ForwordEndcapSupport2Z[2] = {FwECSz_21, FwECSz_22};
      double ForwordEndcapSupport2Rin[2] = {FwECSri_2, FwECSri_2};
      double ForwordEndcapSupport2Rout[2] = {FwECSro_2, FwECSro_2};

      G4Polycone* FWEcapP2 =
        new G4Polycone("ECL forward Endcap Support 2rd piece", 0.0, FwECSdp_2, 2, ForwordEndcapSupport2Z, ForwordEndcapSupport2Rin, ForwordEndcapSupport2Rout);

      G4LogicalVolume* FWEcapP2_logi = new G4LogicalVolume(FWEcapP2, Materials::get("G4_Fe"), "ECLFwECSupport2rd", 0, 0, 0);


      EclCM FwECSw_3 = 14.0 * cm;
      EclCM FwECSh_3 = 26.5 * cm;
      EclCM FwECSt_3 = 7.0 * cm;
      EclCM FwECSri_3 = FwECSri_2;
      EclCM FwECSro_3 = FwECSri_3 + FwECSh_3;
      EclRad FwECSdp_3 = FwECSw_3 / ((FwECSro_3 + FwECSri_3) / 2.0);
      double FwECSz_31 = FwECSz_22;
      double FwECScut_3 = 3.0 * cm;
      double FwECSz_32 = FwECSz_31 + FwECSt_3 - FwECScut_3;
      double FwECSz_33 = FwECSz_31 + FwECSt_3;

//      TripletF(z_31, ri_3, ro_3),
//      TripletF(z_32, ri_3, ro_3),
//      TripletF(z_33, ri_2 +cut_3, ro_3);
      double ForwordEndcapSupport3Z[3] = {FwECSz_31, FwECSz_32, FwECSz_33};
      double ForwordEndcapSupport3Rin[3] = {FwECSri_3, FwECSri_3, FwECSri_2 + FwECScut_3};
      double ForwordEndcapSupport3Rout[3] = {FwECSro_3, FwECSro_3 , FwECSro_3};




      G4Polycone* FWEcapP3 =
        new G4Polycone("ECL forward Endcap Support 3rd piece", 0.0, FwECSdp_3, 3, ForwordEndcapSupport3Z, ForwordEndcapSupport3Rin, ForwordEndcapSupport3Rout);

      G4LogicalVolume* FWEcapP3_logi = new G4LogicalVolume(FWEcapP3, Materials::get("G4_Fe"), "ECLFwECSupport3rd", 0, 0, 0);


      EclCM FwECSw_4 = FwECSw_3;
      EclCM FwECSh_4 = 16.0 * cm;
      EclCM FwECSt_4 = 16.0 * cm;
      EclCM FwECSro_4 = FwECSro_3 + 4.0 * cm;
      EclCM FwECSri_4 = FwECSro_4 - FwECSh_4;
      EclRad FwECSdp_4 = FwECSw_4 / ((FwECSro_4 + FwECSri_4) / 2.0);
      double FwECSz_42 = FwECSz_31;
      double FwECSz_41 = FwECSz_42 - FwECSt_4 / 2.0;
      double FwECSz_43 = FwECSz_42;
      double FwECSz_44 = FwECSz_41 + FwECSt_4;

//      TripletF(z_41, ri_4, ro_4),
//      TripletF(z_42, ri_4, ro_4),
//      TripletF(z_43, ro_4 -3.0, ro_4),
//      TripletF(z_44, ro_4 -3.0, ro_4);
      double ForwordEndcapSupport4Z[4] = {FwECSz_41, FwECSz_42, FwECSz_43, FwECSz_44};
      double ForwordEndcapSupport4Rin[4] = {FwECSri_4, FwECSri_4, FwECSro_4 - 3.0 * cm, FwECSro_4 - 3.0 * cm };
      double ForwordEndcapSupport4Rout[4] = {FwECSro_4, FwECSro_4, FwECSro_4, FwECSro_4};


      G4Polycone* FWEcapP4 =
        new G4Polycone("ECL forward Endcap Support 4th piece", 0.0, FwECSdp_4, 4, ForwordEndcapSupport4Z, ForwordEndcapSupport4Rin, ForwordEndcapSupport4Rout);

      G4LogicalVolume* FWEcapP4_logi = new G4LogicalVolume(FWEcapP4, Materials::get("G4_Fe"), "ECLFwECSupport4nd", 0, 0, 0);


      EclCM FwECSw_5 = FwECSw_4;
      EclCM FwECSh_5 = k_l2r3 - FwECSro_4; //8.0;
      //EclCM FwECSt_5 = FwECSt_4;
      EclCM FwECSri_5 = FwECSro_4;
      EclCM FwECSro_5 = FwECSri_5 + FwECSh_5;
      EclRad FwECSdp_5 = FwECSw_5 / ((FwECSro_5 + FwECSri_5) / 2.0);
      double FwECSz_51 = FwECSz_41;
      double FwECSz_52 = FwECSz_44;
//      TripletF(z_51, ri_5, ro_5),
//      TripletF(z_52, ri_5, ro_5);

      double ForwordEndcapSupport5Z[2] = {FwECSz_51, FwECSz_52};
      double ForwordEndcapSupport5Rin[2] = {FwECSri_5, FwECSri_5};
      double ForwordEndcapSupport5Rout[2] = {FwECSro_5, FwECSro_5};

      G4Polycone* FWEcapP5 =
        new G4Polycone("ECL forward Endcap Support 5th piece", 0.0, FwECSdp_5, 2, ForwordEndcapSupport5Z, ForwordEndcapSupport5Rin, ForwordEndcapSupport5Rout);

      G4LogicalVolume* FWEcapP5_logi = new G4LogicalVolume(FWEcapP5, Materials::get("G4_Fe"), "ECLFwECSupport5nd", 0, 0, 0);





      for (int i = 0; i < (ECSnSectors / 2); ++i) {

        G4Transform3D ECSrot1 = G4RotateZ3D(-BwECSdp1 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot2 = G4RotateZ3D(-BwECSdp2 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot3 = G4RotateZ3D(-BwECSdp3 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot_1 = G4RotateZ3D(-FwECSdp_1 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot_2 = G4RotateZ3D(-FwECSdp_2 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot_3 = G4RotateZ3D(-FwECSdp_3 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot_4 = G4RotateZ3D(-FwECSdp_4 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);
        G4Transform3D ECSrot_5 = G4RotateZ3D(-FwECSdp_5 / 2.0 + (2 * i) * ECSdPhi + ECSoffset);



        assemblyBackwordEndcapSupport->AddPlacedVolume(BackEcapP1_logi, ECSrot1);
        assemblyBackwordEndcapSupport->AddPlacedVolume(BackEcapP2_logi, ECSrot2);
        assemblyBackwordEndcapSupport->AddPlacedVolume(BackEcapP3_logi, ECSrot3);
        assemblyForwordEndcapSupport->AddPlacedVolume(FWEcapP1_logi, ECSrot_1);
        assemblyForwordEndcapSupport->AddPlacedVolume(FWEcapP2_logi, ECSrot_2);
        assemblyForwordEndcapSupport->AddPlacedVolume(FWEcapP3_logi, ECSrot_3);
        assemblyForwordEndcapSupport->AddPlacedVolume(FWEcapP4_logi, ECSrot_4);
        assemblyForwordEndcapSupport->AddPlacedVolume(FWEcapP5_logi, ECSrot_5);
      }

      assemblyBackwordEndcapSupport->MakeImprint(logical_ecl, Global_offset);
      assemblyForwordEndcapSupport->MakeImprint(logical_ecl, Global_offset);
    }//makeSupport

    void GeoECLCreator::makeEndcap(const bool aForward)
    {
      const int nPhiSegs
      (aForward ? k_forwNPhiSegs : k_backNPhiSegs) ;

      const double
      signZ(aForward ? 1 : -1) ;
      const EclCM
      farZ(aForward ? k_forwConFarZ      : k_backConFarZ) ,
           farInnerR(aForward ? k_forwConFarInnerR : k_backConFarInnerR) ,
           farOuterR(aForward ? k_forwConFarOuterR : k_backConFarOuterR) ,
           flatZ(aForward ? farZ - k_forwConHflat : farZ + k_backConHflat) ,
           nearZ(aForward ? k_forwConNearZ     : k_backConNearZ) ,
           nearInnerR(aForward ? k_forwConNearInnerR : k_backConNearInnerR) ,
           nearOuterR(aForward ? k_forwConNearOuterR : k_backConNearOuterR) ;
      const double
      tanOuterAng(signZ * (farOuterR - nearOuterR) / (flatZ - nearZ)) ,
                  cosOuterAng(cos(atan(tanOuterAng))) ,
//                  sinOuterAng(sin(atan(tanOuterAng))) ,
                  tanInnerAng(signZ * (farInnerR - nearInnerR) / (farZ - nearZ)) ,
                  cosInnerAng(cos(atan(tanInnerAng))) ;
      const EclCM
//      rInnerFlat(nearInnerR + signZ * (flatZ - nearZ)*tanInnerAng) ,
      suiThick(k_endConInnerThick / cosInnerAng) ,
               farCryZ(farZ - signZ * k_endConFarThick),
               farCryInnerR(farInnerR - k_endConFarThick * tanInnerAng + suiThick) ,
               farCryOuterR(farOuterR - k_endConOuterThick) ,
               sthick(k_endConSiWallThick / cosOuterAng) ,
               flatCryZ(flatZ
                        - signZ * (k_endConOuterThick - sthick) / tanOuterAng) ,
               nearCryZ(nearZ + signZ * k_endConNearThick) ,
               nearCryInnerR(nearInnerR + k_endConNearThick * tanInnerAng + suiThick) ,
               nearCryOuterR(nearOuterR) ,
               flatCryInnerR(nearCryInnerR +
                             signZ * (flatCryZ - nearCryZ)*tanInnerAng) ;

      const EclRad deltaPhi(2 * M_PI / nPhiSegs) ;

//      const EclRad theta(atan(tanInnerAng)) ;

      const EclCM finGap(0.01 * cm) ;
      const EclCM pF_a(nearCryOuterR - nearCryInnerR - finGap) ;
      const EclCM pF_A(farCryOuterR - flatCryInnerR - finGap) ;
      const EclCM pF_L(signZ * (flatCryZ - nearCryZ)) ;
      const EclCM pF_h(k_endConFinThick / 2) ;
      const G4Point3D front(pF_a / 2, pF_h / 2, 0) ;
      const G4Point3D back(flatCryInnerR - nearCryInnerR + pF_A / 2,
                           pF_h / 2, -pF_L) ;
      const EclCM pF_th(G4Vector3D(front - back).theta()) ;
      const EclCM pF_ph(G4Vector3D(front - back).phi()) ;
//  const float phFinA[] = { pF_L/2, pF_th/k_degRad, pF_ph/k_degRad,
//                           pF_h/2, pF_A/2, pF_A/2, 0,
//                           pF_h/2, pF_a/2, pF_a/2, 0 } ;
//  const std::vector< float > phFin ( phFinA, phFinA + 11 ) ;

//  aVolSpecs.push_back( new G3WSpecTRAP( phiFinName , phiFinDescrip ,
//                                          G3WMaterial::getAluminum() ));
//  G3WVolumeSpec* phiFinVS ( G3WVolumeSpec::findVol( phiFinName , &aVolSpecs ));

      const EclCM pF_xf(pF_L * tan(pF_th)*cos(pF_ph) / 2) ;
      const G4Point3D f1(-pF_xf - pF_A / 2 , pF_h / 2 , -pF_L / 2) ;
      const G4Point3D f2(pF_xf - pF_a / 2 , pF_h / 2 ,  pF_L / 2) ;
      const G4Point3D f3(pF_xf + pF_a / 2 , pF_h / 2 ,  pF_L / 2) ;
      const G4Point3D s1(flatCryInnerR , 0, flatCryZ) ;
      const G4Point3D s2(nearCryInnerR , 0, nearCryZ) ;
      const G4Point3D s3(nearCryOuterR - finGap , 0, nearCryZ) ;

      G4Transform3D phFinTr0(f1, f2, f3, s1, s2, s3) ;
      G4Transform3D phFinTr1((aForward ? G4Transform3D::Identity :
                              G4Transform3D(G4TranslateY3D(pF_h)))
                             *phFinTr0) ;

      G4Transform3D phFinTr2(G4RotateZ3D(deltaPhi)*
                             G4TranslateY3D(-pF_h)*phFinTr1) ;

      G4LogicalVolume* EndcapPhiFin = new G4LogicalVolume(
        new G4Trap("EndcapPhiFin", pF_L / 2, pF_th, pF_ph, pF_h / 2, pF_A / 2, pF_A / 2, 0, pF_h / 2, pF_a / 2, pF_a / 2, 0)
        , Materials::get("G4_Al"), "EndcapPhiFin", 0, 0, 0);

      // -- begin; reinforcing bars
      const EclCM rfL1 = 2.0 * cm;
      const EclCM rfL2 = 4.0 * cm;
      const EclCM rfL = rfL1 + rfL2;
      const EclCM rfThick = 4.0 * cm;
      const double rfSinPo2 = (rfThick / 2.0) / farCryOuterR;
      const double rfCosPo2 = sqrt(1. -  rfSinPo2 * rfSinPo2);
      const EclCM rfOuterR = farCryOuterR * rfCosPo2; //farCryOuterR;
      const EclCM rfr3 = farCryInnerR;
      const EclCM rfr1 = rfr3 - rfL * tanInnerAng;
      const EclCM rfr2 = rfr3 - rfL2 * tanInnerAng;
      const EclCM rfz3 = farCryZ;
      const EclCM rfz2 = rfz3 - signZ * rfL2;
      const EclCM rfz1 = rfz2 - signZ * rfL1;

      const EclCM rfGap(0.1 * cm) ;
      const EclCM rf2_a(rfOuterR - rfr2 - rfGap) ;
      const EclCM rf2_A(rfOuterR - rfr3 - rfGap) ;
      const EclCM rf2_L(signZ * (rfz3 - rfz2)) ;
      const EclCM rf2_h(rfThick / 2) ;
      const G4Point3D rf2_front(rf2_a / 2, rf2_h / 2, 0) ;
      const G4Point3D rf2_back(rfr3 - rfr2 + rf2_A / 2,
                               rf2_h / 2, -rf2_L) ;
      const EclCM rf2_th(G4Vector3D(rf2_front - rf2_back).theta()) ;
      const EclCM rf2_ph(G4Vector3D(rf2_front - rf2_back).phi()) ;
//  const float rf2_par[] = { rf2_L/2, rf2_th/k_degRad, rf2_ph/k_degRad,
//                            rf2_h/2, rf2_A/2, rf2_A/2, 0,
//                            rf2_h/2, rf2_a/2, rf2_a/2, 0 } ;
//  const std::vector< float > rf2 ( rf2_par, rf2_par + 11 ) ;
      G4LogicalVolume* EndcapReinforcingBar2 = new G4LogicalVolume(
        new G4Trap("Endcap reinforcing bar2", rf2_L / 2, rf2_th, rf2_ph, rf2_h / 2, rf2_A / 2, rf2_A / 2, 0, rf2_h / 2, rf2_a / 2, rf2_a / 2, 0)
        , Materials::get("G4_Fe"), "EndcapReinforcingBar2", 0, 0, 0);

      const EclCM rf2_xf(rf2_L * tan(rf2_th)*cos(rf2_ph) / 2) ;
      const G4Point3D rf2_f1(-rf2_xf - rf2_A / 2 , rf2_h / 2 , -rf2_L / 2) ;
      const G4Point3D rf2_f2(rf2_xf - rf2_a / 2 , rf2_h / 2 ,  rf2_L / 2) ;
      const G4Point3D rf2_f3(rf2_xf + rf2_a / 2 , rf2_h / 2 ,  rf2_L / 2) ;
      const G4Point3D rf2_s1(rfr3 , 0, rfz3) ;
      const G4Point3D rf2_s2(rfr2 , 0, rfz2) ;
      const G4Point3D rf2_s3(rfOuterR - rfGap , 0, rfz2) ;

      G4Transform3D rf2Tr0(rf2_f1, rf2_f2, rf2_f3,
                           rf2_s1, rf2_s2, rf2_s3) ;
      G4Transform3D rf2Tr1((aForward ? G4Transform3D::Identity :
                            G4Transform3D(G4TranslateY3D(rf2_h)))
                           *rf2Tr0) ;
      G4Transform3D rf2Tr2(G4RotateZ3D(deltaPhi)*
                           G4TranslateY3D(-rf2_h)*rf2Tr1) ;


      const EclCM rf_a(rfOuterR - rfr1 - rfGap) ;
      const EclCM rf_A(rfOuterR - rfr2 - rfGap) ;
      const EclCM rf_L(signZ * (rfz2 - rfz1)) ;
      const EclCM rf_h(rfThick / 2) ;
      const G4Point3D rf_front(rf_a / 2, rf_h / 2, 0) ;
      const G4Point3D rf_back(rfr2 - rfr1 + rf_A / 2,
                              rf_h / 2, -rf_L) ;
      const EclCM rf_th(G4Vector3D(rf_front - rf_back).theta()) ;
      const EclCM rf_ph(G4Vector3D(rf_front - rf_back).phi()) ;
//  const float rf_par[] = { rf_L/2, rf_th/k_degRad, rf_ph/k_degRad,
//                           rf_h/2, rf_A/2, rf_A/2, 0,
//                           rf_h/2, rf_a/2, rf_a/2, 0 } ;
//  const std::vector< float > rf ( rf_par, rf_par + 11 ) ;
      G4LogicalVolume* EndcapReinforcingBar = new G4LogicalVolume(
        new G4Trap("Endcap reinforcing bar", rf_L / 2, rf_th, rf_ph, rf_h / 2, rf_A / 2, rf_A / 2, 0, rf_h / 2, rf_a / 2, rf_a / 2, 0)
        , Materials::get("G4_Fe"), "EndcapReinforcingBar", 0, 0, 0);

      const EclCM rf_xf(rf_L * tan(rf_th)*cos(rf_ph) / 2) ;
      const G4Point3D rf_f1(-rf_xf - rf_A / 2 , rf_h / 2 , -rf_L / 2) ;
      const G4Point3D rf_f2(rf_xf - rf_a / 2 , rf_h / 2 ,  rf_L / 2) ;
      const G4Point3D rf_f3(rf_xf + rf_a / 2 , rf_h / 2 ,  rf_L / 2) ;
      const G4Point3D rf_s1(rfr2 , 0, rfz2) ;
      const G4Point3D rf_s2(rfr1 , 0, rfz1) ;
      const G4Point3D rf_s3(rfOuterR - rfGap , 0, rfz1) ;

      G4Transform3D rfTr0(rf_f1, rf_f2, rf_f3,
                          rf_s1, rf_s2, rf_s3) ;
      G4Transform3D rfTr1((aForward ? G4Transform3D::Identity :
                           G4Transform3D(G4TranslateY3D(rf_h)))
                          *rfTr0) ;






      G4AssemblyVolume* assemblyEndcap = new G4AssemblyVolume();
      assemblyEndcap->AddPlacedVolume(EndcapPhiFin, phFinTr1);
      assemblyEndcap->AddPlacedVolume(EndcapPhiFin, phFinTr2);
      assemblyEndcap->AddPlacedVolume(EndcapReinforcingBar2, rf2Tr1);
      assemblyEndcap->AddPlacedVolume(EndcapReinforcingBar2, rf2Tr2);

      if (!aForward) {
        assemblyEndcap->AddPlacedVolume(EndcapReinforcingBar, rfTr1);
      } else {
        G4Transform3D rfTr2(G4RotateZ3D(deltaPhi)*
                            G4TranslateY3D(-rf_h)*rfTr1) ;
        assemblyEndcap->AddPlacedVolume(EndcapReinforcingBar, rfTr2);
      }



      for (int iPhiSeg(1) ; iPhiSeg != 1 + nPhiSegs ; ++iPhiSeg) {
        G4RotateZ3D rotZ((iPhiSeg - 1)*deltaPhi) ;
        G4Transform3D EndcapSegRot = G4RotateZ3D((iPhiSeg - 1) * deltaPhi);
        assemblyEndcap->MakeImprint(logical_ecl, EndcapSegRot);
      }


    }//makeEndcap



  }//ecl
}//belle2
