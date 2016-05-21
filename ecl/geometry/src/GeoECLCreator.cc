/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4Material.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4Trap.hh>
#include <G4UnionSolid.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4Transform3D.hh>
#include "G4SDManager.hh"

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#define TWOPI (2.0*M_PI)

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

    const bool overlaps(false);

    const int k_barNPhiSegs(144);
    const int k_forwNPhiSegs(16);
    const int k_backNPhiSegs(16);

    // warned as unused 20140503 const EclIdentifier
    // warned as unused 20140503 k_barNRings(46) ,
    // warned as unused 20140503 k_forwNRings(13) ,
    // warned as unused 20140503 k_backNRings(10);

    //const double k_CLR(0.01   * CLHEP::cm);                 // clearance around outside
    //const double k_barCryPullOut    (  0.5   *CLHEP::cm ); // Barrel Crystal pullout
    const double k_barFinThick(0.05  * CLHEP::cm);         // fin thickness in barrel
    const double k_barConOuterR(166.0 * CLHEP::cm);        // outer R of barrel container
    const double k_barConOuterThick(0.8 * CLHEP::cm);      // thickness (eff) of outer skin
    const double k_barConInnerR(125.01 * CLHEP::cm);       // inner R of barrel container
    const double k_barConInnerThick(0.15 * CLHEP::cm);     // thickness of inner skin
    const double k_barConBackZOut(-122.50 * CLHEP::cm);    // Z of container back at outer R
    const double k_barConBackZIn(-100.58 * CLHEP::cm);     // Z of container back at inner R
    const double k_barConForwZIn(199.1 * CLHEP::cm);       // Z of container forw at inner R
    const double k_barConForwZOut(229.0 * CLHEP::cm);      // Z of container forw at outer R
    const double k_barConEndConThick(0.4 * CLHEP::cm);     // endcone thickess, both ends
    const double k_barConEndRngThick(0.2 * CLHEP::cm);     // endring thickess, both ends
    const double k_barConBackRThick(10.5 * CLHEP::cm);     // thickness of sup ring in back
    const double k_barConForwRThick(9.0 * CLHEP::cm);      // thickness of sup ring in forw
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
    //const double k_endConMetThick   (  1.34  *CLHEP::cm ); // ec's metal fitting thickness

    //  const double k_barCryPhiAng  (  2.500*CLHEP::deg ); // crystal azimuthal width
    //  const double k_barPhiTiltAng (  1.250*CLHEP::deg ); // crystal tilt in phi
    const double k_barBackAng(52.902 * CLHEP::deg);     // theta at container back
    const double k_barForwAng(32.987 * CLHEP::deg);     // theta at container front

    const double k_barBackSin(sin(k_barBackAng));
    const double k_barBackCos(cos(k_barBackAng));
    const double k_barBackTan(tan(k_barBackAng));
    const double k_barForwSin(sin(k_barForwAng));
    const double k_barForwCos(cos(k_barForwAng));
    const double k_barForwTan(tan(k_barForwAng));

    const double k_barCryOuterR(k_barConOuterR - k_barConOuterThick);

    const double k_barCryInnerR(k_barConInnerR + k_barConInnerThick);

    const double k_barCryH(32.0 * CLHEP::cm);  // rough
    const double k_barCryMargin(.8 * CLHEP::cm);  // rough, work
    const double k_barCryOuterQ(k_barCryInnerR + k_barCryH);

    const double k_barCryBackZOut(k_barConBackZOut + k_barConEndRngThick);
    const double k_barCryForwZOut(k_barConForwZOut - k_barConEndRngThick);
    const double k_barSupBackFarZ(k_barConBackZOut - k_barConBackRThick);
    const double k_barSupForwFarZ(k_barConForwZOut + k_barConForwRThick);
    const double k_barSupForwFarIR(k_barConInnerR +
                                   (k_barSupForwFarZ - k_barConForwZIn)*
                                   k_barForwTan);
    // unused const double k_barCryBackZIn(k_barConBackZIn + k_barFinThick / k_barBackSin);
    // unused const double k_barCryForwZIn(k_barConForwZIn - k_barFinThick / k_barForwSin);
    const double k_barConBackR5(k_barCryInnerR + k_barFinThick / k_barBackCos);
    const double k_barConForwR5(k_barCryInnerR + k_barFinThick / k_barForwCos);
    const double k_barConTFC(k_barFinThick + k_barConEndConThick);
    const double k_barConBackRg(k_barConTFC / k_barBackCos);
    const double k_barConBackR4(k_barConInnerR + k_barConBackRg);
    const double k_barConForwRg(k_barConTFC / k_barForwCos);
    const double k_barConForwR4(k_barConInnerR + k_barConForwRg);
    const double k_barConBackZ3(k_barConBackZIn -
                                (k_barConTFC *
                                 (k_barBackSin + k_barBackCos / k_barBackTan) -
                                 (k_barFinThick / k_barBackCos +
                                  k_barConInnerThick) / k_barBackTan));
    const double k_barConForwZ3(k_barConForwZIn +
                                (k_barConTFC *
                                 (k_barForwSin + k_barForwCos / k_barForwTan) -
                                 (k_barFinThick / k_barForwCos +
                                  k_barConInnerThick) / k_barForwTan));
    const double k_barConBackR2(k_barCryInnerR + k_barBackTan *
                                (k_barConBackZ3 - k_barCryBackZOut));
    const double k_barConForwR2(k_barCryInnerR - k_barForwTan *
                                (k_barConForwZ3 - k_barCryForwZOut));
    const double k_barConBackR3(k_barConBackR2 + k_barConBackRg);
    const double k_barConForwR3(k_barConForwR2 + k_barConForwRg);
    const double k_barConBackR1(k_barConBackR2 +
                                k_barConEndRngThick* k_barBackTan);
    const double k_barConForwR1(k_barConForwR2 +
                                k_barConEndRngThick* k_barForwTan);

    const double k_c1z3(k_barSupForwFarZ + 31.*CLHEP::cm);
    const double k_c1z2(k_c1z3 - 1.*CLHEP::cm);
    const double k_c1z1(k_c1z3 - 75.*CLHEP::cm);
    const double k_c1r1(167.0 * CLHEP::cm);
    const double k_c1r2(k_c1r1 + 2.0 * CLHEP::cm);
    const double k_c1r3(k_c1r1 + 32.0 * CLHEP::cm);

    const double k_c2z1(k_c1z3 - 444.0 * CLHEP::cm);
    const double k_c2z2(k_c2z1 + 1.0 * CLHEP::cm);
    const double k_c2z3(k_c2z1 + 75.*CLHEP::cm);
    const double k_c2r1(k_c1r1);
    const double k_c2r2(k_c1r2);
    const double k_c2r3(k_c1r3);

    const double k_l1z1(k_barSupForwFarZ);
    const double k_l1z2(k_l1z1 + 3.5 * CLHEP::cm);
    const double k_l1z3(k_l1z1 + 10.0 * CLHEP::cm);
    const double k_l1r4(k_c1r1 - 0.2 * CLHEP::cm);
    const double k_l1r3(k_l1r4 - 3.5 * CLHEP::cm);
    const double k_l1r1(k_l1r4 - 15.8 * CLHEP::cm);
    const double k_l1r2(k_l1r1 + 2.45 * CLHEP::cm);
    const double k_l1ang(6.3 * CLHEP::cm / ((k_l1r1 + k_l1r4) / 2.0));

    const double k_l2z3(k_barSupBackFarZ);
    const double k_l2z2(k_l2z3 - 3.5 * CLHEP::cm);
    const double k_l2z1(k_l2z3 - 12.0 * CLHEP::cm);
    const double k_l2r3(k_l1r4);
    const double k_l2r2(k_l2r3 - 3.5 * CLHEP::cm);
    const double k_l2r1(k_l2r3 - 11.8 * CLHEP::cm);
    const double k_l2ang(9.0 * CLHEP::cm / ((k_l2r1 + k_l2r3) / 2.0));

    // add foil thickness //
    const double foilthickness = 0.0100 * CLHEP::cm; // crystal wrapping foil 100 um
    const double thinfoilthickness = foilthickness * 0.8; // thin crystal wrapping foil 80 um
    const double thinpentafoilthickness = foilthickness * 0.2; // pentagon crystal foil
    const double brthetafinthickness = 0.0500 * CLHEP::cm; // barrel theta fin 500 um
    const double brphifinthickness = 0.0500 * CLHEP::cm; // barrel phi fin 500 um

    GeoECLCreator::GeoECLCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector("ECLSensitiveDetector", (2 * 24)*CLHEP::eV, 10 * CLHEP::MeV);
      G4SDManager::GetSDMpointer()->AddNewDetector(m_sensitive);
      m_bkgsensitive.clear();
    }


    GeoECLCreator::~GeoECLCreator()
    {
      //      delete m_sensitive;
      for (BkgSensitiveDetector* sensitive : m_bkgsensitive) delete sensitive;
      m_bkgsensitive.clear();
    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {

      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");
      G4Material* medAir = geometry::Materials::get("Air");
      G4ThreeVector noMove(0.0, 0.0, 0.0);

      // ECL container (barrel + endcaps + supports)

      // eclSolid now contains the barrel's forward and backward outer support cylinders

      double eclSolid_I[16];
      double eclSolid_O[16];
      double eclSolid_Z[16];

      eclSolid_Z[0] = k_c2z1;
      eclSolid_I[0] = k_l2r3 - 36.0 * CLHEP::cm;
      eclSolid_O[0] = k_c2r3;

      eclSolid_Z[1] = k_c2z2;
      eclSolid_I[1] = k_l2r3 - 36.0 * CLHEP::cm;
      eclSolid_O[1] = k_c2r3;

      eclSolid_Z[2] = k_c2z2;
      eclSolid_I[2] = k_l2r3 - 36.0 * CLHEP::cm;
      eclSolid_O[2] = k_c1r2;

      eclSolid_Z[3] = k_backConFarZ;
      eclSolid_I[3] = k_l2r3 - 36.0 * CLHEP::cm;
      eclSolid_O[3] = k_c1r2;

      eclSolid_Z[4] = k_backConFarZ;
      eclSolid_I[4] = k_backConFarInnerR;
      eclSolid_O[4] = k_c1r2;

      eclSolid_Z[5] = k_backConNearZ;
      eclSolid_I[5] = k_backConNearInnerR;
      eclSolid_O[5] = k_c1r2;

      eclSolid_Z[6] = k_backConNearZ;
      eclSolid_I[6] = k_barConInnerR;
      eclSolid_O[6] = k_c1r2;

      eclSolid_Z[7] = k_forwConNearZ;
      eclSolid_I[7] = k_barConInnerR;
      eclSolid_O[7] = k_c1r2;

      eclSolid_Z[8] = k_forwConNearZ;
      eclSolid_I[8] = k_forwConNearInnerR;
      eclSolid_O[8] = k_c1r2;

      eclSolid_Z[9] = k_forwConFarZ;
      eclSolid_I[9] = k_forwConFarInnerR;
      eclSolid_O[9] = k_c1r2;

      eclSolid_Z[10] = k_forwConFarZ;
      eclSolid_I[10] = k_l2r3  - 42.0 * CLHEP::cm;
      eclSolid_O[10] = k_c1r2;

      eclSolid_Z[11] = k_forwConFarZ + 6.0 * CLHEP::cm;
      eclSolid_I[11] = k_l2r3  - 42.0 * CLHEP::cm;
      eclSolid_O[11] = k_c1r2;

      eclSolid_Z[12] = k_forwConFarZ + 6.0 * CLHEP::cm;
      eclSolid_I[12] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      eclSolid_O[12] = k_c1r2;

      eclSolid_Z[13] = k_c1z2;
      eclSolid_I[13] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      eclSolid_O[13] = k_c1r2;

      eclSolid_Z[14] = k_c1z2;
      eclSolid_I[14] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      eclSolid_O[14] = k_c1r3;

      eclSolid_Z[15] = k_c1z3;
      eclSolid_I[15] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      eclSolid_O[15] = k_c1r3;

      G4Polycone* eclSolid = new G4Polycone("eclSolid", 0, TWOPI, 16, eclSolid_Z, eclSolid_I, eclSolid_O);
      G4LogicalVolume* eclLogical = new G4LogicalVolume(eclSolid, medAir, "eclLogical");
      new G4PVPlacement(0, noMove, eclLogical, "eclPhysical", &topVolume, false, 0, overlaps);

      // Place barrel and endcaps, including support structures, in ECL container

      makeBarrel(content, eclLogical);
      makeForwardEndcap(content, eclLogical);
      makeBackwardEndcap(content, eclLogical);

    }

    void GeoECLCreator::makeBarrel(const GearDir& content, G4LogicalVolume* eclLogical)
    {

      G4Material* medAir = geometry::Materials::get("Air");
      G4Material* medIron = geometry::Materials::get("G4_Fe");
      G4Material* medCsI = geometry::Materials::get(content.getString("CsI"));
      G4Material* medSi = geometry::Materials::get(content.getString("Si"));
      G4Material* medAlTeflon = geometry::Materials::get(content.getString("AlTeflon"));
      G4Material* medAl = geometry::Materials::get(content.getString("Al"));

      G4ThreeVector noMove(0.0, 0.0, 0.0);

      // create a diode in case it is needed for background study
      double diodeWidth = content.getLength("k_diodewidth") * CLHEP::cm;
      double diodeLength = content.getLength("k_diodelength") * CLHEP::cm;
      double diodeHeight = content.getLength("k_diodeheight") * CLHEP::cm;
      G4Box* diodeSolid = new G4Box("diode", diodeWidth / 2, diodeLength / 2, diodeHeight / 2);

      // Barrel outer support cylinders (one at each end)

      double cylinderF_Z[4] = {k_c1z1, k_c1z2, k_c1z2, k_c1z3};
      double cylinderF_I[4] = {k_c1r1, k_c1r1, k_c1r1, k_c1r1};
      double cylinderF_O[4] = {k_c1r2, k_c1r2, k_c1r3, k_c1r3};
      G4Polycone* supportFSolid =
        new G4Polycone("eclBarrelSupportForwSolid", 0, TWOPI, 4, cylinderF_Z, cylinderF_I, cylinderF_O);
      G4LogicalVolume* supportFLogical = new G4LogicalVolume(supportFSolid, medIron, "eclBarrelSupportForwLogical");
      new G4PVPlacement(0, noMove, supportFLogical, "eclBarrelSupportForwPhysical", eclLogical, false, 0, overlaps);

      double cylinderB_Z[4] = {k_c2z1, k_c2z2, k_c2z2, k_c2z3};
      double cylinderB_I[4] = {k_c2r1, k_c2r1, k_c2r1, k_c2r1};
      double cylinderB_O[4] = {k_c2r3, k_c2r3, k_c2r2, k_c1r2};
      G4Polycone* supportBSolid =
        new G4Polycone("eclBarrelSupportBackSolid", 0, TWOPI, 4, cylinderB_Z, cylinderB_I, cylinderB_O);
      G4LogicalVolume* supportBLogical = new G4LogicalVolume(supportBSolid, medIron, "eclBarrelSupportBackLogical");
      new G4PVPlacement(0, noMove, supportBLogical, "eclBarrelSupportBackPhysical", eclLogical, false, 0, overlaps);

      // Make barrel support pieces - axial ribs and pipes - first since some of
      // these geometry parameters are needed later.

      const EclRad outerSectorOffset = (2.796 - 2.5) * CLHEP::deg;
      const int nSectors = k_barNPhiSegs / 2;
      const EclRad dPhi = TWOPI / nSectors;

      const EclCM rib1w = 5.05 / 2 * CLHEP::cm;
      const EclCM rib1h = (2.5 + 0.3) * CLHEP::cm; // additional pusher
      const EclCM rib1z1 = k_barCryBackZOut;
      const EclCM rib1z2 = k_barCryForwZOut;
      const EclCM rib1ro = k_barCryOuterR;
      const EclCM rib1ri = rib1ro - rib1h;
      const EclRad ribdp1 = rib1w / ((rib1ro + rib1ri) / 2.0);

      double rib1_Z[2] = {rib1z1, rib1z2};
      double rib1_I[2] = {rib1ri, rib1ri};
      double rib1_O[2] = {rib1ro, rib1ro};

      G4Polycone* rib1Solid = new G4Polycone("eclBarrelSupportRib1Solid", 0.0, ribdp1, 2, rib1_Z, rib1_I, rib1_O);
      G4LogicalVolume* rib1Logical = new G4LogicalVolume(rib1Solid, medIron, "eclBarrelSupportRib1Logical", 0, 0, 0);

      const EclCM rib2h = 1.6 * CLHEP::cm;
      const EclCM rib2z1 = k_barSupBackFarZ + 13.5 * CLHEP::cm;
      const EclCM rib2z2 = k_barSupForwFarZ - 12.0 * CLHEP::cm;
      const EclCM rib2ro = k_barCryOuterR - 0.21 * CLHEP::cm;
      const EclCM rib2ri = rib2ro - rib2h;
      const EclRad ribdp2 = dPhi - ribdp1 * 2;

      double rib2_Z[2] = {rib2z1, rib2z2};
      double rib2_I[2] = {rib2ri, rib2ri};
      double rib2_O[2] = {rib2ro, rib2ro};

      G4Polycone* rib2Solid = new G4Polycone("eclBarrelSupportRib2Solid", 0.0, ribdp2, 2, rib2_Z, rib2_I, rib2_O);
      G4LogicalVolume* rib2Logical = new G4LogicalVolume(rib2Solid, medIron, "eclBarrelSupportRib2Logical", 0, 0, 0);

      const EclCM gap4 = 0.1 * CLHEP::cm;
      const EclCM rib4w = 0.3 * CLHEP::cm;
      const EclCM rib4h = (rib1ri - k_barCryOuterQ) - k_barCryMargin - gap4;
      const EclCM rib4z1 = k_barSupBackFarZ + 10.5 * CLHEP::cm + 1.8 * CLHEP::cm;
      const EclCM rib4z2 = k_barSupForwFarZ - 9.0 * CLHEP::cm - 1.0 * CLHEP::cm;
      //const EclCM rib4ro = rib1ri; // LEP: move inward by 0.821mm to avoid eclBarrelCrystalSector--eclBarrelOuterSector boundary
      const EclCM rib4ro = rib1ri - 0.821 * CLHEP::mm;
      const EclCM rib4ri = rib4ro - rib4h;
      const EclRad ribdp4 = rib4w / ((rib4ro + rib4ri) / 2.0);

      double rib4_Z[2] = {rib4z1, rib4z2};
      double rib4_I[2] = {rib4ri, rib4ri};
      double rib4_O[2] = {rib4ro, rib4ro};

      G4Polycone* rib4Solid = new G4Polycone("eclBarrelSupportRib4Solid", 0.0, ribdp4, 2, rib4_Z, rib4_I, rib4_O);
      G4LogicalVolume* rib4Logical = new G4LogicalVolume(rib4Solid, medIron, "eclBarrelSupportRib4Logical", 0, 0, 0);

      const EclCM t5 = 0.05 * CLHEP::cm;
      const EclCM l5 = (5.0 + 329.0) * CLHEP::cm;
      const EclCM rib5z1 = rib1z1;
      const EclCM rib5z2 = rib5z1 + l5;
      const EclCM ribr52 = 1.2 / 2 * CLHEP::cm;
      const EclCM ribr51 = ribr52 - t5;
      //EclCM ribr5 = rib1ro - 1.25 * CLHEP::cm - 1.58 * CLHEP::cm; // LEP: move inward by 6.5mm to avoid eclBarrelCrystalSector--eclBarrelOuterSector boundary
      EclCM ribr5 = rib1ro - 1.25 * CLHEP::cm - 1.58 * CLHEP::cm - 6.5 * CLHEP::mm;
      const EclRad ribdp5 = atan2(ribr52, ribr5);
      ribr5 = sqrt(ribr5 * ribr5 + ribr52 * ribr52);

      double rib5_Z[2] = {rib5z1, rib5z2};
      double rib5_I[2] = {ribr51, ribr51};
      double rib5_O[2] = {ribr52, ribr52};

      G4Polycone* rib5Solid = new G4Polycone("eclBarrelSupportRib5Solid", 0, TWOPI, 2, rib5_Z, rib5_I, rib5_O);
      G4LogicalVolume* rib5Logical = new G4LogicalVolume(rib5Solid, medIron, "eclBarrelSupportRib5Logical", 0, 0, 0);

      // The above parts will be placed later (within the inner or outer sector).

      // Make the barrel support container - solid iron.  Most of its interior will be filled
      // with two radially-split air-filled Polycones: eclBarrelCrystalContainer (for crystals etc)
      // and eclBarrelOuter (for the just-made support pieces 4 and 5), leaving just a thin
      // envelope of iron.

      double barrel_Z[6] = { k_barConBackZOut, k_barCryBackZOut, k_barConBackZ3, k_barConForwZ3, k_barCryForwZOut, k_barConForwZOut };
      double barrel_I[6] = { k_barConBackR1, k_barConBackR2, k_barConInnerR, k_barConInnerR, k_barConForwR2, k_barConForwR1 };
      double barrel_O[6] = { k_barCryOuterR, k_barCryOuterR, k_barCryOuterR, k_barCryOuterR, k_barCryOuterR, k_barCryOuterR };

      G4Polycone* barrelSolid =
        new G4Polycone("eclBarrelSolid", 0, TWOPI, 6, barrel_Z, barrel_I, barrel_O);
      G4LogicalVolume* barrelLogical = new G4LogicalVolume(barrelSolid, medIron, "eclBarrelLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, barrelLogical, "eclBarrelPhysical", eclLogical, false, 0, overlaps);

      // The outer part of the ECL central barrel is air and will be filled with G4PVReplica
      // sectors, each containing iron support bars of types 1 and 2 that had been made earlier.

      double supportContainer_I[2];
      double supportContainer_O[2];
      double supportContainer_Z[2];
      supportContainer_Z[0] = k_barCryBackZOut;
      supportContainer_O[0] = k_barCryOuterR;
      supportContainer_I[0] = rib1ri;
      supportContainer_Z[1] = k_barCryForwZOut;
      supportContainer_O[1] = k_barCryOuterR;
      supportContainer_I[1] = rib1ri;

      G4Polycone* supportContainerSolid = new G4Polycone("eclBarrelSupportContainerSolid", 0, TWOPI, 2,
                                                         supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportContainerLogical = new G4LogicalVolume(supportContainerSolid, medAir, "eclBarrelSupportContainerLogical");
      new G4PVPlacement(0, noMove, supportContainerLogical, "eclBarrelSupportContainerPhysical", barrelLogical, false, 0, overlaps);
      G4Polycone* supportSectorSolid = new G4Polycone("eclBarrelSupportSectorSolid", -0.5 * dPhi, dPhi, 2,
                                                      supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportSectorLogical = new G4LogicalVolume(supportSectorSolid, medAir, "eclBarrelSupportSectorLogical");
      new G4PVReplica("eclBarrelSupportSectorPhysical", supportSectorLogical, supportContainerLogical,
                      kPhi, nSectors, dPhi, 0.5 * dPhi + outerSectorOffset);

      // "Radial" boundaries of the ECL barrel's crystal-bearing sectors are not quite radial,
      // by design, so we cannot use G4PVReplicas of a G4Polycone for each sector.
      // Instead, we use an azimuthally-tilted and displaced G4Polycone for each sector.
      // This should be a G4Parameterised sector but this cannot be converted into a TGeo
      // volume by VGM so, instead, we place 72 copies of the tilted displaced sector.
      // One such tilted "crystal" sector contains two rows of crystals and one
      // aluminum PhiFin (at the high-phi side). This fin defines the sector's high-phi side.
      // The sector is the union of two identical adjacent G4Polycone half-sectors, where the
      // high-phi half-sector contains crystals and the PhiFin and the low-phi half-sector
      // contains only crystals.
      //
      // The G4Polycone container of these tilted sectors is initially air but will be almost
      // (but not quite) entirely filled will the tilted sectors.

      double crystalContainer_Z[8] = {
        k_barCryBackZOut, k_barConBackZ3, k_barConBackZIn, k_barConBackZIn,
        k_barConForwZIn, k_barConForwZIn, k_barConForwZ3, k_barCryForwZOut
      };
      double crystalContainer_I[8] = {
        k_barConBackR3, k_barConBackR4, k_barConBackR5, k_barCryInnerR,
        k_barCryInnerR, k_barConForwR5, k_barConForwR4, k_barConForwR3
      };
      double crystalContainer_O[8] = {
        rib1ri, rib1ri, rib1ri, rib1ri,
        rib1ri, rib1ri, rib1ri, rib1ri
      };

      G4Polycone* crystalContainerSolid = new G4Polycone("eclBarrelCrystalContainerSolid", 0, TWOPI, 8,
                                                         crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4LogicalVolume* crystalContainerLogical = new G4LogicalVolume(crystalContainerSolid, medAir, "eclBarrelCrystalContainerLogical");
      new G4PVPlacement(0, noMove, crystalContainerLogical, "eclBarrelCrystalContainerPhysical", barrelLogical, false, 0, overlaps);

      // geometry of barrel phi fin determines that of one barrel sector:
      // inner and outer radii differ slightly from the values for crystalContainer

      double brforwangle = 33.003 * CLHEP::deg; // same as Belle I
      double brbackangle = 52.879 * CLHEP::deg;

      double brphifina = 298.14 * CLHEP::cm / 2; // shorter base
      double brphifinL = 19.49 * CLHEP::cm / 2; // height

      double f_xForw = brphifinL / tan(brforwangle);
      double f_xBack = brphifinL / tan(brbackangle);
      double f_dx = (f_xForw - f_xBack) / 2;

      double brphifinA = brphifina + f_xBack + f_xForw;
      double brphifinh = brphifinthickness / 2; // thickness
      double brphifintheta = atan(f_dx / brphifinL);

      double brphifin_shorterbaseleft = 100.08 * CLHEP::cm;
      double cposix = 140.594699 * CLHEP::cm;
      double cposiy = 6.478866 * CLHEP::cm;
      double brphitiltphi = 3.7579644 * CLHEP::deg;
      double brphirmoveout = - 3.50 * CLHEP::cm;
      double brphizmove = 0.0 * CLHEP::cm;
      double brphimoveperp = 0.0 * CLHEP::cm;

      // Start with point on high-phi side of the phi fin and unit vectors parallel to and perp to this fin.
      G4ThreeVector u1(cos(brphitiltphi), sin(brphitiltphi), 0.0);
      G4ThreeVector v1(-sin(brphitiltphi), cos(brphitiltphi), 0.0);
      G4ThreeVector u2(cos(brphitiltphi - 0.5 * dPhi), sin(brphitiltphi - 0.5 * dPhi), 0.0);
      G4ThreeVector v2(-sin(brphitiltphi - 0.5 * dPhi), cos(brphitiltphi - 0.5 * dPhi), 0.0);
      G4ThreeVector u3(cos(brphitiltphi - dPhi), sin(brphitiltphi - dPhi), 0.0);
      G4ThreeVector v3(-sin(brphitiltphi - dPhi), cos(brphitiltphi - dPhi), 0.0);
      G4ThreeVector Q0(cposix, cposiy, 0.0); // in the centre of the PhiFin
      Q0 = Q0 + brphirmoveout * u1 + (brphimoveperp + brphifinh + 0.02 * CLHEP::mm) * v1; // on high-phi side of PhiFin
      // Construct the (displaced) origin of the tilted half-sector
      G4ThreeVector Q1 = Q0 + u1 * (Q0 * (v1 - v2) / sin(0.5 * dPhi));
      // Get the inner and outer radii of the tilted half-sector (relative to Q1)
      double wi = Q1 * u1;
      double ri = sqrt(wi * wi + (k_barCryInnerR * k_barCryInnerR - Q1.mag2())) - wi;
      double wo = Q1 * u2;
      double ro = sqrt(wo * wo + (rib1ri * rib1ri - Q1.mag2())) - wo - 0.005 * CLHEP::mm;
      // Get the shift of one half-sector relative to the other in the G4UnionSolid's reference frame
      double dr = sqrt(wo * wo + (k_barCryInnerR * k_barCryInnerR - Q1.mag2())) - wo - ri;
      G4ThreeVector halfSectorShift(dr * cos(-0.5 * dPhi), dr * sin(-0.5 * dPhi), 0.0);
      G4RotationMatrix halfSectorRotate(-0.5 * dPhi, 0.0, 0.0);
      // Construct the (displaced) origin of the tilted full sector
      G4ThreeVector Q2 = Q0 + u1 * (Q0 * (v1 - v3) / sin(dPhi));

      crystalContainer_I[0] += 0.35 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      crystalContainer_I[1] += 0.35 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      crystalContainer_I[2] += 0.35 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      crystalContainer_I[3] = crystalContainer_I[4] = ri;
      crystalContainer_I[5] += 0.34 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      crystalContainer_I[6] += 0.34 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      crystalContainer_I[7] += 0.34 * CLHEP::mm; // to avoid corner-clipping of tilted sector with its container
      for (int j = 0; j < 8; ++j) crystalContainer_O[j] = ro;
      G4Transform3D TrSector = G4RotateZ3D(brphitiltphi - 0.5 * dPhi) * G4Translate3D(Q2);
      G4Transform3D TrSectorInverse = G4Translate3D(-Q2) * G4RotateZ3D(0.5 * dPhi - brphitiltphi);
      G4Polycone* crystalHalfSectorSolid = new G4Polycone("eclBarrelCrystalHalfSectorSolid", 0, 0.5 * dPhi, 8,
                                                          crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4UnionSolid* crystalSectorSolid = new G4UnionSolid("eclBarrelCrystalSectorSolid",
                                                          crystalHalfSectorSolid, crystalHalfSectorSolid,
                                                          &halfSectorRotate, halfSectorShift);
      G4LogicalVolume* crystalSectorLogical = new G4LogicalVolume(crystalSectorSolid, medAir, "eclBarrelCrystalSectorLogical");
      // Cannot use G4PVParameterised(): incompatible with TGeo and VGM. So make 72 placed copies instead.
      // BarrelSectorParameterisation* barrelSectorParameterisation = new BarrelSectorParameterisation(TrSector, nSectors);
      // new G4PVParameterised("eclBarrelCrystalSectorPhysical", crystalSectorLogical, crystalContainerLogical, kUndefined, nSectors, barrelSectorParameterisation, overlaps);
      for (int iSector = 0; iSector < nSectors; ++iSector) {
        new G4PVPlacement(G4RotateZ3D(dPhi * iSector) * TrSector, crystalSectorLogical, "eclBarrelCrystalSectorPhysical",
                          crystalContainerLogical, false, iSector, overlaps);
      }

      // Place the phi fin (union of 3 solids) within the inner sector

      G4Trap* phifinSolid1 = new G4Trap("solidEclphifinSolid1",
                                        brphifinL, brphifintheta, M_PI,
                                        brphifinh, brphifinA, brphifinA, 0.0,
                                        brphifinh, brphifina, brphifina, 0.0);

      double brphifinL2 = 9.46 * CLHEP::cm / 2; // height
      double f_xForw2 = 0.0;
      double f_xBack2 = brphifinL2 / tan(brbackangle);
      double f_dx2 = (f_xForw2 - f_xBack2) / 2;
      double brphifinA2 = brphifinA + f_xForw2 + f_xBack2;
      double brphifintheta2 = atan(f_dx2 / brphifinL2);

      G4Trap* phifinSolid2 = new G4Trap("eclBarrelPhiFinSolid2",
                                        brphifinL2, brphifintheta2, M_PI,
                                        brphifinh, brphifinA2, brphifinA2, 0.0,
                                        brphifinh, brphifinA, brphifinA, 0.0);

      double brphifinL3 = 2.94 * CLHEP::cm / 2; // height

      G4Trap* phifinSolid3 = new G4Trap("eclBarrelPhiFinSolid3",
                                        brphifinL3, 0.0, M_PI,
                                        brphifinh, brphifinA2, brphifinA2, 0.0,
                                        brphifinh, brphifinA2, brphifinA2, 0.0);

      //LEP: x translation is simplified for shape1-shape2 and corrected (by ~2 cm) for shape12-shape3
      //G4Transform3D shape2position = G4Translate3D((brphifinA + f_dx2) - (brphifina + f_dx) - f_xBack * 2, 0.0, -(brphifinL + brphifinL2));   // move shape 2
      //G4Transform3D shape3position = G4Translate3D(brphifinA2 - (brphifina + f_dx) - f_xBack * 2 - f_xBack2 * 2, 0.0, -(brphifinL + brphifinL2 * 2 + brphifinL3)); // move shape 3
      G4Transform3D shape2position = G4Translate3D(f_dx + f_dx2, 0.0, -(brphifinL + brphifinL2));   // move shape 2
      G4Transform3D shape3position = G4Translate3D((f_dx * brphifinL2 + f_dx2 * brphifinL) / (brphifinL + brphifinL2), 0.0,
                                                   -(brphifinL + brphifinL2 * 2 + brphifinL3)); // move shape 3

      G4UnionSolid* phifinSolid12 = new G4UnionSolid("eclBarrelPhiFinSolid12", phifinSolid1, phifinSolid2, shape2position);
      G4UnionSolid* phiFinSolid = new G4UnionSolid("eclBarrelPhiFinSolid", phifinSolid12, phifinSolid3, shape3position);
      G4LogicalVolume* phiFinLogical = new G4LogicalVolume(phiFinSolid, medAl, "eclBarrelPhiFinLogical", 0, 0, 0);

      G4Transform3D r00 = G4RotateY3D(-0.5 * M_PI);
      G4Transform3D tilt_z = G4RotateY3D(0.0);
      G4Transform3D central = G4Translate3D(0.0, 0.0, brphifina - brphifin_shorterbaseleft + f_dx);
      G4Transform3D tilt_psi = G4RotateX3D(0.0);
      G4Transform3D tilt_phi = G4RotateZ3D(brphitiltphi);
      G4Transform3D cposition = G4Translate3D(cposix + brphirmoveout * cos(brphitiltphi) - brphimoveperp * sin(brphitiltphi),
                                              cposiy + brphirmoveout * sin(brphitiltphi) + brphimoveperp * cos(brphitiltphi),
                                              brphizmove);
      G4Transform3D Tr = cposition * tilt_phi * tilt_psi * central * tilt_z * r00;
      new G4PVPlacement(TrSectorInverse * Tr, phiFinLogical, "eclBarrelPhiFinPhysical", crystalSectorLogical, false, 0, overlaps);

      // add barrel fin parameters and previous crystal parameters/////////
      double fink_BLL = 30 * CLHEP::cm;
      double fink_Ba = 5.387 * CLHEP::cm;
      double fink_Bb = 5.5868 * CLHEP::cm;
      double fink_BA = 6.0997 * CLHEP::cm;
      double fink_Bh = 5.4504 * CLHEP::cm;
      double fink_z_TILTED = 33.667039 * CLHEP::deg;
      double fink_phi_TILTED = 1.149806 * CLHEP::deg;
      double fink_perpC = 136.08246 * CLHEP::cm;
      double fink_phiC = 1.344548 * CLHEP::cm;
      double fink_zC = 210.1509 * CLHEP::cm;
      double fink_phi_init = 0.0094779 * CLHEP::deg; // add correction

      double  prevk_Ba = 5.387 * CLHEP::cm;
      double  prevk_BA =  6.0997 * CLHEP::cm;
      double  prevk_Bh =  5.4504 * CLHEP::cm;
      double  prevk_BH = 6.0997 * CLHEP::cm;;
      double  prevk_z_TILTED =  33.667039 * CLHEP::deg;
      double  prevk_phi_TILTED = 1.149806 * CLHEP::deg;
      double  prevk_perpC = 136.08246 * CLHEP::cm;
      double  prevk_phiC = 1.344548 * CLHEP::cm;
      double  prevk_zC = 210.1509 * CLHEP::cm;;
      double  prevk_phi_init =  0.0094779 * CLHEP::deg;

      // Place each crystal within the sector: two rows of crystals (29 forward and 17 backward per row)

      int nCrystal = content.getNumberNodes("BarrelCrystals/BarrelCrystal");

      for (int iCry = 1; iCry <= nCrystal; ++iCry) {

        GearDir layerContent(content);
        layerContent.append((format("/BarrelCrystals/BarrelCrystal[%1%]/") % iCry).str());

        double k_BLL = layerContent.getLength("K_BLL") * CLHEP::cm;
        double k_Ba = layerContent.getLength("K_Ba")   * CLHEP::cm;
        double k_Bb = layerContent.getLength("K_Bb")   * CLHEP::cm;
        double k_Bh = layerContent.getLength("K_Bh")   * CLHEP::cm;
        double k_BA = layerContent.getLength("K_BA")   * CLHEP::cm;
        double k_BH = layerContent.getLength("K_BH")   * CLHEP::cm;
        double k_z_TILTED = layerContent.getAngle("K_z_TILTED");
        double k_phi_TILTED = layerContent.getAngle("K_phi_TILTED");
        double k_perpC = layerContent.getLength("K_perpC") * CLHEP::cm;
        double k_phiC = layerContent.getAngle("K_phiC");
        double k_zC = layerContent.getLength("K_zC") * CLHEP::cm;
        double k_phi_init = layerContent.getAngle("K_phi_init"); // add correction

        double cDx1 = k_Ba / 2;
        double cDx2 = k_Bb / 2;
        double cDy1 = k_Bh / 2;
        double cDx3 = k_BA / 2;
        double cDy2 = k_BH / 2;
        double cDz = k_BLL / 2;
        double cDx4 = (cDx3 * cDy1 + (cDx2 - cDx1) * cDy2) / cDy1; // replaced by coplanarity

        // add barrel foil dimensions ///////////////////////
        double trapangle1 = atan(2 * cDy1 / (cDx2 - cDx1)); // the smaller angle of the trap
        double trapangle2 = atan(2 * cDy2 / (cDx4 - cDx3));
        double foilcDz = cDz + foilthickness;
        double foilcDx1 = cDx1 + foilthickness * tan(trapangle1 / 2);
        double foilcDx2 = cDx2 + foilthickness / tan(trapangle1 / 2);
        double foilcDy1 = cDy1 + foilthickness;
        double foilcDx3 = cDx3 + foilthickness * tan(trapangle2 / 2);
        double foilcDy2 = cDy2 + foilthickness;
        double foilcDx4 = foilcDx3 + (foilcDx2 - foilcDx1) * foilcDy2 / foilcDy1;

        G4Transform3D r00 = G4RotateZ3D((iCry >= 30 ? -0.5 : 0.5) * M_PI);
        G4Transform3D phi_init = G4RotateZ3D(k_phi_init);
        G4Transform3D tilt_z = G4RotateY3D(k_z_TILTED);
        G4Transform3D tilt_phi = G4RotateZ3D(k_phi_TILTED);
        G4Transform3D position = G4Translate3D(k_perpC, 0, k_zC);  // Move over to the left...
        G4Transform3D pos_phi = G4RotateZ3D(k_phiC);
        G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * r00 * phi_init;

        //  add barrel foil, initially as a solid trapezoid //////////////

        G4Trap* foilSolid = new G4Trap((format("eclBarrelFoilSolid_%1%") % iCry).str(),
                                       foilcDz, 0.0, 0.0,
                                       foilcDy1, foilcDx2, foilcDx1, 0.0,
                                       foilcDy2, foilcDx4, foilcDx3, 0.0);
        G4LogicalVolume* foilLogical = new G4LogicalVolume(foilSolid, medAlTeflon,
                                                           (format("eclBarrelFoilLogical_%1%") % iCry).str(), 0, 0, 0);

        //  place the crystal inside the foil, occupying most of the interior volume

        G4Trap* crystalSolid = new G4Trap((format("eclBarrelCrystalSolid_%1%") % iCry).str(),
                                          cDz, 0.0, 0.0,
                                          cDy1, cDx2, cDx1, 0.0,
                                          cDy2, cDx4, cDx3, 0.0);
        G4LogicalVolume* crystalLogical = new G4LogicalVolume(crystalSolid, medCsI,
                                                              (format("eclBarrelCrystalLogical_%1%") % iCry).str(), 0, 0, 0);
        crystalLogical->SetSensitiveDetector(m_sensitive);
        new G4PVPlacement(0, noMove, crystalLogical, (format("eclBarrelCrystalPhysical_%1%") % iCry).str(),
                          foilLogical, false, 72 + 9 * (iCry - 1), overlaps);

        G4Transform3D BrR = TrSectorInverse * G4RotateZ3D(0.0) * Tr;
        G4Transform3D BrRR = TrSectorInverse * G4RotateZ3D(-2.488555 * CLHEP::deg) * Tr; // replace 2.494688
        new G4PVPlacement(BrR,  foilLogical, (format("eclBarrelFoilPhysical_0_%1%") % iCry).str(), crystalSectorLogical, false, 0,
                          overlaps);
        new G4PVPlacement(BrRR, foilLogical, (format("eclBarrelFoilPhysical_1_%1%") % iCry).str(), crystalSectorLogical, false, 1,
                          overlaps);

        if (isBeamBkgStudy) {
          int diodeId = (iCry - 1) * k_barNPhiSegs + 1152;
          G4Transform3D diodePos = G4Translate3D(0, 0, (k_BLL + diodeHeight) / 2 + 0.1);  // Move over to the left...
          G4LogicalVolume* diodeLogical = new G4LogicalVolume(diodeSolid, medSi, (format("eclBarrelDiodeLogical_%1%") % iCry).str(), 0, 0, 0);
          m_bkgsensitive.push_back(new BkgSensitiveDetector("ECL", diodeId));
          diodeLogical->SetSensitiveDetector(m_bkgsensitive.back());
          new G4PVPlacement(BrR * diodePos,  diodeLogical, (format("eclBarrelDiodePhysical_%1%") % diodeId).str(),
                            crystalSectorLogical, false, diodeId, overlaps);
          diodeId--;
          new G4PVPlacement(BrRR * diodePos, diodeLogical, (format("eclBarrelDiodePhysical_%1%") % diodeId).str(),
                            crystalSectorLogical, false, diodeId, overlaps);
        }

        // add barrel theta fin //////////////////////////////////////////////////////////////////////////

        if (iCry % 4 == 1) { // store the parameters of crystals before the fin (previous)
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

        if (iCry % 4 == 2 && (iCry != 2 && iCry != 46)) { // barrel fin between every  4 crystals (iCry%4==2 means after the fin)

          double smalldev = 0.01;
          if (iCry < 29) { // choose the lower one
            fink_Ba = k_Ba * (1 - smalldev);
            fink_Bb = k_Ba * (1 - smalldev);
            fink_BA = k_BA * (1 - smalldev); // / sin((prevk_z_TILTED + k_z_TILTED) / 2);
          } else {
            fink_Ba = prevk_Ba * (1 - smalldev);
            fink_Bb = prevk_Ba * (1 - smalldev);
            fink_BA = prevk_BA * (1 - smalldev); // / sin((prevk_z_TILTED + k_z_TILTED) / 2);
          }

          fink_Bh = brthetafinthickness;
          fink_z_TILTED = ((prevk_z_TILTED + atan((prevk_BH - prevk_Bh) / k_BLL / 2)) +
                           (k_z_TILTED - atan((k_BH - k_Bh) / k_BLL / 2))) / 2; // average angle of 2 crystal sides
          fink_BLL = k_BLL; // (30 cm)
          fink_phi_TILTED = (prevk_phi_TILTED + k_phi_TILTED) / 2; // average
          // average and move more for fin longer than crystal
          fink_perpC = (prevk_perpC + k_perpC +
                        (fink_BLL - k_BLL - smalldev * fabs(prevk_zC + k_zC)) * sin(fink_z_TILTED)) / 2;
          fink_phiC = (prevk_phiC + k_phiC) / 2; // average
          // average and move more for fin longer than crystal
          fink_zC = (prevk_zC + k_zC  - (prevk_BH - prevk_Bh) / 2 / sin(prevk_z_TILTED) +
                     (k_BH - k_Bh) / 2 / sin(k_z_TILTED) +
                     (fink_BLL - k_BLL - smalldev * fabs(prevk_zC + k_zC)) * cos(fink_z_TILTED)) / 2;
          fink_phi_init = (prevk_phi_init + k_phi_init) / 2; // average

          double fincDx1 = fink_Ba / 2;
          double fincDx2 = fink_Bb / 2;
          double fincDy = fink_Bh / 2;
          double fincDx3 = fink_BA / 2;
          double fincDz = fink_BLL / 2;
          double fincDx4 = fincDx3 + (fincDx2 - fincDx1); // coplanarity

          G4Transform3D phi_init = G4RotateZ3D(fink_phi_init);
          G4Transform3D tilt_z = G4RotateY3D(fink_z_TILTED);
          G4Transform3D tilt_phi = G4RotateZ3D(fink_phi_TILTED);
          G4Transform3D position = G4Translate3D(fink_perpC, 0, fink_zC);  // Move over to the left...
          G4Transform3D pos_phi = G4RotateZ3D(fink_phiC);
          G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * r00 * phi_init;

          G4Trap* finSolid = new G4Trap((format("eclBarrelFinSolid_%1%") % iCry).str(),
                                        fincDz, 0.0, 0.0,
                                        fincDy, fincDx2, fincDx1, 0.0,
                                        fincDy, fincDx4, fincDx3, 0.0);
          G4LogicalVolume* finLogical = new G4LogicalVolume(finSolid, medAl, (format("eclBarrelFinLogical_%1%") % iCry).str(), 0, 0, 0);
          G4Transform3D BrR = G4RotateZ3D(0.0) * Tr;
          G4Transform3D BrRR = G4RotateZ3D(-2.488555 * CLHEP::deg) * Tr; // replace 2.494688
          new G4PVPlacement(TrSectorInverse * BrR,  finLogical, (format("eclBarrelFinPhysicalA_%1%") % iCry).str(),
                            crystalSectorLogical, false, 0, overlaps);
          new G4PVPlacement(TrSectorInverse * BrRR, finLogical, (format("eclBarrelFinPhysicalB_%1%") % iCry).str(),
                            crystalSectorLogical, false, 1, overlaps);

        } // end if barrel fin between every 4 crystals

      }//iCry

      // place the iron ribs/pipes at the outer radius of the barrel

      double iSectorsRot = - 0.5 * dPhi + outerSectorOffset;
      // LEP: to place 1 and 2 properly in the G4Replica sector, omit iSectorsRot here
      // LEP: ... but retain iSectorsRot for 4 and 5 for the G4PVParameterised sector
      G4Transform3D rot11 = G4RotateZ3D(-0.5 * dPhi);
      G4Transform3D rot12 = G4RotateZ3D(-ribdp1 + dPhi - 0.5 * dPhi);
      G4Transform3D rot2 = G4RotateZ3D(-ribdp2 / 2.0 + dPhi / 2 - 0.5 * dPhi);

      G4Transform3D rot41 = G4RotateZ3D(-ribdp4 + ribdp1 + iSectorsRot);
      G4Transform3D rot42 = G4RotateZ3D(-ribdp1 + dPhi + iSectorsRot);

      G4Transform3D X5 = G4Translate3D(ribr5, 0.0, 0.0);
      G4Transform3D rot51 = G4RotateZ3D(ribdp5 + ribdp1 + iSectorsRot);
      G4Transform3D rot52 = G4RotateZ3D(-ribdp5 - ribdp1 + dPhi + iSectorsRot);
      G4Transform3D tr51 = rot51 * X5;
      G4Transform3D tr52 = rot52 * X5;

      new G4PVPlacement(rot11, rib1Logical, "eclBarrelSupport1Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(rot12, rib1Logical, "eclBarrelSupport1Physical", supportSectorLogical, false, 1, overlaps);
      new G4PVPlacement(rot2, rib2Logical, "eclBarrelSupport2Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(TrSectorInverse * rot41, rib4Logical, "eclBarrelSupport4Physical", crystalSectorLogical, false, 0, overlaps);
      new G4PVPlacement(TrSectorInverse * rot42, rib4Logical, "eclBarrelSupport4Physical", crystalSectorLogical, false, 1, overlaps);
      new G4PVPlacement(TrSectorInverse * tr51, rib5Logical, "eclBarrelSupport5Physical", crystalSectorLogical, false, 0, overlaps);
      new G4PVPlacement(TrSectorInverse * tr52, rib5Logical, "eclBarrelSupport5Physical", crystalSectorLogical, false, 1, overlaps);

      // -------------------- Start Barrel Outer Skin Spec ---------------------

      double skin_Z[6] = { k_barSupBackFarZ, k_barConBackZOut, k_barConBackZOut, k_barConForwZOut, k_barConForwZOut, k_barSupForwFarZ };
      double skin_I[6] =  {k_barConBackR1, k_barConBackR1, k_barCryOuterR, k_barCryOuterR, k_barConForwR1, k_barSupForwFarIR};
      double skin_O[6] = {k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR, k_barConOuterR};

      G4Polycone* skinSolid = new G4Polycone("eclBarrelSkinSolid", 0.0, TWOPI, 6, skin_Z, skin_I, skin_O);
      G4LogicalVolume* skinLogical = new G4LogicalVolume(skinSolid, medIron, "eclBarrelSkinLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, skinLogical, "eclBarrelSkinPhysical", eclLogical, false, 0, overlaps);

      const double BASoffset = 2.796 * CLHEP::deg;
      const double BAScut = 0.5 * CLHEP::cm; // due to the PCON overlap

      double L1_Z[4] = {k_l1z1, k_l1z2, k_l1z2, k_l1z3};
      double L1_I[4] = {k_l1r1, k_l1r2, k_l1r3, k_l1r3};
      double L1_O[4] = {k_l1r4 - BAScut, k_l1r4 - BAScut, k_l1r4 - BAScut, k_l1r4 - BAScut};

      G4Polycone* L1ContainerSolid = new G4Polycone("eclBarrelSupportForwEllContainerSolid", 0.0, TWOPI, 4, L1_Z, L1_I, L1_O);
      G4LogicalVolume* L1ContainerLogical = new G4LogicalVolume(L1ContainerSolid, medAir,
                                                                "eclBarrelSupportForwEllContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, L1ContainerLogical, "eclBarrelSupportForwEllContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* L1SectorSolid = new G4Polycone("eclBarrelSupportForwEllSectorSolid", -0.5 * dPhi, dPhi, 4, L1_Z, L1_I, L1_O);
      G4LogicalVolume* L1SectorLogical = new G4LogicalVolume(L1SectorSolid, medAir, "eclBarrelSupportForwEllSectorLogical", 0, 0, 0);
      new G4PVReplica("eclBarrelSupportForwLSectorPhysical", L1SectorLogical, L1ContainerLogical,
                      kPhi, nSectors, dPhi, -dPhi + BASoffset);

      G4Polycone* L1Solid = new G4Polycone("eclBarrelSupportForwEllSolid", -0.5 * k_l1ang, k_l1ang, 4, L1_Z, L1_I, L1_O);
      G4LogicalVolume* L1Logical = new G4LogicalVolume(L1Solid, medIron, "eclBarrelSupportForwEllLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, L1Logical, "eclBarrelSupportFwdLPhysical", L1SectorLogical, false, 0, overlaps);

      double L2_Z[4] = {k_l2z1, k_l2z2, k_l2z2, k_l2z3};
      double L2_I[4] = {k_l2r2, k_l2r2, k_l2r1, k_l2r1};
      double L2_O[4] = {k_l2r3 - BAScut, k_l2r3 - BAScut, k_l2r3 - BAScut, k_l2r3 - BAScut};

      G4Polycone* L2ContainerSolid = new G4Polycone("eclBarrelSupportBackEllContainerSolid", 0.0, TWOPI, 4, L2_Z, L2_I, L2_O);
      G4LogicalVolume* L2ContainerLogical = new G4LogicalVolume(L2ContainerSolid, medAir,
                                                                "eclBarrelSupportBackEllContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, L2ContainerLogical, "eclBarrelSupportBackEllContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* L2SectorSolid = new G4Polycone("eclBarrelSupportBackEllSectorSolid", -0.5 * dPhi, dPhi, 4, L2_Z, L2_I, L2_O);
      G4LogicalVolume* L2SectorLogical = new G4LogicalVolume(L2SectorSolid, medAir, "eclBarrelSupportBackEllSectorLogical", 0, 0, 0);
      new G4PVReplica("eclBarrelSupportBackEllSectorPhysical", L2SectorLogical, L2ContainerLogical,
                      kPhi, nSectors, dPhi, -dPhi + BASoffset);

      G4Polycone* L2Solid = new G4Polycone("eclBarrelSupportBackEllSolid", -0.5 * k_l2ang, k_l2ang, 4, L2_Z, L2_I, L2_O);
      G4LogicalVolume* L2Logical = new G4LogicalVolume(L2Solid, medIron, "eclBarrelSupportBackEllLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, L2Logical, "eclBarrelSupportBackEllPhysical", L2SectorLogical, false, 0, overlaps);

    }//makeBarrel()

    void GeoECLCreator::makeForwardEndcap(const GearDir& content, G4LogicalVolume* eclLogical)
    {

      G4Material* medAir = geometry::Materials::get("Air");
      G4Material* medIron = geometry::Materials::get("G4_Fe");
      G4Material* medCsI = geometry::Materials::get(content.getString("CsI"));
      G4Material* medSi = geometry::Materials::get(content.getString("Si"));
      G4Material* medAlTeflon = geometry::Materials::get(content.getString("AlTeflon"));
      G4Material* medAlTeflon_thin = geometry::Materials::get(content.getString("AlTeflon_thin"));
      G4Material* medAlTeflon_thinpenta = geometry::Materials::get(content.getString("AlTeflon_thinpenta"));

      G4ThreeVector noMove(0.0, 0.0, 0.0);

      const double dPhi = TWOPI / k_forwNPhiSegs;

      // create a diode in case it is needed for background study
      double diodeWidth = content.getLength("k_diodewidth") * CLHEP::cm;
      double diodeLength = content.getLength("k_diodelength") * CLHEP::cm;
      double diodeHeight = content.getLength("k_diodeheight") * CLHEP::cm;
      G4Box* diodeSolid = new G4Box("diode", diodeWidth / 2, diodeLength / 2, diodeHeight / 2);

      double crystalContainer_I[6];
      double crystalContainer_O[6];
      double crystalContainer_Z[6];

      crystalContainer_Z[0] = k_forwConNearZ;
      crystalContainer_I[0] = k_forwConNearInnerR;
      crystalContainer_O[0] = k_barConInnerR;

      double slope_I = (k_forwConFarInnerR - k_forwConNearInnerR) / (k_forwConFarZ - k_forwConNearZ);
      crystalContainer_Z[1] = k_barConForwZ3;
      crystalContainer_I[1] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[1] - crystalContainer_Z[0]);
      crystalContainer_O[1] = k_barConInnerR;

      crystalContainer_Z[2] = k_barCryForwZOut;
      crystalContainer_I[2] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[2] - crystalContainer_Z[0]);
      crystalContainer_O[2] = k_barConForwR2;

      crystalContainer_Z[3] = k_barConForwZOut;
      crystalContainer_I[3] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[3] - crystalContainer_Z[0]);
      crystalContainer_O[3] = k_barConForwR1;

      crystalContainer_Z[4] = k_l1z1;
      crystalContainer_I[4] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[4] - crystalContainer_Z[0]);
      crystalContainer_O[4] = k_barSupForwFarIR;

      double slope_O = (k_l1r2 - k_barSupForwFarIR) / (k_l1z2 - k_l1z1);
      crystalContainer_Z[5] = k_forwConFarZ;
      crystalContainer_I[5] = k_forwConFarInnerR;
      crystalContainer_O[5] = crystalContainer_O[4] + slope_O * (crystalContainer_Z[5] - crystalContainer_Z[4]);

      double supportContainer_I[8];
      double supportContainer_O[8];
      double supportContainer_Z[8];

      supportContainer_Z[0] = k_forwConFarZ;
      supportContainer_I[0] = k_l2r3  - 42.0 * CLHEP::cm;
      supportContainer_O[0] = crystalContainer_O[4] + slope_O * (supportContainer_Z[0] - crystalContainer_Z[4]);

      supportContainer_Z[1] = k_l1z2;
      supportContainer_I[1] = k_l2r3  - 42.0 * CLHEP::cm;
      supportContainer_O[1] = k_l1r2;

      supportContainer_Z[2] = k_l1z2;
      supportContainer_I[2] = k_l2r3  - 42.0 * CLHEP::cm;
      supportContainer_O[2] = k_l1r3;

      supportContainer_Z[3] = k_forwConFarZ + 6.0 * CLHEP::cm;
      supportContainer_I[3] = k_l2r3  - 42.0 * CLHEP::cm;
      supportContainer_O[3] = k_l1r3;

      supportContainer_Z[4] = k_forwConFarZ + 6.0 * CLHEP::cm;
      supportContainer_I[4] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      supportContainer_O[4] = k_l1r3;

      supportContainer_Z[5] = k_l1z3;
      supportContainer_I[5] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      supportContainer_O[5] = k_l1r3;

      supportContainer_Z[6] = k_l1z3;
      supportContainer_I[6] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      supportContainer_O[6] = k_c1r1;

      supportContainer_Z[7] = k_c1z3;
      supportContainer_I[7] = k_l2r3  - 42.0 * CLHEP::cm + 3.0 * CLHEP::cm;
      supportContainer_O[7] = k_c1r1;

      G4Polycone* supportContainerSolid = new G4Polycone("eclForwardSupportContainerSolid", 0.0, TWOPI, 8,
                                                         supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportContainerLogical = new G4LogicalVolume(supportContainerSolid, medAir,
          "eclForwardSupportContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, supportContainerLogical, "eclForwardSupportContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* supportSectorSolid = new G4Polycone("eclForwardSupportSectorSolid", -dPhi, 2.0 * dPhi, 8,
                                                      supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportSectorLogical = new G4LogicalVolume(supportSectorSolid, medAir, "eclForwardSupportSectorLogical", 0, 0, 0);
      new G4PVReplica("eclForwardSupportSectorPhysical", supportSectorLogical, supportContainerLogical,
                      kPhi, k_forwNPhiSegs / 2, 2.0 * dPhi, 0.0);

      G4Polycone* crystalContainerSolid = new G4Polycone("eclForwardCrystalContainerSolid", 0.0, TWOPI, 6,
                                                         crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4LogicalVolume* crystalContainerLogical = new G4LogicalVolume(crystalContainerSolid, medAir,
          "eclForwardCrystalContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, crystalContainerLogical, "eclForwardCrystalContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* crystalSectorSolid = new G4Polycone("eclForwardCrystalSectorSolid", -0.5 * dPhi, dPhi, 6,
                                                      crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4LogicalVolume* crystalSectorLogical = new G4LogicalVolume(crystalSectorSolid, medAir, "eclForwardCrystalSectorLogical", 0, 0, 0);
      new G4PVReplica("eclForwardCrystalSectorPhysical", crystalSectorLogical, crystalContainerLogical,
                      kPhi, k_forwNPhiSegs, dPhi, 0.0);

      // Forward endcap support

      const double ECSoffset = 0.0;

      EclCM ECSw_1 = 13.0 * CLHEP::cm;
      EclCM ECSh_1 = 17.0 * CLHEP::cm;
      EclCM ECSt_1 = 4.0 * CLHEP::cm;
      EclCM ECSro_1 = 142.0 * CLHEP::cm;
      EclCM ECSri_1 = ECSro_1 - ECSh_1;
      EclRad ECSdp_1 = ECSw_1 / ((ECSro_1 + ECSri_1) / 2.0);
      double ECSz_11 = k_forwConFarZ;
      double ECSz_12 = ECSz_11 + ECSt_1;
      double support1_Z[2] = {ECSz_11, ECSz_12};
      double support1_I[2] = {ECSri_1, ECSri_1};
      double support1_O[2] = {ECSro_1, ECSro_1};

      G4Polycone* support1Solid = new G4Polycone("eclForwardSupport1Solid", 0.0, ECSdp_1, 2, support1_Z, support1_I, support1_O);
      G4LogicalVolume* support1Logical = new G4LogicalVolume(support1Solid, medIron, "eclForwardSupport1", 0, 0, 0);

      EclCM ECSw_2 = 6.0 * CLHEP::cm;
      EclCM ECSh_2 = 13.0 * CLHEP::cm;
      EclCM ECSt_2 = 13.7 * CLHEP::cm;
      EclCM ECSri_2 = ECSri_1 + 3.5 * CLHEP::cm;
      EclCM ECSro_2 = ECSri_2 + ECSh_2;
      EclRad ECSdp_2 = ECSw_2 / ((ECSro_2 + ECSri_2) / 2.0);
      double ECSz_21 = ECSz_12;
      double ECSz_22 = ECSz_21 + ECSt_2;
      double support2_Z[2] = {ECSz_21, ECSz_22};
      double support2_I[2] = {ECSri_2, ECSri_2};
      double support2_O[2] = {ECSro_2, ECSro_2};

      G4Polycone* support2Solid = new G4Polycone("eclForwardSupport2Solid", 0.0, ECSdp_2, 2, support2_Z, support2_I, support2_O);
      G4LogicalVolume* support2Logical = new G4LogicalVolume(support2Solid, medIron, "eclForwardSupport2Logical", 0, 0, 0);

      EclCM ECSw_3 = 14.0 * CLHEP::cm;
      EclCM ECSh_3 = 26.5 * CLHEP::cm;
      EclCM ECSt_3 = 7.0 * CLHEP::cm;
      EclCM ECSri_3 = ECSri_2;
      EclCM ECSro_3 = ECSri_3 + ECSh_3;
      EclRad ECSdp_3 = ECSw_3 / ((ECSro_3 + ECSri_3) / 2.0);
      double ECSz_31 = ECSz_22;
      double ECScut_3 = 3.0 * CLHEP::cm;
      double ECSz_32 = ECSz_31 + ECSt_3 - ECScut_3;
      double ECSz_33 = ECSz_31 + ECSt_3;
      double support3_Z[3] = {ECSz_31, ECSz_32, ECSz_33};
      double support3_I[3] = {ECSri_3, ECSri_3, ECSri_2 + ECScut_3};
      double support3_O[3] = {ECSro_3, ECSro_3 , ECSro_3};

      G4Polycone* support3Solid = new G4Polycone("eclForwardSupport3Solid", 0.0, ECSdp_3, 3, support3_Z, support3_I, support3_O);
      G4LogicalVolume* support3Logical = new G4LogicalVolume(support3Solid, medIron, "eclForwardSupport3Logical", 0, 0, 0);

      EclCM ECSw_4 = ECSw_3;
      EclCM ECSh_4 = 16.0 * CLHEP::cm;
      EclCM ECSt_4 = 16.0 * CLHEP::cm;
      EclCM ECSro_4 = ECSro_3 + 4.0 * CLHEP::cm;
      EclCM ECSri_4 = ECSro_4 - ECSh_4;
      EclRad ECSdp_4 = ECSw_4 / ((ECSro_4 + ECSri_4) / 2.0);
      double ECSz_42 = ECSz_31;
      double ECSz_41 = ECSz_42 - ECSt_4 / 2.0;
      double ECSz_43 = ECSz_42;
      double ECSz_44 = ECSz_41 + ECSt_4;
      double support4_Z[4] = {ECSz_41, ECSz_42, ECSz_43, ECSz_44};
      double support4_I[4] = {ECSri_4, ECSri_4, ECSro_4 - 3.0 * CLHEP::cm, ECSro_4 - 3.0 * CLHEP::cm };
      double support4_O[4] = {ECSro_4, ECSro_4, ECSro_4, ECSro_4};

      G4Polycone* support4Solid = new G4Polycone("eclForwardSupport4Solid", 0.0, ECSdp_4, 4, support4_Z, support4_I, support4_O);
      G4LogicalVolume* support4Logical = new G4LogicalVolume(support4Solid, medIron, "eclForwardSupport4Logical", 0, 0, 0);

      EclCM ECSw_5 = ECSw_4;
      EclCM ECSh_5 = k_l2r3 - ECSro_4; //8.0;
      EclCM ECSri_5 = ECSro_4;
      EclCM ECSro_5 = ECSri_5 + ECSh_5;
      EclRad ECSdp_5 = ECSw_5 / ((ECSro_5 + ECSri_5) / 2.0);
      double ECSz_51 = ECSz_41;
      double ECSz_52 = ECSz_44;
      double support5_Z[2] = {ECSz_51, ECSz_52};
      double support5_I[2] = {ECSri_5, ECSri_5};
      double support5_O[2] = {ECSro_5, ECSro_5};

      G4Polycone* support5Solid = new G4Polycone("eclForwardSupport5Solid", 0.0, ECSdp_5, 2, support5_Z, support5_I, support5_O);
      G4LogicalVolume* support5Logical = new G4LogicalVolume(support5Solid, medIron, "eclForwardSupport5Logical", 0, 0, 0);

      G4Transform3D ECSrot_1 = G4RotateZ3D(-ECSdp_1 / 2.0 + ECSoffset);
      G4Transform3D ECSrot_2 = G4RotateZ3D(-ECSdp_2 / 2.0 + ECSoffset);
      G4Transform3D ECSrot_3 = G4RotateZ3D(-ECSdp_3 / 2.0 + ECSoffset);
      G4Transform3D ECSrot_4 = G4RotateZ3D(-ECSdp_4 / 2.0 + ECSoffset);
      G4Transform3D ECSrot_5 = G4RotateZ3D(-ECSdp_5 / 2.0 + ECSoffset);

      new G4PVPlacement(ECSrot_1, support1Logical, "eclForwardSupport1Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot_2, support2Logical, "eclForwardSupport2Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot_3, support3Logical, "eclForwardSupport3Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot_4, support4Logical, "eclForwardSupport4Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot_5, support5Logical, "eclForwardSupport5Physical", supportSectorLogical, false, 0, overlaps);

      makeEndcapSupport(1, crystalSectorLogical);

      // forward endcap crystals and container

      int k_forwMPerRing[] = { 3, 3, 4, 4, 4, 6, 6, 6, 6, 6, 6, 9, 9 };

      int iRing = 0;
      int nRing = 0;
      int iPhi = 0;

      for (int iCry = 1; iCry <= 72; ++iCry) {
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
        double Rphi1 = counter.getAngle("K_Rphi1");
        double Rtheta = counter.getAngle("K_Rtheta");
        double Rphi2 = counter.getAngle("K_Rphi2");
        double Pr = counter.getLength("K_Pr") * CLHEP::cm;
        double Ptheta = counter.getAngle("K_Ptheta");
        double Pphi = counter.getAngle("K_Pphi");
        double halflength = 15.0 * CLHEP::cm;

        G4Material* material = medAlTeflon;
        double foilthk = foilthickness;
        if (iCry == 5) {
          material = medAlTeflon_thinpenta;
          foilthk = thinpentafoilthickness;
        } else if (iCry <= 6) {
          material = medAlTeflon_thin;
          foilthk = thinfoilthickness;
        }

        // add forward foil dimensions ////////////////////////////
        double trapangle1 = atan(2 * h1 / (bl1 - tl1)); // the smaller angle of the trap
        double trapangle2 = atan(2 * h2 / (bl2 - tl2));
        double foilh1 = h1 + foilthk;
        double foilh2 = h2 + foilthk;
        double foiltl1 = tl1 + foilthk * tan(trapangle1 / 2);
        double foilbl1 = bl1 + foilthk / tan(trapangle1 / 2);
        double foiltl2 = tl2 + foilthk * tan(trapangle2 / 2);
        double foilbl2 = foiltl2 + (foilbl1 - foiltl1) * foilh2 / foilh1;
        double foilhalflength = halflength + foilthk;

        G4Transform3D m1 = G4RotateZ3D(Rphi1);
        G4Transform3D m2 = G4RotateY3D(Rtheta);
        G4Transform3D m3 = G4RotateZ3D(Rphi2);
        G4Transform3D position = G4Translate3D(Pr * sin(Ptheta) * cos(Pphi),
                                               Pr * sin(Ptheta) * sin(Pphi),
                                               Pr * cos(Ptheta));  // Move over to the left...
        G4Transform3D Tr = G4RotateZ3D(-0.5 * dPhi) * position * m3 * m2 * m1;

        if (iCry == 5) { // Pentagon!!

          double smalls = 1.0E-9 * CLHEP::cm; // should be zero but G4Trap won't accept that
          double trih1 = 0.166375 * CLHEP::cm; // from Belle I code
          double tritl1 = smalls;
          double trih2 = trih1 * bl2 / bl1;
          double tribl1 = bl1;
          double tribl2 = bl2;
          double tritl2 = tritl1 * tribl2 / tribl1;
          double tritheta = atan(((h1 + trih1) - (h2 + trih2)) / (2.0 * halflength)); // z-incline of crystal trigon
          G4Transform3D trigonPos = G4Translate3D(0, -(h1 + h2 + trih1 + trih2) / 2, 0); // move crystal trigon

          /////////////////  add trigon to make the trapezoid into a pentagon //////////////////////

          double triangle1 = atan(2 * trih1 / tribl1); // the smaller angle of the trigon
          double triangle2 = atan(2 * trih2 / tribl2);
          double trifoilbl1 = foilbl1;
          double trifoiltl1 = smalls;
          double trifoilh1 = trih1 + 0.5 * foilthk * (1.0 / cos(triangle1) - 1.0);
          double trifoilbl2 = foilbl2;
          double trifoiltl2 = trifoiltl1 * trifoilbl2 / trifoilbl1;
          double trifoilh2 = trih2 + 0.5 * foilthk * (1.0 / cos(triangle2) - 1.0);
          double trifoiltheta = atan(((foilh1 + trifoilh1) - (foilh2 + trifoilh2)) / (2.0 * foilhalflength)); // z-incline of foil trigon
          G4Transform3D trifoilPos = G4Translate3D(0, -(foilh1 + foilh2 + trifoilh1 + trifoilh2) / 2, 0); // move foil trigon

          G4Trap* foilTrapezoid = new G4Trap((format("eclFwdFoilTrapezoid_%1%") % iCry).str(),
                                             foilhalflength, 0.0, 0.0,
                                             foilh1, foilbl1, foiltl1, alpha1,
                                             foilh2, foilbl2, foiltl2, alpha2);
          G4Trap* foilTrigon = new G4Trap((format("eclFwdFoilTrigon_%1%") % iCry).str(),
                                          foilhalflength, trifoiltheta, 0.5 * M_PI,
                                          trifoilh1, trifoiltl1, trifoilbl1, alpha1,
                                          trifoilh2, trifoiltl2, trifoilbl2, alpha2);
          G4UnionSolid* foilSolid = new G4UnionSolid((format("eclFwdFoilSolid_%1%") % iCry).str(), foilTrapezoid, foilTrigon, trifoilPos);
          G4LogicalVolume* foilLogical = new G4LogicalVolume(foilSolid, material, (format("eclFwdFoilLogical_%1%") % iCry).str(), 0, 0, 0);

          G4Trap* crystalTrapezoid = new G4Trap((format("eclFwdCrystalTrapezoid_%1%") % iCry).str(),
                                                halflength, 0.0, 0.0,
                                                h1, bl1, tl1, alpha1,
                                                h2, bl2, tl2, alpha2);
          G4Trap* crystalTrigon = new G4Trap((format("eclFwdCrystalTrigon_%1%") % iCry).str(),
                                             halflength, tritheta, 0.5 * M_PI,
                                             trih1, tritl1, tribl1, alpha1,
                                             trih2, tritl2, tribl2, alpha2);
          G4UnionSolid* crystalSolid = new G4UnionSolid((format("eclFwdCrystalSolid_%1%") % iCry).str(),
                                                        crystalTrapezoid, crystalTrigon, trigonPos);
          G4LogicalVolume* crystalLogical = new G4LogicalVolume(crystalSolid, medCsI,
                                                                (format("eclFwdCrystalLogical_%1%") % iCry).str(), 0, 0, 0);
          crystalLogical->SetSensitiveDetector(m_sensitive);

          new G4PVPlacement(0, noMove, crystalLogical, (format("eclFwdCrystalPhysical_%1%") % iCry).str(),
                            foilLogical, false, (iCry - 1), overlaps);
          new G4PVPlacement(Tr, foilLogical, (format("eclFwdFoilPhysical_%1%") % iCry).str(), crystalSectorLogical, false, 0, overlaps);

        } // End of Pentagon
        else { // Trapezoids

          G4Trap* foilSolid = new G4Trap((format("eclFwdFoilSolid_%1%") % iCry).str(),
                                         foilhalflength, 0.0, 0.0,
                                         foilh1, foilbl1, foiltl1, alpha1,
                                         foilh2, foilbl2, foiltl2, alpha2);
          G4LogicalVolume* foilLogical = new G4LogicalVolume(foilSolid, material, (format("eclFwdFoilLogical_%1%") % iCry).str(), 0, 0, 0);

          G4Trap* crystalSolid = new G4Trap((format("eclFwdCrystalSolid_%1%") % iCry).str(),
                                            halflength, 0.0, 0.0,
                                            h1, bl1, tl1, alpha1,
                                            h2, bl2, tl2, alpha2);
          G4LogicalVolume* crystalLogical = new G4LogicalVolume(crystalSolid, medCsI,
                                                                (format("eclFwdCrystalLogical_%1%") % iCry).str(), 0, 0, 0);
          crystalLogical->SetSensitiveDetector(m_sensitive);

          new G4PVPlacement(0, noMove, crystalLogical, (format("eclFwdCrystalPhysical_%1%") % iCry).str(),
                            foilLogical, false, (iCry - 1), overlaps);
          new G4PVPlacement(Tr, foilLogical, (format("eclFwdFoilPhysical_%1%") % iCry).str(), crystalSectorLogical, false, 0, overlaps);

        } // End of Trapezoids

        if (isBeamBkgStudy) {
          iPhi = iCry - nRing - 1;
          int diodeId = nRing * k_forwNPhiSegs + iPhi;
          G4LogicalVolume* diodeLogical = new G4LogicalVolume(diodeSolid, medSi, (format("eclFwdDiodeLogical_%1%") % diodeId).str(), 0, 0, 0);
          m_bkgsensitive.push_back(new BkgSensitiveDetector("ECL", diodeId));
          diodeLogical->SetSensitiveDetector(m_bkgsensitive.back());
          G4Transform3D diodePos = G4Translate3D(0, 0, halflength + (diodeHeight) / 2 + 0.1); // Move over to the left...
          new G4PVPlacement(Tr * diodePos,  diodeLogical, (format("eclFwdDiodePhysical_%1%") % iCry).str(),
                            crystalSectorLogical, false, iCry, overlaps);
          if (iPhi == (k_forwMPerRing[iRing] - 1)) {nRing = nRing + k_forwMPerRing[iRing]; iRing++;}
        }

      }//forward endcap crystals

    }//makeForwardEndcap()

    void GeoECLCreator::makeBackwardEndcap(const GearDir& content, G4LogicalVolume* eclLogical)
    {

      G4Material* medAir = geometry::Materials::get("Air");
      G4Material* medIron = geometry::Materials::get("G4_Fe");
      G4Material* medCsI = geometry::Materials::get(content.getString("CsI"));
      G4Material* medSi = geometry::Materials::get(content.getString("Si"));
      G4Material* medAlTeflon = geometry::Materials::get(content.getString("AlTeflon"));
      G4Material* medAlTeflon_thin = geometry::Materials::get(content.getString("AlTeflon_thin"));

      G4ThreeVector noMove(0.0, 0.0, 0.0);

      const double dPhi = TWOPI / k_backNPhiSegs;

      // create a diode in case it is needed for background study
      double diodeWidth = content.getLength("k_diodewidth") * CLHEP::cm;
      double diodeLength = content.getLength("k_diodelength") * CLHEP::cm;
      double diodeHeight = content.getLength("k_diodeheight") * CLHEP::cm;
      G4Box* diodeSolid = new G4Box("diode", diodeWidth / 2, diodeLength / 2, diodeHeight / 2);

      double supportContainer_I[2];
      double supportContainer_O[2];
      double supportContainer_Z[2];

      supportContainer_Z[0] = k_c2z1;
      supportContainer_I[0] = k_l2r3 - 36.0 * CLHEP::cm;
      supportContainer_O[0] = k_c1r1;

      supportContainer_Z[1] = k_backConFarZ;
      supportContainer_I[1] = k_l2r3 - 36.0 * CLHEP::cm;
      supportContainer_O[1] = k_c1r1;

      double crystalContainer_I[12];
      double crystalContainer_O[12];
      double crystalContainer_Z[12];

      crystalContainer_Z[0] = k_backConFarZ;
      crystalContainer_I[0] = k_backConFarInnerR;
      crystalContainer_O[0] = k_c1r1;

      double slope_I = (k_backConNearInnerR - k_backConFarInnerR) / (k_backConNearZ - k_backConFarZ);
      crystalContainer_Z[1] = k_l2z1;
      crystalContainer_I[1] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[1] - crystalContainer_Z[0]);
      crystalContainer_O[1] = k_c1r1;

      crystalContainer_Z[2] = k_l2z1;
      crystalContainer_I[2] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[2] - crystalContainer_Z[0]);
      crystalContainer_O[2] = k_l2r2;

      crystalContainer_Z[3] = k_l2z2;
      crystalContainer_I[3] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[3] - crystalContainer_Z[0]);
      crystalContainer_O[3] = k_l2r2;

      crystalContainer_Z[4] = k_l2z2;
      crystalContainer_I[4] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[4] - crystalContainer_Z[0]);
      crystalContainer_O[4] = k_l2r1;

      crystalContainer_Z[5] = k_barSupBackFarZ;
      crystalContainer_I[5] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[5] - crystalContainer_Z[0]);
      crystalContainer_O[5] = k_l2r1;

      crystalContainer_Z[6] = k_barSupBackFarZ;
      crystalContainer_I[6] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[6] - crystalContainer_Z[0]);
      crystalContainer_O[6] = k_barConBackR1;

      crystalContainer_Z[7] = k_barConBackZOut;
      crystalContainer_I[7] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[7] - crystalContainer_Z[0]);
      crystalContainer_O[7] = k_barConBackR1;

      crystalContainer_Z[8] = k_barCryBackZOut;
      crystalContainer_I[8] = crystalContainer_I[0] + slope_I * (crystalContainer_Z[8] - crystalContainer_Z[0]);
      crystalContainer_O[8] = k_barConBackR2;

      crystalContainer_Z[9] = k_backConNearZ;
      crystalContainer_I[9] = k_backConNearInnerR;
      crystalContainer_O[9] = k_barConInnerR;

      double slope_O = (k_barConInnerR - k_barConBackR2) / (k_barConBackZ3 - k_barCryBackZOut);
      crystalContainer_Z[10] = k_backConNearZ;
      crystalContainer_I[10] = k_barConInnerR;
      crystalContainer_O[10] = crystalContainer_O[7] + slope_O * (crystalContainer_Z[10] - crystalContainer_Z[7]);

      crystalContainer_Z[11] = k_barConBackZ3;
      crystalContainer_I[11] = k_barConInnerR;
      crystalContainer_O[11] = k_barConInnerR;

      G4Polycone* supportContainerSolid = new G4Polycone("eclBackwardSupportContainerSolid", 0.0, TWOPI, 2,
                                                         supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportContainerLogical = new G4LogicalVolume(supportContainerSolid, medAir,
          "eclBackwardSupportContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, supportContainerLogical, "eclBackwardSupportContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* supportSectorSolid = new G4Polycone("eclBackwardSupportSectorSolid", -dPhi, 2.0 * dPhi, 2,
                                                      supportContainer_Z, supportContainer_I, supportContainer_O);
      G4LogicalVolume* supportSectorLogical = new G4LogicalVolume(supportSectorSolid, medAir, "eclBackwardSupportSectorLogical", 0, 0, 0);
      new G4PVReplica("eclBackwardSupportSectorPhysical", supportSectorLogical, supportContainerLogical,
                      kPhi, k_backNPhiSegs / 2, 2.0 * dPhi, 0.0);

      G4Polycone* crystalContainerSolid = new G4Polycone("eclBackwardCrystalContainerSolid", 0.0, TWOPI, 12,
                                                         crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4LogicalVolume* crystalContainerLogical = new G4LogicalVolume(crystalContainerSolid, medAir,
          "eclBackwardCrystalContainerLogical", 0, 0, 0);
      new G4PVPlacement(0, noMove, crystalContainerLogical, "eclBackwardCrystalContainerPhysical", eclLogical, false, 0, overlaps);

      G4Polycone* crystalSectorSolid = new G4Polycone("eclBackwardCrystalSectorSolid", -0.5 * dPhi, dPhi, 12,
                                                      crystalContainer_Z, crystalContainer_I, crystalContainer_O);
      G4LogicalVolume* crystalSectorLogical = new G4LogicalVolume(crystalSectorSolid, medAir, "eclBackwardCrystalSectorLogical", 0, 0, 0);
      new G4PVReplica("eclBackwardCrystalSectorPhysical", crystalSectorLogical, crystalContainerLogical,
                      kPhi, k_backNPhiSegs, dPhi, 0.0);

      // backward endcap support

      const double ECSoffset = 0.0;

      // In fact they're not polycone, but for simplicity...

      double ECSw1 = 13.0 * CLHEP::cm;
      double ECSh1 = 18.5 * CLHEP::cm;
      double ECSt1 = 4.0 * CLHEP::cm;
      double ECSro1 = 149.6 * CLHEP::cm;
      double ECSri1 = ECSro1 - ECSh1;
      double ECSdp1 = ECSw1 / ((ECSro1 + ECSri1) / 2.0);
      double ECSdz1 = ECSt1;
      double ECSz12 = k_backConFarZ;
      double ECSz11 = ECSz12 - ECSdz1;
      double support1_Z[2] = {ECSz11, ECSz12};
      double support1_I[2] = {ECSri1, ECSri1};
      double support1_O[2] = {ECSro1, ECSro1};

      G4Polycone* support1Solid = new G4Polycone("eclBackwardSupport1Solid", 0.0, ECSdp1, 2, support1_Z, support1_I, support1_O);
      G4LogicalVolume* support1Logical = new G4LogicalVolume(support1Solid, medIron, "eclBackwardSupport1Logical", 0, 0, 0);

      double ECSw2 = 6.0 * CLHEP::cm;
      double ECSh2 = 13.5 * CLHEP::cm;
      double ECSt2 = 21.2 * CLHEP::cm;
      double ECSri21 = ECSri1 + 3.5 * CLHEP::cm;
      double ECScut2 = 3.0 * CLHEP::cm;  // guess, I could not find exact fig.
      double ECSri22 = ECSri21 + ECScut2; //cut2*tan(M_PI/4.0);
      double ECSro2 = ECSri21 + ECSh2;
      double ECSdp2 = ECSw2 / ((ECSro2 + ECSri21) / 2.0);
      double ECSz25 = ECSz11;
      double ECSz24 = ECSz25 - ECSt2 + ECScut2;  //  *tan(M_PI/4.0);
      double ECSz23 = ECSz25 - ECSt2;
      double support2_Z[3] = {ECSz23, ECSz24, ECSz25};
      double support2_I[3] = {ECSri22, ECSri21, ECSri21};
      double support2_O[3] = {ECSro2, ECSro2, ECSro2};

      G4Polycone* support2Solid = new G4Polycone("eclBackwardSupport2Solid", 0.0, ECSdp2, 3, support2_Z, support2_I, support2_O);
      G4LogicalVolume* support2Logical = new G4LogicalVolume(support2Solid, medIron, "eclBackwardSupport2Logical", 0, 0, 0);

      double ECSw3 = 14.0 * CLHEP::cm;
      double ECSh3 = k_l2r3 - ECSro2; //18.9;
      double ECSt3 = 16.0 * CLHEP::cm;
      double ECSri3 = ECSro2;
      double ECSro3 = ECSri3 + ECSh3;
      double ECSdp3 = ECSw3 / ((ECSro3 + ECSri3) / 2.0);
      double ECSz37 = ECSz11 - 5.7 * CLHEP::cm;
      double ECSz36 = ECSz37 - ECSt3;

      double support3_Z[2] = {ECSz36, ECSz37};
      double support3_I[2] = {ECSri3, ECSri3};
      double support3_O[2] = {ECSro3, ECSro3};

      G4Polycone* support3Solid = new G4Polycone("eclBackwardSupport3Solid", 0.0, ECSdp3, 2, support3_Z, support3_I, support3_O);
      G4LogicalVolume* support3Logical = new G4LogicalVolume(support3Solid, medIron, "eclBackwardSupport3Logical", 0, 0, 0);

      G4Transform3D ECSrot1 = G4RotateZ3D(-ECSdp1 / 2.0 + ECSoffset);
      G4Transform3D ECSrot2 = G4RotateZ3D(-ECSdp2 / 2.0 + ECSoffset);
      G4Transform3D ECSrot3 = G4RotateZ3D(-ECSdp3 / 2.0 + ECSoffset);

      new G4PVPlacement(ECSrot1, support1Logical, "eclBackwardSupport1Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot2, support2Logical, "eclBackwardSupport2Physical", supportSectorLogical, false, 0, overlaps);
      new G4PVPlacement(ECSrot3, support3Logical, "eclBackwardSupport3Physical", supportSectorLogical, false, 0, overlaps);

      makeEndcapSupport(0, crystalSectorLogical);

      // backward endcap crystals and container

      int k_backMPerRing[] = { 9, 9, 6, 6, 6, 6, 6, 4, 4, 4 };

      int iRing = 0;
      int nRing = 0;
      int iPhi = 0;

      for (int iCry = 73; iCry <= 132; ++iCry) {
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
        double Rphi1 = counter.getAngle("K_Rphi1");
        double Rtheta = counter.getAngle("K_Rtheta");
        double Rphi2 = counter.getAngle("K_Rphi2");
        double Pr = counter.getLength("K_Pr") * CLHEP::cm;
        double Ptheta = counter.getAngle("K_Ptheta");
        double Pphi = counter.getAngle("K_Pphi");
        double halflength = 15.0 * CLHEP::cm;

        G4Material* material = medAlTeflon;
        double foilthk = foilthickness;
        if ((iCry == 110 || iCry == 113 || iCry == 116 || iCry == 119) || (121 <= iCry && iCry <= 128)) {
          material = medAlTeflon_thin;
          foilthk = thinfoilthickness;
        }

        // add backward foil dimensions ////////////////////////////
        double trapangle1 = atan(2 * h1 / (bl1 - tl1)); // the smaller angle of the trap
        double trapangle2 = atan(2 * h2 / (bl2 - tl2));
        double foilh1 = h1 + foilthk;
        double foilh2 = h2 + foilthk;
        double foiltl1 = tl1 + foilthk * tan(trapangle1 / 2);
        double foilbl1 = bl1 + foilthk / tan(trapangle1 / 2);
        double foiltl2 = tl2 + foilthk * tan(trapangle2 / 2);
        double foilbl2 = foiltl2 + (foilbl1 - foiltl1) * foilh2 / foilh1;
        double foilhalflength = halflength + foilthk;

        G4Transform3D m1 = G4RotateZ3D(Rphi1);
        G4Transform3D m2 = G4RotateY3D(Rtheta);
        G4Transform3D m3 = G4RotateZ3D(Rphi2);
        G4Transform3D position = G4Translate3D(Pr * sin(Ptheta) * cos(Pphi),
                                               Pr * sin(Ptheta) * sin(Pphi),
                                               Pr * cos(Ptheta));  // Move over to the left...
        G4Transform3D Tr = G4RotateZ3D(-0.5 * dPhi) * position * m3 * m2 * m1;

        /////////////////  add backward foil ///////////////////////////////////////

        G4Trap* foilSolid = new G4Trap((format("eclBwdFoilSolid_%1%") % iCry).str(),
                                       foilhalflength, 0.0, 0.0,
                                       foilh1, foilbl1, foiltl1, alpha1,
                                       foilh2, foilbl2, foiltl2, alpha2);
        G4LogicalVolume* foilLogical = new G4LogicalVolume(foilSolid, material, (format("eclBwdFoilLogical_%1%") % iCry).str(), 0, 0, 0);

        // place the crystal inside its foil, occupying almost all of the interior volume
        G4Trap* crystalSolid = new G4Trap((format("eclBwdCrystalSolid_%1%") % iCry).str(),
                                          halflength, 0.0, 0.0,
                                          h1, bl1, tl1, alpha1,
                                          h2, bl2, tl2, alpha2);
        G4LogicalVolume* crystalLogical = new G4LogicalVolume(crystalSolid, medCsI,
                                                              (format("eclBwdCrystalLogical_%1%") % iCry).str(), 0, 0, 0);
        crystalLogical->SetSensitiveDetector(m_sensitive);
        new G4PVPlacement(0, noMove, crystalLogical, (format("eclBwdCrystalPhysical_%1%") % iCry).str(),
                          foilLogical, false, (1152 + 6624) / 16 + (iCry - 73), overlaps);
        new G4PVPlacement(Tr, foilLogical, (format("eclBwdFoilPhysical_%1%") % iCry).str(),
                          crystalSectorLogical, false, 0, overlaps);

        if (isBeamBkgStudy) {
          iPhi = (iCry - 72) - nRing - 1;
          int diodeId = nRing * k_backNPhiSegs + iPhi + 7776;
          G4LogicalVolume* diodeLogical = new G4LogicalVolume(diodeSolid, medSi, (format("eclBwdDiodeLogical_%1%") % diodeId).str(), 0, 0, 0);
          m_bkgsensitive.push_back(new BkgSensitiveDetector("ECL", diodeId));
          diodeLogical->SetSensitiveDetector(m_bkgsensitive.back());
          G4Transform3D diodePos = G4Translate3D(0, 0, halflength + (diodeHeight) / 2 + 0.1); // Move over to the left...
          new G4PVPlacement(Tr * diodePos,  diodeLogical, (format("eclBwdDiodePhysical_%1%") % iCry).str(), crystalSectorLogical, false, iCry,
                            overlaps);
          if (iPhi == (k_backMPerRing[iRing] - 1)) {nRing = nRing + k_backMPerRing[iRing]; iRing++;}
        }
      }//crystals + foils

    }//makeBackwardEndcap()

    void GeoECLCreator::makeEndcapSupport(const bool aForward, G4LogicalVolume* crystalSectorLogical)
    {

      G4Material* medIron = geometry::Materials::get("G4_Fe");
      G4Material* medAl = geometry::Materials::get("G4_Al");

      string direction = (aForward ? "Fwd" : "Bwd");

      const int nPhiSegs
      (aForward ? k_forwNPhiSegs : k_backNPhiSegs);

      const double
      signZ(aForward ? 1 : -1);
      const EclCM
      farZ(aForward ? k_forwConFarZ      : k_backConFarZ) ,
           farInnerR(aForward ? k_forwConFarInnerR : k_backConFarInnerR) ,
           farOuterR(aForward ? k_forwConFarOuterR : k_backConFarOuterR) ,
           flatZ(aForward ? farZ - k_forwConHflat : farZ + k_backConHflat) ,
           nearZ(aForward ? k_forwConNearZ     : k_backConNearZ) ,
           nearInnerR(aForward ? k_forwConNearInnerR : k_backConNearInnerR) ,
           nearOuterR(aForward ? k_forwConNearOuterR : k_backConNearOuterR);
      const double
      tanOuterAng(signZ * (farOuterR - nearOuterR) / (flatZ - nearZ)) ,
                  cosOuterAng(cos(atan(tanOuterAng))) ,
                  tanInnerAng(signZ * (farInnerR - nearInnerR) / (farZ - nearZ)) ,
                  cosInnerAng(cos(atan(tanInnerAng)));
      const EclCM
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
                             signZ * (flatCryZ - nearCryZ)*tanInnerAng);

      const EclRad deltaPhi(TWOPI / nPhiSegs);

      const EclCM finGap(0.01 * CLHEP::cm);
      const EclCM pF_a(nearCryOuterR - nearCryInnerR - finGap);
      const EclCM pF_A(farCryOuterR - flatCryInnerR - finGap);
      const EclCM pF_L(signZ * (flatCryZ - nearCryZ));
      const EclCM pF_h(k_endConFinThick / 2);
      const G4Point3D front(pF_a / 2, pF_h / 2, 0);
      const G4Point3D back(flatCryInnerR - nearCryInnerR + pF_A / 2,
                           pF_h / 2, -pF_L);
      const EclCM pF_th(G4Vector3D(front - back).theta());
      const EclCM pF_ph(G4Vector3D(front - back).phi());

      const EclCM pF_xf(pF_L * tan(pF_th)*cos(pF_ph) / 2);
      const G4Point3D f1(-pF_xf - pF_A / 2 , pF_h / 2 , -pF_L / 2);
      const G4Point3D f2(pF_xf - pF_a / 2 , pF_h / 2 ,  pF_L / 2);
      const G4Point3D f3(pF_xf + pF_a / 2 , pF_h / 2 ,  pF_L / 2);
      const G4Point3D s1(flatCryInnerR , 0, flatCryZ);
      const G4Point3D s2(nearCryInnerR , 0, nearCryZ);
      const G4Point3D s3(nearCryOuterR - finGap , 0, nearCryZ);

      G4Transform3D phFinTr0(f1, f2, f3, s1, s2, s3);
      G4Transform3D phFinTr1((aForward ? G4Transform3D::Identity :
                              G4Transform3D(G4TranslateY3D(pF_h)))
                             *phFinTr0);

      G4Transform3D phFinTr2(G4RotateZ3D(deltaPhi)*
                             G4TranslateY3D(-pF_h)*phFinTr1);

      G4Trap* phiFinSolid = new G4Trap((format("ecl%1%PhiFinSolid") % direction).str(),
                                       pF_L / 2, pF_th, pF_ph,
                                       pF_h / 2, pF_A / 2, pF_A / 2, 0.0,
                                       pF_h / 2, pF_a / 2, pF_a / 2, 0.0);
      G4LogicalVolume* phiFinLogical = new G4LogicalVolume(phiFinSolid, medAl,
                                                           (format("ecl%1%PhiFinLogical") % direction).str(), 0, 0, 0);

      // -- begin; reinforcing bars
      const EclCM rfL1 = 2.0 * CLHEP::cm;
      const EclCM rfL2 = 4.0 * CLHEP::cm;
      const EclCM rfL = rfL1 + rfL2;
      const EclCM rfThick = 4.0 * CLHEP::cm;
      const double rfSinPo2 = (rfThick / 2.0) / farCryOuterR;
      const double rfCosPo2 = sqrt(1. -  rfSinPo2 * rfSinPo2);
      const EclCM rfOuterR = farCryOuterR * rfCosPo2; //farCryOuterR;
      const EclCM rfr3 = farCryInnerR;
      const EclCM rfr1 = rfr3 - rfL * tanInnerAng;
      const EclCM rfr2 = rfr3 - rfL2 * tanInnerAng;
      const EclCM rfz3 = farCryZ;
      const EclCM rfz2 = rfz3 - signZ * rfL2;
      const EclCM rfz1 = rfz2 - signZ * rfL1;

      const EclCM rfGap(0.1 * CLHEP::cm);
      const EclCM rf2_a(rfOuterR - rfr2 - rfGap);
      const EclCM rf2_A(rfOuterR - rfr3 - rfGap);
      const EclCM rf2_L(signZ * (rfz3 - rfz2));
      const EclCM rf2_h(rfThick / 2);
      const G4Point3D rf2_front(rf2_a / 2, rf2_h / 2, 0);
      const G4Point3D rf2_back(rfr3 - rfr2 + rf2_A / 2,
                               rf2_h / 2, -rf2_L);
      const EclCM rf2_th(G4Vector3D(rf2_front - rf2_back).theta());
      const EclCM rf2_ph(G4Vector3D(rf2_front - rf2_back).phi());
      G4Trap* rib2Solid = new G4Trap((format("ecl%1%EndcapRib2Solid") % direction).str(),
                                     rf2_L / 2, rf2_th, rf2_ph,
                                     rf2_h / 2, rf2_A / 2, rf2_A / 2, 0.0,
                                     rf2_h / 2, rf2_a / 2, rf2_a / 2, 0.0);
      G4LogicalVolume* rib2Logical = new G4LogicalVolume(rib2Solid, medIron,
                                                         (format("ecl%1%EndcapRib2Logical") % direction).str(), 0, 0, 0);

      const EclCM rf2_xf(rf2_L * tan(rf2_th)*cos(rf2_ph) / 2);
      const G4Point3D rf2_f1(-rf2_xf - rf2_A / 2 , rf2_h / 2 , -rf2_L / 2);
      const G4Point3D rf2_f2(rf2_xf - rf2_a / 2 , rf2_h / 2 ,  rf2_L / 2);
      const G4Point3D rf2_f3(rf2_xf + rf2_a / 2 , rf2_h / 2 ,  rf2_L / 2);
      const G4Point3D rf2_s1(rfr3 , 0, rfz3);
      const G4Point3D rf2_s2(rfr2 , 0, rfz2);
      const G4Point3D rf2_s3(rfOuterR - rfGap , 0, rfz2);

      G4Transform3D rf2Tr0(rf2_f1, rf2_f2, rf2_f3,
                           rf2_s1, rf2_s2, rf2_s3);
      G4Transform3D rf2Tr1((aForward ? G4Transform3D::Identity :
                            G4Transform3D(G4TranslateY3D(rf2_h)))
                           *rf2Tr0);
      G4Transform3D rf2Tr2(G4RotateZ3D(deltaPhi)*
                           G4TranslateY3D(-rf2_h)*rf2Tr1);


      const EclCM rf_a(rfOuterR - rfr1 - rfGap);
      const EclCM rf_A(rfOuterR - rfr2 - rfGap);
      const EclCM rf_L(signZ * (rfz2 - rfz1));
      const EclCM rf_h(rfThick / 2);
      const G4Point3D rf_front(rf_a / 2, rf_h / 2, 0);
      const G4Point3D rf_back(rfr2 - rfr1 + rf_A / 2,
                              rf_h / 2, -rf_L);
      const EclCM rf_th(G4Vector3D(rf_front - rf_back).theta());
      const EclCM rf_ph(G4Vector3D(rf_front - rf_back).phi());
      G4Trap* ribSolid = new G4Trap((format("ecl%1%EndcapRibSolid") % direction).str(),
                                    rf_L / 2, rf_th, rf_ph,
                                    rf_h / 2, rf_A / 2, rf_A / 2, 0.0,
                                    rf_h / 2, rf_a / 2, rf_a / 2, 0.0);
      G4LogicalVolume* ribLogical = new G4LogicalVolume(ribSolid, medIron, (format("ecl%1%EndcapRibLogical") % direction).str(), 0, 0, 0);

      const EclCM rf_xf(rf_L * tan(rf_th)*cos(rf_ph) / 2);
      const G4Point3D rf_f1(-rf_xf - rf_A / 2 , rf_h / 2 , -rf_L / 2);
      const G4Point3D rf_f2(rf_xf - rf_a / 2 , rf_h / 2 ,  rf_L / 2);
      const G4Point3D rf_f3(rf_xf + rf_a / 2 , rf_h / 2 ,  rf_L / 2);
      const G4Point3D rf_s1(rfr2 , 0, rfz2);
      const G4Point3D rf_s2(rfr1 , 0, rfz1);
      const G4Point3D rf_s3(rfOuterR - rfGap , 0, rfz1);

      G4Transform3D rfTr0(rf_f1, rf_f2, rf_f3,
                          rf_s1, rf_s2, rf_s3);
      G4Transform3D rfTr1((aForward ? G4Transform3D::Identity :
                           G4Transform3D(G4TranslateY3D(rf_h)))
                          *rfTr0);

      G4RotateZ3D rot(-0.5 * deltaPhi);
      new G4PVPlacement(rot * phFinTr1, phiFinLogical, (format("ecl%1%PhiFinPhysical") % direction).str(),
                        crystalSectorLogical, false, 0, overlaps);
      new G4PVPlacement(rot * phFinTr2, phiFinLogical, (format("ecl%1%PhiFinPhysical") % direction).str(),
                        crystalSectorLogical, false, 1, overlaps);
      new G4PVPlacement(rot * rf2Tr1, rib2Logical, (format("ecl%1%EndcapRib2Physical") % direction).str(),
                        crystalSectorLogical, false, 0, overlaps);
      new G4PVPlacement(rot * rf2Tr2, rib2Logical, (format("ecl%1%EndcapRib2Physical") % direction).str(),
                        crystalSectorLogical, false, 1, overlaps);

      if (!aForward) {
        new G4PVPlacement(rot * rfTr1, ribLogical, (format("ecl%1%EndcapRibPhysical") % direction).str(),
                          crystalSectorLogical, false, 0, overlaps);
      } else {
        G4Transform3D rfTr2(G4RotateZ3D(deltaPhi) * G4TranslateY3D(-rf_h) * rfTr1);
        new G4PVPlacement(rot * rfTr2, ribLogical, (format("ecl%1%EndcapRibPhysical") % direction).str(),
                          crystalSectorLogical, false, 0, overlaps);
      }

    }//makeEndcapSupport

  }//ecl
}//belle2
