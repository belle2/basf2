#include <ecl/geometry/GeoECLCreator.h>
#include "ecl/geometry/BelleLathe.h"
#include "ecl/geometry/BelleCrystal.h"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include <G4VisAttributes.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4AssemblyVolume.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Trd.hh>
#include <G4TwoVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4PVReplica.hh>
#include "G4UserLimits.hh"
#include "G4ReflectionFactory.hh"

#include <iostream>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#include "G4Vector3D.hh"
#include "G4Point3D.hh"
#include "ecl/geometry/shapes.h"
#include <geometry/Materials.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::geometry;

void Belle2::ECL::GeoECLCreator::forward(G4LogicalVolume& _top)
{
  G4LogicalVolume* top = &_top;

  bool sec = 0;
  double phi0 = 0, dphi = (sec) ? M_PI / 16 : 2 * M_PI;

  bool b_inner_support_ring = 1;
  bool b_outer_support_ring = 1;
  bool b_support_wall = 1;
  bool b_ribs = 1;
  bool b_septum_wall = 1;
  bool b_crystals = 1;
  bool b_preamplifier = 1;
  bool b_support_leg = 1;
  bool b_support_structure_13 = 1;
  bool b_support_structure_15 = 1;
  bool b_connectors = 1;
  bool b_boards = 1;
  bool b_cover = 1;

  b_connectors &= b_support_structure_15;
  b_boards &= b_support_structure_15;

  int overlap = m_overlap;
  const double th0 = 13.12, th1 = 32.98;
  const double ZT = 437, ZI = 434, RI = 431, RIp = 532.2, RC = 1200.4, RT = 1415;
  if (b_inner_support_ring) {
    zr_t vc1[] = {{ZI - 487, 410}, {ZT - (RIp - 410 - 20 / cosd(th0)) / tand(th0), 410}, {ZT, RIp - 20 / cosd(th0)}, {ZT, RIp}, {3., RI}, {3., 418}, {ZI - 487, 418}};
    std::vector<zr_t> contour1(vc1, vc1 + sizeof(vc1) / sizeof(zr_t));
    G4VSolid* part1solid = new BelleLathe("fwd_part1solid", phi0, dphi, contour1);
    G4LogicalVolume* part1logical = new G4LogicalVolume(part1solid, Materials::get("SUS304"), "part1logical", 0, 0, 0);
    part1logical->SetVisAttributes(att("iron"));
    new G4PVPlacement(G4Translate3D(0, 0, 1960), part1logical, "part1physical", top, false, 0, overlap);
  }

  if (b_support_wall) {
    // solving equation to get L : 3+L*cosd(th1)+1.6*cosd(th1+90) = 434 + 3 - 107.24
    double L = (ZT - 107.24 - 3 - 1.6 * cosd(th1 + 90)) / cosd(th1);
    double R0 = 418, R1 = RC;
    zr_t vc23[] = {{0, R0}, {3, R0}, {3, R1}, {3 + L * cosd(th1), R1 + L * sind(th1)},
      {3 + L * cosd(th1) + 1.6 * cosd(th1 + 90), R1 + L * sind(th1) + 1.6 * sind(th1 + 90)}, {3 + 1.6 * cosd(th1 + 90), R1 + 1.6 * sind(th1 + 90)}, {0, R1}
    };
    std::vector<zr_t> contour23(vc23, vc23 + sizeof(vc23) / sizeof(zr_t));
    G4VSolid* part23solid = new BelleLathe("fwd_part23solid", phi0, dphi, contour23);
    G4LogicalVolume* part23logical = new G4LogicalVolume(part23solid, Materials::get("A5052"), "part23logical", 0, 0, 0);
    part23logical->SetVisAttributes(att("alum"));
    new G4PVPlacement(G4Translate3D(0, 0, 1960), part23logical, "part23physical", top, false, 0, overlap);
  }

  if (b_outer_support_ring) {
    zr_t vc4[] = {{3 + (RT - 20 - RC) / tand(th1), RT - 20}, {ZT, RT - 20}, {ZT, RT}, {3 + (RT - RC) / tand(th1), RT}};
    std::vector<zr_t> contour4(vc4, vc4 + sizeof(vc4) / sizeof(zr_t));
    G4VSolid* part4solid = new BelleLathe("fwd_part4solid", phi0, dphi, contour4);
    G4LogicalVolume* part4logical = new G4LogicalVolume(part4solid, Materials::get("SUS304"), "part4logical", 0, 0, 0);
    part4logical->SetVisAttributes(att("iron"));
    new G4PVPlacement(G4Translate3D(0, 0, 1960), part4logical, "part4physical", top, false, 0, overlap);
  }

  zr_t cont_array_in[] = {{3., RI}, {ZT, RIp}, {ZT, RT - 20}, {3 + (RT - 20 - RC) / tand(th1), RT - 20}, {3, RC}};
  std::vector<zr_t> contour_in(cont_array_in, cont_array_in + sizeof(cont_array_in) / sizeof(zr_t));
  G4VSolid* innervolume_solid = new BelleLathe("fwd_innervolume_solid", 0, 2 * M_PI, contour_in);
  G4LogicalVolume* innervolume_logical = new G4LogicalVolume(innervolume_solid, Materials::get("G4_AIR"),
                                                             "innervolume_logical", 0, 0, 0);
  innervolume_logical->SetVisAttributes(att("air"));
  new G4PVPlacement(G4Translate3D(0, 0, 1960), innervolume_logical, "ECLForwardPhysical", top, false, 0, overlap);

  G4VSolid* innervolumesector_solid = new BelleLathe("fwd_innervolumesector_solid", -M_PI / 8, M_PI / 4, contour_in);
  G4LogicalVolume* innervolumesector_logical = new G4LogicalVolume(innervolumesector_solid, Materials::get("G4_AIR"),
      "innervolumesector_logical", 0, 0, 0);
  innervolumesector_logical->SetVisAttributes(att("air"));
  new G4PVReplica("ECLForwardSectorPhysical", innervolumesector_logical, innervolume_logical, kPhi, 8, M_PI / 4, 0);

  if (b_ribs) {
    double H = 60, W = 20;
    double X0 = RIp, X1 = RT - 20;
    G4TwoVector r0o(X1, 0), r1o(X1 * sqrt(1 - pow(W / X1, 2)), W);
    double beta = asin(W / X0);
    G4TwoVector r0i(X0 / cos(beta / 2), 0), r1i(X0 * cos(beta / 2) - tan(beta / 2) * (W - X0 * sin(beta / 2)), W);
    double dxymzp = (r0o - r0i).x(), dxypzp = (r1o - r1i).x();
    double theta = atan(tand(th0) / 2);
    double dxymzm = dxymzp + tand(th0) * H, dxypzm = dxypzp + tand(th0) * H;

    G4TwoVector m0 = (r0i + r0o) * 0.5, m1 = (r1i + r1o) * 0.5, dm = m1 - m0;
    double alpha = atan(dm.x() / dm.y());

    G4VSolid* solid6_p1 = new G4Trap("fwd_solid6_p1", H / 2, theta, 0, W / 2, dxymzm / 2, dxypzm / 2, alpha, W / 2, dxymzp / 2,
                                     dxypzp / 2,
                                     alpha);
    G4LogicalVolume* lsolid6_p1 = new G4LogicalVolume(solid6_p1, Materials::get("SUS304"), "lsolid6", 0, 0, 0);
    lsolid6_p1->SetVisAttributes(att("iron"));
    G4Transform3D tsolid6_p1(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, W / 2, ZT - H / 2));
    new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*tsolid6_p1, lsolid6_p1, "psolid6_p1", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p1, lsolid6_p1, "psolid6_p2", innervolumesector_logical, false, 0, overlap);

    H = 40;
    dxymzm = dxymzp + tand(th0) * H, dxypzm = dxypzp + tand(th0) * H;
    G4VSolid* solid6_p2 = new G4Trap("fwd_solid6_p2", H / 2, theta, 0, W / 2, dxypzm / 2, dxymzm / 2, -alpha, W / 2, dxypzp / 2,
                                     dxymzp / 2,
                                     -alpha);
    G4LogicalVolume* lsolid6_p2 = new G4LogicalVolume(solid6_p2, Materials::get("SUS304"), "lsolid6", 0, 0, 0);
    lsolid6_p2->SetVisAttributes(att("iron"));
    G4Transform3D tsolid6_p2(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, -W / 2, ZT - H / 2));
    new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p2, lsolid6_p2, "psolid6_p3", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid6_p2, lsolid6_p2, "psolid6_p4", innervolumesector_logical, false, 0, overlap);

    double hpad = 148.4;
    G4VSolid* solid7_p8 = new G4Box("fwd_solid7_p8", hpad / 2, (140. - 40) / 2 / 2, 40. / 2);
    G4LogicalVolume* lsolid7 = new G4LogicalVolume(solid7_p8, Materials::get("SUS304"), "lsolid7", 0, 0, 0);
    lsolid7->SetVisAttributes(att("iron"));
    double dx = sqrt(X1 * X1 - 70 * 70) - hpad / 2;
    G4Transform3D tsolid7_p1(G4Translate3D(dx, -20 - 25, ZT - 40. / 2));
    new G4PVPlacement(tsolid7_p1, lsolid7, "psolid7_p1", innervolumesector_logical, false, 0, overlap);
    G4Transform3D tsolid7_p2(G4Translate3D(dx, 20 + 25, ZT - 40. / 2));
    new G4PVPlacement(tsolid7_p2, lsolid7, "psolid7_p2", innervolumesector_logical, false, 0, overlap);

    double L = X1 - (X0 - tand(th0) * 40) - 10;
    G4VSolid* solid13 = new G4Box("fwd_solid13", L / 2, 5. / 2, 18. / 2);
    G4LogicalVolume* lsolid13 = new G4LogicalVolume(solid13, Materials::get("SUS304"), "lsolid13", 0, 0, 0);
    lsolid13->SetVisAttributes(att("iron"));
    G4Transform3D tsolid13(G4TranslateZ3D(ZT - 60 + 18. / 2)*G4TranslateY3D(-5. / 2 - 0.5 / 2)*G4TranslateX3D(X0 - tand(
                             th0) * 40 + L / 2 + 5));
    new G4PVPlacement(tsolid13, lsolid13, "psolid13_p1", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid13, lsolid13, "psolid13_p2", innervolumesector_logical, false, 0, overlap);
  }


  double zsep = 125;
  if (b_septum_wall) {
    double d = 5;
    Point_t vin[] = {{ZT - zsep, RIp - tand(th0)* zsep}, {ZT - 60, RIp - tand(th0) * 60}, {ZT - 60, RT - 20 - d}, {ZT - zsep, RT - 20 - d}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector contour_swall[n * 2];
    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2);

    G4VSolid* septumwall_solid = new BelleCrystal("fwd_septumwall_solid", n, contour_swall);

    G4LogicalVolume* septumwall_logical = new G4LogicalVolume(septumwall_solid, Materials::get("A5052"),
                                                              "septumwall_logical", 0, 0, 0);
    septumwall_logical->SetVisAttributes(att("alum2"));
    new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0), septumwall_logical,
                      "septumwall_physical", innervolumesector_logical, false, 0, overlap);

    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall2_solid = new BelleCrystal("fwd_septumwall2_solid", n, contour_swall);

    G4LogicalVolume* septumwall2_logical = new G4LogicalVolume(septumwall2_solid, Materials::get("A5052"),
                                                               "septumwall2_logical", 0, 0, 0);
    septumwall2_logical->SetVisAttributes(att("alum2"));
    new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0.5 / 2 / 2),
                      septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, -0.5 / 2 / 2),
                      septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 1, overlap);
  }

  zr_t vcr[] = {{3., RI}, {ZT - zsep, RIp - tand(th0)* zsep}, {ZT - zsep, RT - 20}, {3 + (RT - 20 - RC) / tand(th1), RT - 20}, {3, RC}};
  std::vector<zr_t> ccr(vcr, vcr + sizeof(vcr) / sizeof(zr_t));
  G4VSolid* crystalvolume_solid = new BelleLathe("fwd_crystalvolume_solid", 0, M_PI / 8, ccr);
  G4LogicalVolume* crystalvolume_logical = new G4LogicalVolume(crystalvolume_solid, Materials::get("G4_AIR"),
      "crystalvolume_logical", 0, 0, 0);
  crystalvolume_logical->SetVisAttributes(att("air"));
  new G4PVPlacement(G4RotateZ3D(-M_PI / 8), crystalvolume_logical, "ECLForwardCrystalSectorPhysical_0", innervolumesector_logical,
                    false, 0, overlap);
  new G4PVPlacement(G4RotateZ3D(0), crystalvolume_logical, "ECLForwardCrystalSectorPhysical_1", innervolumesector_logical, false, 1,
                    overlap);

  if (b_septum_wall) {
    double d = 5, aRC = RC - 30e-6;
    Point_t vin[] = {{3., RI}, {ZT - zsep, RIp - tand(th0)* zsep}, {ZT - zsep, RT - 20 - d}, {3 + (RT - 20 - d - aRC) / tand(th1), RT - 20 - d}, {3, aRC}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector contour_swall[n * 2];

    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall3_solid = new BelleCrystal("fwd_septumwall3_solid", n, contour_swall);

    G4LogicalVolume* septumwall3_logical = new G4LogicalVolume(septumwall3_solid, Materials::get("A5052"),
                                                               "septumwall3_logical", 0, 0, 0);
    septumwall3_logical->SetVisAttributes(att("alum2"));
    new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0.5 / 2 / 2), septumwall3_logical,
                      "septumwall3_physical_0", crystalvolume_logical, false, 0, overlap);
    new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, -0.5 / 2 / 2),
                      septumwall3_logical, "septumwall3_physical_1", crystalvolume_logical, false, 1, overlap);
  }

  //  vector<cplacement_t> bp = load_placements("/ecl/data/crystal_placement_forward.dat");
  vector<cplacement_t> bp = load_placements(m_sap, ECLParts::forward);
  if (b_crystals) {
    //    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_forward.dat");
    vector<shape_t*> cryst = load_shapes(m_sap, ECLParts::forward);
    vector<G4LogicalVolume*> wrapped_crystals;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      wrapped_crystals.push_back(wrapped_crystal(s, "forward", 0.20 - 0.02));
    }

    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      const cplacement_t& t = *it;
      auto s = find_if(cryst.begin(), cryst.end(), [&t](const shape_t* shape) {return shape->nshape == t.nshape;});
      if (s == cryst.end()) continue;

      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(t);
      int indx = it - bp.begin();
      new G4PVPlacement(twc, wrapped_crystals[s - cryst.begin()], suf("ECLForwardWrappedCrystal_Physical", indx), crystalvolume_logical,
                        false, indx, overlap);
    }
  }

  if (b_preamplifier) {
    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(*it);
      int indx = it - bp.begin();
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.20 + get_pa_box_height() / 2)*G4RotateZ3D(-M_PI / 2), get_preamp(),
                                  suf("phys_forward_preamplifier", indx), crystalvolume_logical, false, indx, 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_support_leg) {
    const G4VisAttributes* batt = att("iron");

    G4VSolid* s1 = new G4Box("fwd_leg_p1", 130. / 2, 170. / 2, 40. / 2);
    G4LogicalVolume* l1 = new G4LogicalVolume(s1, Materials::get("SUS304"), "l1", 0, 0, 0);
    G4Transform3D t1 = G4Translate3D(0, 170. / 2, 40. / 2);
    l1->SetVisAttributes(batt);

    G4VSolid* s2 = new G4Box("fwd_leg_p2", 60. / 2, 130. / 2, 137. / 2);
    G4LogicalVolume* l2 = new G4LogicalVolume(s2, Materials::get("SUS304"), "l2", 0, 0, 0);
    G4Transform3D t2 = G4Translate3D(0, 130. / 2 + 35, 40. + 137. / 2);
    l2->SetVisAttributes(batt);

    Point_t v3[] = {{ -75. / 2, -265. / 2}, {75. / 2 - 30, -265. / 2}, {75. / 2, -265. / 2 + 30}, {75. / 2, 265. / 2} , { -75. / 2, 265. / 2}};
    const int n3 = sizeof(v3) / sizeof(Point_t);
    G4ThreeVector c3[n3 * 2];

    for (int i = 0; i < n3; i++) c3[i + 0] = G4ThreeVector(v3[i].x, v3[i].y, -140. / 2);
    for (int i = 0; i < n3; i++) c3[i + n3] = G4ThreeVector(v3[i].x, v3[i].y, 140. / 2);

    G4VSolid* s3 = new BelleCrystal("fwd_leg_p3", n3, c3);
    G4LogicalVolume* l3 = new G4LogicalVolume(s3, Materials::get("SUS304"), "l3", 0, 0, 0);
    G4Transform3D t3 = G4Translate3D(0, 265. / 2 + 35, 40. + 137. + 75. / 2) * G4RotateY3D(-M_PI / 2);
    l3->SetVisAttributes(batt);

    G4VSolid* s4 = new G4Box("fwd_leg_p4", 130. / 2, 5. / 2, 5. / 2);
    G4LogicalVolume* l4 = new G4LogicalVolume(s4, Materials::get("SUS304"), "l4", 0, 0, 0);
    G4Transform3D t4 = G4Translate3D(0, 170. - 5. / 2, -5. / 2);
    l4->SetVisAttributes(batt);

    G4VSolid* s5 = new G4Box("fwd_leg_p5", 140. / 2, 130. / 2, 80. / 2);
    G4LogicalVolume* l5 = new G4LogicalVolume(s5, Materials::get("SUS304"), "l5", 0, 0, 0);
    G4Transform3D t5 = G4Translate3D(0, 180. + 130. / 2, 97. + 80. / 2);
    l5->SetVisAttributes(batt);

    G4VSolid* s6 = new G4Box("fwd_leg_p6", 140. / 2, 110. / 2, 160. / 2);
    G4LogicalVolume* l6 = new G4LogicalVolume(s6, Materials::get("G4_AIR"), "l6", 0, 0, 0);
    G4Transform3D t6 = G4Translate3D(0, 310. + 110. / 2, 97. + 160. / 2);
    l6->SetVisAttributes(att("air"));

    G4VSolid* s6a = new G4Box("fwd_leg_p6a", 140. / 2, (110. - 45.) / 2, 160. / 2);
    G4LogicalVolume* l6a = new G4LogicalVolume(s6a, Materials::get("SUS304"), "l6a", 0, 0, 0);
    l6a->SetVisAttributes(batt);
    new G4PVPlacement(G4TranslateY3D(-45. / 2), l6a, "l6a_physical", l6, false, 0, overlap);

    G4VSolid* s6b = new G4Box("fwd_leg_p6b", 60. / 2, 45. / 2, 160. / 2);
    G4LogicalVolume* l6b = new G4LogicalVolume(s6b, Materials::get("SUS304"), "l6b", 0, 0, 0);
    l6b->SetVisAttributes(batt);
    double dy = 110. / 2 - 45 + 45. / 2;
    new G4PVPlacement(G4TranslateY3D(dy), l6b, "l6b_physical", l6, false, 0, overlap);

    G4VSolid* s6c = new G4Box("fwd_leg_p6c", 40. / 2, 45. / 2, 22.5 / 2);
    G4LogicalVolume* l6c = new G4LogicalVolume(s6c, Materials::get("SUS304"), "l6c", 0, 0, 0);
    l6c->SetVisAttributes(batt);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 1, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 2, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 3, overlap);

    G4AssemblyVolume* support_leg = new G4AssemblyVolume();

    support_leg->AddPlacedVolume(l1, t1);
    support_leg->AddPlacedVolume(l2, t2);
    support_leg->AddPlacedVolume(l3, t3);
    support_leg->AddPlacedVolume(l4, t4);
    support_leg->AddPlacedVolume(l5, t5);
    support_leg->AddPlacedVolume(l6, t6);

    for (int i = 0; i < 8; i++) {
      G4Transform3D tp = G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4) * G4Translate3D(0, 1415 - 165 + 420. / 2,
                         1960 + ZT + (97. + 160.) / 2) * G4Translate3D(0, -420. / 2, -(97. + 160.) / 2);
      support_leg->MakeImprint(top, tp, 0, overlap);
    }

    // G4VSolid* s_all = new G4Box("leg_all", 140. / 2, 420. / 2, (97. + 160) / 2);
    // G4LogicalVolume* l_all = new G4LogicalVolume(s_all, Materials::get("G4_AIR"), "l_all", 0, 0, 0);
    // l_all->SetVisAttributes(att("silv"));
    // G4Transform3D tp = G4Translate3D(0, -420. / 2, -(97. + 160.) / 2);
    // support_leg->MakeImprint(l_all, tp, 0, overlap);

    // for (int i = 0; i < 8; i++)
    //   new G4PVPlacement(G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1415 - 165 + 420. / 2,
    //                     1960 + ZT + (97. + 160.) / 2), l_all, suf("support_leg_physical", i), top, false, i, overlap);

    // for (int i = 0; i < 8; i++)
    //   new G4PVPlacement(G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1415 - 165 + 420. / 2,
    //                     1960 + ZT + (97. + 160.) / 2)*tp * t4, l4, suf("support_leg_p4_physical", i), top, false, i, overlap);
  }


  if (b_support_structure_13) { // numbering scheme as in ECL-004K102.pdf page 13

    // Define one layer as one assembly volume
    G4AssemblyVolume* acs = new G4AssemblyVolume();

    double Z0 = 434, Ro = 1415 - 20;

    // double R2_0 = 667, R2_1 = 1245/cos(M_PI/16), dR2 = R2_1 - R2_0, dz = dR2*cos(M_PI/16);
    // G4VSolid* sv_crystal_support1 = new G4Trd("sv_crystal_support1", 30/2, 30/2, R2_0*sin(M_PI/16)-40*cos(M_PI/16), R2_1*sin(M_PI/16)-40*cos(M_PI/16), dz/2);
    // G4LogicalVolume* lv_crystal_support1 = new G4LogicalVolume(sv_crystal_support1, world_mat, "lv_crystal_support1", 0, 0, 0);
    // //      lv_crystal_support->SetVisAttributes(airvol);
    // new G4PVPlacement(G4Translate3D(R2_0*cos(M_PI/16)+dz/2+3,0,Z0-95)*G4RotateY3D(M_PI/2), lv_crystal_support1, "phys_crystal_support1", crystalSectorLogical, false, 0, overlaps);

    // double dx=425+33, dy1 = dx*tan(M_PI/16), dz = 630+20-67, dy2 = dy1+dz*tan(M_PI/16);
    // G4VSolid* sv_crystal_support1 = new G4Trd("sv_crystal_support1", 30/2, 30/2, dy1, dy2, dz/2);
    // G4LogicalVolume* lv_crystal_support1 = new G4LogicalVolume(sv_crystal_support1, world_mat, "lv_crystal_support1", 0, 0, 0);
    // //      lv_crystal_support->SetVisAttributes(airvol);
    // new G4PVPlacement(G4Translate3D(40/sin(M_PI/16)+dx+dz/2,0,Z0-95)*G4RotateY3D(M_PI/2), lv_crystal_support1, "phys_crystal_support1", crystalSectorLogical, false, 0, overlaps);

    G4VSolid* solid10_p1 = new G4Box("fwd_solid10_p1", 558. / 2 + 9.5, 20. / 2, 105. / 2);
    G4VSolid* solid10_p2 = new G4Box("fwd_solid10_p2", 559. / 2 + 9.5, 11. / 2, 71. / 2);
    G4VSolid* solid10_p3 = new G4SubtractionSolid("fwd_solid10_p3", solid10_p1, solid10_p2, G4Translate3D(0, -11. / 2, 71. / 2 - 17.5));

    G4LogicalVolume* lsolid10 = new G4LogicalVolume(solid10_p3, Materials::get("A6063"), "lsolid10", 0, 0, 0);
    lsolid10->SetVisAttributes(att("alum"));
    G4Transform3D tsolid10_p1(G4RotateZ3D(M_PI / 16)*G4Translate3D(954.5 + 2.55 - 1, -30, Z0 - 105. / 2 - 5));
    acs->AddPlacedVolume(lsolid10, tsolid10_p1);
    //      new G4PVPlacement(tsolid10_p1, lsolid10, "psolid10_p1", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid10_p2(G4RotateZ3D(-M_PI / 16)*G4Translate3D(954.5 + 2.55 - 1, 30, Z0 - 105. / 2 - 5)*G4RotateZ3D(M_PI));
    acs->AddPlacedVolume(lsolid10, tsolid10_p2);
    //      new G4PVPlacement(tsolid10_p2, lsolid10, "psolid10_p2", crystalSectorLogical, false, 0, overlaps);

    G4VSolid* solid1_p1 = new G4Box("fwd_solid1_p1", 100. / 2, 30. / 2, 30. / 2);
    G4VSolid* solid1_p2 = new G4Box("fwd_solid1_p2", 80. / 2, 10. / 2, 31. / 2);
    G4VSolid* solid1_p3 = new G4SubtractionSolid("fwd_solid1_p3", solid1_p1, solid1_p2, G4Transform3D::Identity);

    G4LogicalVolume* lsolid1 = new G4LogicalVolume(solid1_p3, Materials::get("A5052"), "lsolid1", 0, 0, 0);
    lsolid1->SetVisAttributes(att("alum"));

    G4Transform3D tsolid1_p1(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 3 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p1);
    //      new G4PVPlacement(tsolid1_p1, lsolid1, "psolid1_p1", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p2(G4RotateZ3D(M_PI / 16 * (1 - 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 3 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p2);
    //      new G4PVPlacement(tsolid1_p2, lsolid1, "psolid1_p2", crystalSectorLogical, false, 0, overlaps);

    G4Transform3D tsolid1_p3(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 2 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p3);
    //      new G4PVPlacement(tsolid1_p3, lsolid1, "psolid1_p3", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p4(G4RotateZ3D(M_PI / 16 * (1 - 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 2 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p4);
    //      new G4PVPlacement(tsolid1_p4, lsolid1, "psolid1_p4", crystalSectorLogical, false, 0, overlaps);

    G4Transform3D tsolid1_p5(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 1 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p5);
    //      new G4PVPlacement(tsolid1_p5, lsolid1, "psolid1_p5", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p6(G4RotateZ3D(M_PI / 16 * (1 - 2 / 3.))*G4Translate3D(Ro - 8 - 50 - 1 * 140, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1, tsolid1_p6);
    //      new G4PVPlacement(tsolid1_p6, lsolid1, "psolid1_p6", crystalSectorLogical, false, 0, overlaps);

    G4Transform3D tsolid1_p8(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3.))*G4Translate3D(Ro - 8 - 50, 0, Z0 - 100));
    acs->AddPlacedVolume(lsolid1, tsolid1_p8);
    //      new G4PVPlacement(tsolid1_p8, lsolid1, "psolid1_p8", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p9(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3. + (1 + 1. / 3) / 3))*G4Translate3D(Ro - 8 - 50, 0, Z0 - 100));
    acs->AddPlacedVolume(lsolid1, tsolid1_p9);
    //      new G4PVPlacement(tsolid1_p9, lsolid1, "psolid1_p9", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p10(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3. + 2 * (1 + 1. / 3) / 3))*G4Translate3D(Ro - 8 - 50, 0, Z0 - 100));
    acs->AddPlacedVolume(lsolid1, tsolid1_p10);
    //      new G4PVPlacement(tsolid1_p10, lsolid1, "psolid1_p10", crystalSectorLogical, false, 0, overlaps);

    G4VSolid* solid1_p11 = new G4Box("fwd_solid1_p1", 100. / 2, 10. / 2, 30. / 2);
    G4LogicalVolume* lsolid1_p2 = new G4LogicalVolume(solid1_p11, Materials::get("A5052"), "lsolid1_p2", 0, 0, 0);
    lsolid1_p2->SetVisAttributes(att("alum"));
    G4Transform3D tsolid1_p11(G4RotateZ3D(M_PI / 16 * (-1 + 2 / 3. - 1. / 3))*G4Translate3D(Ro - 8 - 50, 0, Z0 - 100));
    acs->AddPlacedVolume(lsolid1_p2, tsolid1_p11);
    //      new G4PVPlacement(tsolid1_p11, lsolid1_p2, "psolid1_p11", crystalSectorLogical, false, 0, overlaps);

    G4VSolid* solid1_p12 = new G4Box("fwd_solid1_p1", 86. / 2, 10. / 2, 30. / 2);
    G4LogicalVolume* lsolid1_p3 = new G4LogicalVolume(solid1_p12, Materials::get("A5052"), "lsolid1_p3", 0, 0, 0);
    lsolid1_p3->SetVisAttributes(att("alum"));
    double alpha_p12 = M_PI / 16 * (-1 + 1. / 3);
    G4Transform3D tsolid1_p12(G4Translate3D(532.2 + 43, 0, Z0 - 75));
    acs->AddPlacedVolume(lsolid1_p3, tsolid1_p12);
    //      new G4PVPlacement(tsolid1_p12, lsolid1_p3, "psolid1_p12", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p13(G4RotateZ3D(alpha_p12)*G4Translate3D(532.2 + 43, 0, Z0 - 75));
    acs->AddPlacedVolume(lsolid1_p3, tsolid1_p13);
    //      new G4PVPlacement(tsolid1_p13, lsolid1_p3, "psolid1_p13", crystalSectorLogical, false, 0, overlaps);
    G4Transform3D tsolid1_p14(G4RotateZ3D(-alpha_p12)*G4Translate3D(532.2 + 43, 0, Z0 - 75));
    acs->AddPlacedVolume(lsolid1_p3, tsolid1_p14);
    //      new G4PVPlacement(tsolid1_p14, lsolid1_p3, "psolid1_p14", crystalSectorLogical, false, 0, overlaps);

    G4VSolid* solid1_p4 = new G4Box("fwd_solid1_p4", 160. / 2, 30. / 2, 30. / 2);
    G4VSolid* solid1_p5 = new G4Box("fwd_solid1_p5", 140. / 2, 10. / 2, 31. / 2);
    G4VSolid* solid1_p7 = new G4SubtractionSolid("fwd_solid1_p7", solid1_p4, solid1_p5, G4Transform3D::Identity);
    G4LogicalVolume* lsolid1_p7 = new G4LogicalVolume(solid1_p7, Materials::get("A5052"), "lsolid1_p7", 0, 0, 0);
    lsolid1_p7->SetVisAttributes(att("alum"));
    G4Transform3D tsolid1_p7(G4Translate3D(Ro - 8 - 80 - 4 * 140 + 4, 0, Z0 - 95));
    acs->AddPlacedVolume(lsolid1_p7, tsolid1_p7);
    //      new G4PVPlacement(tsolid1_p7, lsolid1_p7, "psolid1_p7", crystalSectorLogical, false, 0, overlaps);

    //    int sol2count = 0;
    auto get_bracket = [&](double L, double ang, G4Transform3D & lt) {
      double thick = 3;
      double dL = (ang > 0) ? 0 : thick * abs(tan(ang));

      G4VSolid* solid2_p1 = new G4Box("fwd_solid2_p1", (L - 2 * dL) / 2, thick / 2, 30. / 2);
      G4VSolid* solid2_p2 = new G4Box("fwd_solid2_p2", thick / 2, (15. - dL) / 2, 30. / 2);
      double dx = thick / 2, y0 = (15. + dL) / 2;
      G4Transform3D t1(G4Translate3D(L / 2, -dx, 0.)*G4RotateZ3D(-ang)*G4Translate3D(-dx, y0, 0));
      G4Transform3D t2(G4Translate3D(-L / 2, -dx, 0.)*G4RotateZ3D(ang)*G4Translate3D(dx, y0, 0));
      G4VSolid* solid2_p3 = new G4UnionSolid("fwd_solid2_p3", solid2_p1, solid2_p2, t1);
      G4VSolid* solid2_p4 = new G4UnionSolid("fwd_solid2_p4", solid2_p3, solid2_p2, t2);

      G4Transform3D u((ang > 0) ? G4Transform3D::Identity : G4RotateZ3D(M_PI));
      lt = u * G4Translate3D(dx, 0, 0) * G4RotateZ3D(-M_PI / 2);

      return solid2_p4;
    };
    double obj2_dz = Z0 - 95;
    auto place_solid2 = [&](double dz, double L, double ang, double phi, double mx, double dy) {
      G4Transform3D lt;
      G4LogicalVolume* lsolid2 = new G4LogicalVolume(get_bracket(L, ang, lt), Materials::get("A5052"), "lsolid2", 0, 0, 0);
      lsolid2->SetVisAttributes(att("alum"));

      G4Transform3D tsolid2_p1(G4RotateZ3D(phi)*G4Translate3D(mx, dy, dz)*lt);
      //  string pname("psolid2_p"); pname += to_string(++sol2count);
      acs->AddPlacedVolume(lsolid2, tsolid2_p1);
      //  new G4PVPlacement(tsolid2_p1, lsolid2, pname.c_str(), crystalSectorLogical, false, 0, overlaps);
    };
    auto place_solid3 = [&](double L, double ang, const G4Transform3D & t) {
      G4Transform3D lt;
      G4LogicalVolume* lsolid2 = new G4LogicalVolume(get_bracket(L, ang, lt), Materials::get("A5052"), "lsolid2", 0, 0, 0);
      lsolid2->SetVisAttributes(att("alum"));

      G4Transform3D tsolid2_p1(t * lt);
      //  string pname("psolid2_p"); pname += to_string(++sol2count);
      acs->AddPlacedVolume(lsolid2, tsolid2_p1);
      //  new G4PVPlacement(tsolid2_p1, lsolid2, pname.c_str(), crystalSectorLogical, false, 0, overlaps);
    };

    G4Point3D aa(-50 + 15, 15, 0), bb(-50 + 15, -15, 0);

    aa = tsolid1_p1 * G4Point3D(-50 + 15, 15, 0);
    bb = tsolid1_p2 * G4Point3D(-50 + 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p1 * G4Point3D(50 - 15, 15, 0);
    bb = tsolid1_p2 * G4Point3D(50 - 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    G4Point3D r0(0, 40 / cos(M_PI / 16), 0);
    G4Vector3D np(sin(M_PI / 16), cos(M_PI / 16), 0), mid;
    G4RotateZ3D rb(M_PI / 24);
    double L, phi_uu;


    phi_uu = (tsolid1_p1 * G4Vector3D(1, 0, 0)).angle(G4Vector3D(cos(M_PI / 16), -sin(M_PI / 16), 0));
    G4Vector3D n = tsolid1_p1 * G4Vector3D(-sin(phi_uu / 2), -cos(phi_uu / 2), 0);
    double xj = 50 - 15;
    aa = tsolid1_p1 * G4Point3D(-xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p1 * G4Translate3D(-xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L, -phi_uu / 2, tsolid1_p2 * G4Translate3D(-xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));
    aa = tsolid1_p1 * G4Point3D(xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L,  phi_uu / 2, tsolid1_p1 * G4Translate3D(xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L,  phi_uu / 2, tsolid1_p2 * G4Translate3D(xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));

    aa = tsolid1_p3 * G4Point3D(-xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p3 * G4Translate3D(-xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L, -phi_uu / 2, tsolid1_p4 * G4Translate3D(-xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));
    aa = tsolid1_p3 * G4Point3D(xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L,  phi_uu / 2, tsolid1_p3 * G4Translate3D(xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L,  phi_uu / 2, tsolid1_p4 * G4Translate3D(xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));

    aa = tsolid1_p5 * G4Point3D(-xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p5 * G4Translate3D(-xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L, -phi_uu / 2, tsolid1_p6 * G4Translate3D(-xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));
    aa = tsolid1_p5 * G4Point3D(xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L,  phi_uu / 2, tsolid1_p5 * G4Translate3D(xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L,  phi_uu / 2, tsolid1_p6 * G4Translate3D(xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));

    phi_uu = M_PI / 16;
    n = tsolid1_p7 * G4Vector3D(-sin(phi_uu / 2), -cos(phi_uu / 2), 0);
    xj = 80 - 15;
    aa = tsolid1_p7 * G4Point3D(-xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p7 * G4Translate3D(-xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L, -phi_uu / 2, tsolid1_p7 * G4Translate3D(-xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));
    aa = tsolid1_p7 * G4Point3D(xj, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L,  phi_uu / 2, tsolid1_p7 * G4Translate3D(xj, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));
    place_solid3(L,  phi_uu / 2, tsolid1_p7 * G4Translate3D(xj,  15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0,  L / 2, 0));


    aa = tsolid1_p3 * G4Point3D(-50 + 15, 15, 0);
    bb = tsolid1_p4 * G4Point3D(-50 + 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p3 * G4Point3D(50 - 15, 15, 0);
    bb = tsolid1_p4 * G4Point3D(50 - 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);


    aa = tsolid1_p5 * G4Point3D(-50 + 15, 15, 0);
    bb = tsolid1_p6 * G4Point3D(-50 + 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p5 * G4Point3D(50 - 15, 15, 0);
    bb = tsolid1_p6 * G4Point3D(50 - 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);


    obj2_dz = Z0 - 100;
    aa = tsolid1_p8 * G4Point3D(-50 + 15, 15, 0);
    bb = tsolid1_p9 * G4Point3D(-50 + 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 72, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p8 * G4Point3D(50 - 15, 15, 0);
    bb = tsolid1_p9 * G4Point3D(50 - 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 72, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p9 * G4Point3D(-50 + 15, 15, 0);
    bb = tsolid1_p10 * G4Point3D(-50 + 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 72, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p9 * G4Point3D(50 - 15, 15, 0);
    bb = tsolid1_p10 * G4Point3D(50 - 15, -15, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 72, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    phi_uu = (tsolid1_p8 * G4Vector3D(1, 0, 0)).angle(tsolid1_p11 * G4Vector3D(1, 0, 0));
    r0 = tsolid1_p11 * G4Point3D(-50 + 15, 5, 0);
    np = tsolid1_p11 * G4Vector3D(0, 1, 0);

    n  = tsolid1_p8 * G4Vector3D(-sin(phi_uu / 2), -cos(phi_uu / 2), 0);
    aa = tsolid1_p8 * G4Point3D(-50 + 15, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p8 * G4Translate3D(-50 + 15, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));

    aa = tsolid1_p8 * G4Point3D(50 - 15, -15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, phi_uu / 2, tsolid1_p8 * G4Translate3D(50 - 15, -15, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));

    phi_uu = (tsolid1_p11 * G4Vector3D(1, 0, 0)).angle(G4RotateZ3D(-M_PI / 16) * G4Vector3D(1, 0, 0));
    r0 = G4RotateZ3D(-M_PI / 16) * G4Point3D(0, 40, 0);
    np = G4RotateZ3D(-M_PI / 16) * G4Vector3D(0, 1, 0);

    n  = tsolid1_p11 * G4Vector3D(-sin(phi_uu / 2), -cos(phi_uu / 2), 0);
    aa = tsolid1_p11 * G4Point3D(-50 + 15, -5, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p11 * G4Translate3D(-50 + 15, -5, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));

    aa = tsolid1_p11 * G4Point3D(50 - 15, -5, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, phi_uu / 2, tsolid1_p11 * G4Translate3D(50 - 15, -5, 0)*G4RotateZ3D(-phi_uu / 2)*G4Translate3D(0, -L / 2, 0));

    G4Transform3D ttt(G4RotateZ3D(M_PI / 16)*G4Translate3D(Ro - 8 - 50, 0, Z0 - 100));
    phi_uu = (tsolid1_p10 * G4Vector3D(1, 0, 0)).angle(ttt * G4Vector3D(1, 0, 0));
    r0 = ttt * G4Point3D(0, -40, 0);
    np = ttt * G4Vector3D(0, 1, 0);
    n  = tsolid1_p10 * G4Vector3D(-sin(phi_uu / 2), cos(phi_uu / 2), 0);
    aa = tsolid1_p10 * G4Point3D(-50 + 15, 15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L, -phi_uu / 2, tsolid1_p10 * G4Translate3D(-50 + 15, 15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0, L / 2, 0));

    aa = tsolid1_p10 * G4Point3D(50 - 15, 15, 0);
    L = -((aa - r0) * np) / (n * np);
    place_solid3(L,  phi_uu / 2, tsolid1_p10 * G4Translate3D(50 - 15, 15, 0)*G4RotateZ3D(phi_uu / 2)*G4Translate3D(0, L / 2, 0));

    obj2_dz = Z0 - 75;
    aa = tsolid1_p12 * G4Point3D(-43 + 15, -5, 0);
    bb = tsolid1_p13 * G4Point3D(-43 + 15, 5, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p12 * G4Point3D(43 - 15, -5, 0);
    bb = tsolid1_p13 * G4Point3D(43 - 15, 5, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p12 * G4Point3D(-43 + 15, 5, 0);
    bb = tsolid1_p14 * G4Point3D(-43 + 15, -5, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), -M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    aa = tsolid1_p12 * G4Point3D(43 - 15, 5, 0);
    bb = tsolid1_p14 * G4Point3D(43 - 15, -5, 0);
    place_solid2(obj2_dz, (aa - bb).mag(), M_PI / 48, (aa + bb).phi(), (aa + bb).rho() / 2, 0);

    G4VSolid* solid11_p1 = new G4Box("fwd_solid11_p1", 80. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid11_p2 = new G4Box("fwd_solid11_p2", 81. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid11_p3 = new G4SubtractionSolid("fwd_solid11_p3", solid11_p1, solid11_p2, G4Translate3D(0, -3, 3));

    G4LogicalVolume* lsolid11 = new G4LogicalVolume(solid11_p3, Materials::get("SUS304"), "lsolid11", 0, 0, 0);
    lsolid11->SetVisAttributes(att("iron"));
    G4Transform3D tsolid11_p1(G4RotateZ3D(M_PI / 16)*G4Translate3D(580, -35, Z0 - 40));
    acs->AddPlacedVolume(lsolid11, tsolid11_p1);

    G4Transform3D tsolid11_p2(G4RotateZ3D(-M_PI / 16)*G4Translate3D(580, 35, Z0 - 40)*G4RotateZ3D(M_PI));
    acs->AddPlacedVolume(lsolid11, tsolid11_p2);

    G4VSolid* solid12_p1 = new G4Box("fwd_solid12_p1", 120. / 2, 20. / 2, 115. / 2);
    G4VSolid* solid12_p2 = new G4Box("fwd_solid12_p2", 121. / 2, 12. / 2, 86. / 2);
    G4VSolid* solid12_p3 = new G4SubtractionSolid("fwd_solid12_p3", solid12_p1, solid12_p2, G4Translate3D(0, -11. / 2 + 1,
                                                  86. / 2 - 17.5 - 5));
    G4LogicalVolume* lsolid12 = new G4LogicalVolume(solid12_p3, Materials::get("A6063"), "lsolid12", 0, 0, 0);
    lsolid12->SetVisAttributes(att("alum"));

    G4Transform3D tsolid12_p2(G4RotateZ3D(-M_PI / 16)*G4Translate3D(Ro - 8 - 60, 30, Z0 - 115. / 2)*G4RotateZ3D(M_PI));
    acs->AddPlacedVolume(lsolid12, tsolid12_p2);

    G4VSolid* solid12r_p1 = new G4Box("fwd_solid12r_p1", 100. / 2, 30. / 2, 75. / 2);
    G4VSolid* solid12r_p2 = new G4Box("fwd_solid12r_p2", 101. / 2, 21. / 2, 41. / 2);
    G4VSolid* solid12r_p3 = new G4SubtractionSolid("fwd_solid12r_p3", solid12r_p1, solid12r_p2, G4Translate3D(0, -21. / 2 + 5,
                                                   -41. / 2 - 75. / 2 + 40));
    G4LogicalVolume* lsolid12r = new G4LogicalVolume(solid12r_p3, Materials::get("A6063"), "lsolid12r", 0, 0, 0);
    lsolid12r->SetVisAttributes(att("alum"));
    G4Transform3D tsolid12r_p1(G4RotateZ3D(M_PI / 16)*G4Translate3D(Ro - 8 - 50, -30 - 15, Z0 - 40 - 75. / 2));
    acs->AddPlacedVolume(lsolid12r, tsolid12r_p1);

    G4Transform3D tr = G4RotateZ3D(M_PI / 16) * G4ReflectY3D();
    acs->MakeImprint(innervolumesector_logical,  tr, 0, overlap);
    tr = G4RotateZ3D(-M_PI / 16);
    acs->MakeImprint(innervolumesector_logical,  tr, 1, overlap);
  }

  if (b_support_structure_15) { // numbering scheme as in ECL-004K102.pdf page 15
    G4AssemblyVolume* acs = new G4AssemblyVolume();

    double Z0 = 434, Ro = 1415 - 20;

    G4VSolid* solid1_p1 = new G4Box("fwd_solid1_p1", 10. / 2, 398. / 2, 5. / 2);
    G4VSolid* solid1_p2 = new G4Box("fwd_solid1_p2", 4. / 2, 398. / 2 - 32, 6. / 2);
    G4VSolid* solid1_p3 = new G4SubtractionSolid("fwd_solid1_p3", solid1_p1, solid1_p2, G4Transform3D(G4RotationMatrix(),
                                                 G4ThreeVector(-4,
                                                               0, 0)));

    G4LogicalVolume* lsolid1 = new G4LogicalVolume(solid1_p3, Materials::get("SUS304"), "lsolid1", 0, 0, 0);
    lsolid1->SetVisAttributes(att("iron"));
    G4Transform3D tsolid1_p1(G4Translate3D(1350, -16, Z0 - 40 + 3 + 2.5));
    acs->AddPlacedVolume(lsolid1, tsolid1_p1);

    G4VSolid* solid1a_p1 = new G4Box("fwd_solid1a_p1", 10. / 2, 348.5 / 2, 5. / 2);
    G4VSolid* solid1a_p2 = new G4Box("fwd_solid1a_p2", 4. / 2, 348.5 / 2 - 32, 6. / 2);
    G4VSolid* solid1a_p3 = new G4SubtractionSolid("fwd_solid1a_p3", solid1a_p1, solid1a_p2, G4Transform3D(G4RotationMatrix(),
                                                  G4ThreeVector(4, 0, 0)));

    G4LogicalVolume* lsolid1a = new G4LogicalVolume(solid1a_p3, Materials::get("SUS304"), "lsolid1a", 0, 0, 0);
    lsolid1a->SetVisAttributes(att("iron"));
    G4Transform3D tsolid1a_p1(G4Translate3D(1250, -16, Z0 - 40 + 3 + 2.5));
    acs->AddPlacedVolume(lsolid1a, tsolid1a_p1);

    G4VSolid* solid1b_p1 = new G4Box("fwd_solid1b_p1", 10. / 2, (210 + 210 + 16) / 2, 5. / 2);
    G4VSolid* solid1b_p2 = new G4Box("fwd_solid1b_p2", 4. / 2, (210 + 210 + 16) / 2 - 16, 6. / 2);
    G4VSolid* solid1b_p3 = new G4SubtractionSolid("fwd_solid1b_p3", solid1b_p1, solid1b_p2, G4Transform3D(G4RotationMatrix(),
                                                  G4ThreeVector(4, 0, 0)));

    G4LogicalVolume* lsolid1b = new G4LogicalVolume(solid1b_p3, Materials::get("SUS304"), "lsolid1b", 0, 0, 0);
    lsolid1b->SetVisAttributes(att("iron"));
    G4Transform3D t1b(G4Translate3D(1204 - (210 + 210 + 16) / 2 + 16, 0, Z0 - 52 + 8 + 5. / 2));
    G4Transform3D tsolid1b_p1(t1b * G4TranslateY3D(-10)*G4RotateZ3D(-M_PI / 2));
    acs->AddPlacedVolume(lsolid1b, tsolid1b_p1);
    G4Transform3D tsolid1b_p2(t1b * G4TranslateY3D(10)*G4RotateZ3D(M_PI / 2));
    acs->AddPlacedVolume(lsolid1b, tsolid1b_p2);
    G4Transform3D tsolid1b_p3(t1b * G4TranslateY3D(-110)*G4RotateZ3D(M_PI / 2));
    acs->AddPlacedVolume(lsolid1b, tsolid1b_p3);
    G4Transform3D tsolid1b_p4(t1b * G4TranslateY3D(110)*G4RotateZ3D(-M_PI / 2));
    acs->AddPlacedVolume(lsolid1b, tsolid1b_p4);

    G4VSolid* solid2_p1 = new G4Box("fwd_solid2_p1", 20. / 2, 210. / 2, 5. / 2);
    G4VSolid* solid2_p2 = new G4Box("fwd_solid2_p2", 14. / 2, 210. / 2 - 16, 6. / 2);
    G4VSolid* solid2_p3 = new G4SubtractionSolid("fwd_solid2_p3", solid2_p1, solid2_p2, G4Transform3D(G4RotationMatrix(),
                                                 G4ThreeVector(-4,
                                                               0, 0)));

    G4LogicalVolume* lsolid2 = new G4LogicalVolume(solid2_p3, Materials::get("SUS304"), "lsolid2", 0, 0, 0);
    lsolid2->SetVisAttributes(att("iron"));
    G4Transform3D tsolid2_p1(G4Translate3D(1204 - (210 + 210) - 210 / 2 + 16, 58, Z0 - 52 + 8 + 5. / 2)*G4RotateZ3D(-M_PI / 2));
    acs->AddPlacedVolume(lsolid2, tsolid2_p1);
    //      new G4PVPlacement(tsolid2_p1, lsolid2, "psolid2_p1", crystalSectorLogical, false, 0, overlap);
    G4Transform3D tsolid2_p2(G4Translate3D(1204 - (210 + 210) - 210 / 2 + 16, -58, Z0 - 52 + 8 + 5. / 2)*G4RotateZ3D(M_PI / 2));
    acs->AddPlacedVolume(lsolid2, tsolid2_p2);
    //      new G4PVPlacement(tsolid2_p2, lsolid2, "psolid2_p2", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid3_p1 = new G4Box("fwd_solid3_p1", 32. / 2, 396. / 2, 8. / 2);
    G4LogicalVolume* lsolid3 = new G4LogicalVolume(solid3_p1, Materials::get("SUS304"), "lsolid3", 0, 0, 0);
    lsolid3->SetVisAttributes(att("iron"));
    G4Transform3D tsolid3_p1(G4Translate3D(1204, 0, Z0 - 52 + 8. / 2));
    acs->AddPlacedVolume(lsolid3, tsolid3_p1);
    //      new G4PVPlacement(tsolid3_p1, lsolid3, "psolid3_p1", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid3n_p1 = new G4Box("fwd_solid3n_p1", 32. / 2, 230. / 2, 8. / 2);
    G4LogicalVolume* lsolid3n = new G4LogicalVolume(solid3n_p1, Materials::get("SUS304"), "lsolid3n", 0, 0, 0);
    lsolid3n->SetVisAttributes(att("iron"));
    G4Transform3D tsolid3n_p1(G4Translate3D(1204 - 420, 0, Z0 - 52 + 8. / 2));
    acs->AddPlacedVolume(lsolid3n, tsolid3n_p1);
    //      new G4PVPlacement(tsolid3n_p1, lsolid3n, "psolid3n_p1", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid4_p1 = new G4Box("fwd_solid4_p1", 16. / 2, 160. / 2, 8. / 2);
    G4LogicalVolume* lsolid4 = new G4LogicalVolume(solid4_p1, Materials::get("SUS304"), "lsolid4", 0, 0, 0);
    lsolid4->SetVisAttributes(att("iron"));
    G4Transform3D tsolid4_p1(G4Translate3D(598, 0, Z0 - 52 + 8. / 2));
    acs->AddPlacedVolume(lsolid4, tsolid4_p1);
    //      new G4PVPlacement(tsolid4_p1, lsolid4, "psolid4_p1", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid5_p1 = new G4Box("fwd_solid5_p1", 650. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid5_p2 = new G4Box("fwd_solid5_p2", 651. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid5_p3 = new G4SubtractionSolid("solid5_p3", solid5_p1, solid5_p2, G4Translate3D(0, -3, 3));

    G4LogicalVolume* lsolid5 = new G4LogicalVolume(solid5_p3, Materials::get("SUS304"), "lsolid5", 0, 0, 0);
    lsolid5->SetVisAttributes(att("iron"));
    G4Transform3D tsolid5_p1(G4RotateZ3D(M_PI / 16)*G4Translate3D(910, -45, Z0 - 20 - 15));
    acs->AddPlacedVolume(lsolid5, tsolid5_p1);
    //      new G4PVPlacement(tsolid5_p1, lsolid5, "psolid5_p1", crystalSectorLogical, false, 0, overlap);
    G4Transform3D tsolid5_p3(G4RotateZ3D(-M_PI / 16)*G4Translate3D(910, 45, Z0 - 20 - 15)*G4RotateZ3D(M_PI));
    acs->AddPlacedVolume(lsolid5, tsolid5_p3);
    //      new G4PVPlacement(tsolid5_p3, lsolid5, "psolid5_p3", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid7_p1 = new G4Box("fwd_solid7_p1", 130. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid7_p2 = new G4Box("fwd_solid7_p2", 131. / 2, 30. / 2, 40. / 2);
    G4VSolid* solid7_p3 = new G4SubtractionSolid("fwd_solid7_p3", solid7_p1, solid7_p2, G4Transform3D(G4RotationMatrix(),
                                                 G4ThreeVector(0,
                                                               -3, 3)));
    G4LogicalVolume* lsolid7 = new G4LogicalVolume(solid7_p3, Materials::get("SUS304"), "lsolid7", 0, 0, 0);
    lsolid7->SetVisAttributes(att("iron"));
    G4Transform3D tsolid7_p1(G4RotateZ3D(-M_PI / 16)*G4Translate3D(Ro - 8 - 65, 54, Z0 - 40. / 2)*G4RotateZ3D(M_PI));
    acs->AddPlacedVolume(lsolid7, tsolid7_p1);
    //      new G4PVPlacement(tsolid7_p1, lsolid7, "psolid7_p1", crystalSectorLogical, false, 0, overlap);
    G4Transform3D tsolid7_p2(G4RotateZ3D(M_PI / 16)*G4Translate3D(Ro - 8 - 65 - 8, -85, Z0 - 40. / 2));
    acs->AddPlacedVolume(lsolid7, tsolid7_p2);
    //      new G4PVPlacement(tsolid7_p2, lsolid7, "psolid7_p2", crystalSectorLogical, false, 0, overlap);

    // G4Transform3D tsolid7_p3(G4RotateZ3D(M_PI/16)*G4TranslateZ3D(1960 + 438 - 40./2 - 1)*G4TranslateX3D(1315)*G4TranslateY3D(-85));
    // new G4PVPlacement(tsolid7_p3, lsolid7, "psolid7_p3", crystalSectorLogical, false, 0, overlap);
    // G4Transform3D tsolid7_p4(G4RotateZ3D(M_PI/16)*G4TranslateZ3D(1960 + 438 - 40./2 - 1)*G4TranslateX3D(1315)*G4TranslateY3D(85)*G4RotateZ3D(M_PI));
    // new G4PVPlacement(tsolid7_p4, lsolid7, "psolid7_p4", crystalSectorLogical, false, 0, overlap);

    G4VSolid* solid8_p1 = new G4Box("fwd_solid8_p1", 120. / 2, 10. / 2, 42. / 2);
    G4LogicalVolume* lsolid8 = new G4LogicalVolume(solid8_p1, Materials::get("SUS304"), "lsolid8", 0, 0, 0);
    lsolid8->SetVisAttributes(att("iron"));
    G4Transform3D tsolid8_p1(G4RotateZ3D(-M_PI / 16)*G4Translate3D(Ro - 8 - 60, 34, Z0 - 42. / 2));
    acs->AddPlacedVolume(lsolid8, tsolid8_p1);
    //      new G4PVPlacement(tsolid8_p1, lsolid8, "psolid8_p1", crystalSectorLogical, false, 0, overlap);
    // G4Transform3D tsolid8_p2(G4RotateZ3D(-M_PI/16)*G4TranslateZ3D(1960 + 438 - 42./2 - 1)*G4TranslateX3D(1320)*G4TranslateY3D(34)*G4RotateZ3D(M_PI));
    // new G4PVPlacement(tsolid8_p2, lsolid8, "psolid8_p2", crystalSectorLogical, false, 0, overlap);


    G4VSolid* solid9_p1 = new G4Box("fwd_solid9_p1", 26. / 2, 12. / 2, 26. / 2);
    G4LogicalVolume* lsolid9 = new G4LogicalVolume(solid9_p1, Materials::get("SUS304"), "lsolid9", 0, 0, 0);
    lsolid9->SetVisAttributes(att("iron"));
    G4Transform3D tsolid9_p1(G4RotateZ3D(-M_PI / 16)*G4Translate3D(Ro - 8 - 120 + 26. / 2 + 17, 20 + 9 + 10 + 9 + 3,
                             Z0 - 37 + 26. / 2));
    acs->AddPlacedVolume(lsolid9, tsolid9_p1);
    //      new G4PVPlacement(tsolid9_p1, lsolid9, "psolid9", crystalSectorLogical, false, 0, overlap);


    G4VSolid* solid10_p1 = new G4Box("fwd_solid10_p1", 26. / 2, 12. / 2, 42. / 2);
    G4LogicalVolume* lsolid10 = new G4LogicalVolume(solid10_p1, Materials::get("SUS304"), "lsolid10", 0, 0, 0);
    lsolid10->SetVisAttributes(att("iron"));
    //    G4Transform3D tsolid5_p3(G4RotateZ3D(-M_PI/16)*G4Translate3D(910, 45, Z0-20-15)*G4RotateZ3D(M_PI));
    G4Transform3D tsolid10_p1(G4RotateZ3D(-M_PI / 16)*G4Translate3D(910 + 650. / 2 - 221, 20 + 10 + 6 + 6, Z0 - 52 + 42. / 2));
    acs->AddPlacedVolume(lsolid10, tsolid10_p1);
    //      new G4PVPlacement(tsolid10_p1, lsolid10, "psolid10", crystalSectorLogical, false, 0, overlap);

    if (b_connectors) {
      double t = 2, h20 = 32;
      G4VSolid* solid_connector = new G4Box("fwd_solid_connector", (110 + 2 * 20) / 2, (250 + 2 * 20) / 2, h20 / 2);
      G4VSolid* solid_connector2 = new G4Box("fwd_solid_connector2", (20 + t) / 2, (20 + t) / 2, 1.01 * h20 / 2);
      G4VSolid* solid_connector3 = new G4Box("fwd_solid_connector3", 20 / 2, (250 + 2 * 20 + 2) / 2, h20 / 2);
      solid_connector = new G4SubtractionSolid("fwd_solid_connector", solid_connector, solid_connector2, G4Translate3D(55 + (20 + t) / 2,
                                               -140, 0));
      solid_connector = new G4SubtractionSolid("fwd_solid_connector", solid_connector, solid_connector3, G4Translate3D(-70, 0, -t - 1));
      G4LogicalVolume* lsolid_connector = new G4LogicalVolume(solid_connector, Materials::get("G4_AIR"), "lsolid_connector", 0,
                                                              0, 0);
      lsolid_connector->SetVisAttributes(att("air"));
      G4Transform3D tsolid_connector(G4Translate3D(1360 - 60, 0, Z0 - h20 / 2));
      acs->AddPlacedVolume(lsolid_connector, tsolid_connector);
      //      new G4PVPlacement(tsolid_connector, lsolid_connector, "psolid20", crystalSectorLogical, false, 0, overlap);

      auto lvolume = [&](int part, double dx, double dy, double dz) {
        ostringstream ost(""); ost << "solid20_p" << part;
        G4VSolid* sv = new G4Box(ost.str().c_str(), dx / 2, dy / 2, dz / 2);
        ost.str(""); ost << "lsolid20_p" << part;
        return new G4LogicalVolume(sv, Materials::get("A5052"), ost.str().c_str(), 0, 0, 0);
      };

      auto place = [&](G4LogicalVolume * lv, const G4Translate3D & move, int n) {
        lv->SetVisAttributes(att("alum"));
        ostringstream ost(""); ost << "phys_" << lv->GetName();
        new G4PVPlacement(move, lv, ost.str().c_str(), lsolid_connector, false, n, overlap);
      };
      G4LogicalVolume* lv1 = lvolume(1, 20, 250 + 2 * 20, t);
      place(lv1, G4Translate3D(-55 - 10, 0, h20 / 2 - t / 2), 0);
      G4LogicalVolume* lv1_2 = lvolume(1, 20, 250 + 20, t);
      place(lv1_2, G4Translate3D(55 + 10, 10, h20 / 2 - t / 2), 1);

      G4LogicalVolume* lv2 = lvolume(2, 110., 20., t);
      place(lv2, G4Translate3D(0, 250 / 2 + 10, h20 / 2 - t / 2), 0);
      place(lv2, G4Translate3D(0, -250 / 2 - 10, h20 / 2 - t / 2), 1);

      G4LogicalVolume* lv3 = lvolume(3, 110., t, h20 - t);
      place(lv3, G4Translate3D(0, 250 / 2 + t / 2, -t / 2), 0);
      place(lv3, G4Translate3D(0, -250 / 2 - t / 2, -t / 2), 1);

      G4LogicalVolume* lv4 = lvolume(4, t, 250 + 2 * t, h20 - t);
      place(lv4, G4Translate3D(55 + t / 2, 0, -t / 2), 0);
      place(lv4, G4Translate3D(-55 - t / 2, 0, -t / 2), 1);

      G4LogicalVolume* lv5 = lvolume(5, 7, 250, t);
      place(lv5, G4Translate3D(55 - 7. / 2, 0, -h20 / 2 + t / 2 + t), 0);
      place(lv5, G4Translate3D(-55 + 7. / 2, 0, -h20 / 2 + t / 2 + t), 1);

      G4LogicalVolume* lv6 = lvolume(6, 110 - 14, 7, t);
      place(lv6, G4Translate3D(0, 250 / 2 - 7. / 2, -h20 / 2 + t / 2 + t), 0);
      place(lv6, G4Translate3D(0, -250 / 2 + 7. / 2, -h20 / 2 + t / 2 + t), 1);

      G4LogicalVolume* lv7 = lvolume(7, 110, 250, t);
      place(lv7, G4Translate3D(0, 0, -h20 / 2 + t / 2), 0);

      //      G4LogicalVolume *lv8 = lvolume(8, 90, 10, 30);
      G4VSolid* p8_1 = new G4Box("fwd_solid20_p8_1", 90. / 2, 10. / 2, 30. / 2);
      G4VSolid* p8_2 = new G4Box("fwd_solid20_p8_2", 88. / 2, 8. / 2, 30. / 2);
      G4VSolid* sp8 = new G4SubtractionSolid("fwd_solid20_p8", p8_1, p8_2, G4TranslateZ3D(-1));
      G4LogicalVolume* lv8 = new G4LogicalVolume(sp8, Materials::get("A5052"), "lsolid20_p8", 0, 0, 0);
      lv8->SetVisAttributes(att("alum2"));
      for (int i = 0; i < 10; i++) place(lv8, G4Translate3D(0, 25 * (i - 4.5), -h20 / 2 + t + 30 / 2), i);
    }

    if (b_boards) {
      double hbv = 30;
      G4VSolid* solid_board = new G4Box("fwd_solid_board", (210) / 2, (110) / 2, hbv / 2);
      G4LogicalVolume* lsolid_board = new G4LogicalVolume(solid_board, Materials::get("G4_AIR"), "lsolid_board", 0, 0, 0);
      lsolid_board->SetVisAttributes(att("air"));
      for (int i = 0; i < 1; i++) {
        //  G4Transform3D t0 = G4RotateZ3D(M_PI/16*(1-2*i))*G4Translate3D(598-8+210/2, 0, Z0-52+8+5+hbv/2);
        G4Transform3D t0 = G4Translate3D(598 - 8 + 210 / 2, 0, Z0 - 52 + 8 + 5 + hbv / 2);
        G4Transform3D t1 = t0 * G4Translate3D(210,  110 / 2 + 5, 0);
        G4Transform3D t2 = t0 * G4Translate3D(210, -110 / 2 + 5, 0);
        G4Transform3D t3 = t0 * G4Translate3D(2 * 210,  110 / 2 + 5, 0);
        G4Transform3D t4 = t0 * G4Translate3D(2 * 210, -110 / 2 + 5, 0);
        // new G4PVPlacement(t0, lsolid_board, "phys_solid_board0", crystalSectorLogical, false, 0, overlap);
        // new G4PVPlacement(t1, lsolid_board, "phys_solid_board1", crystalSectorLogical, false, 1, overlap);
        // new G4PVPlacement(t2, lsolid_board, "phys_solid_board2", crystalSectorLogical, false, 2, overlap);
        // new G4PVPlacement(t3, lsolid_board, "phys_solid_board3", crystalSectorLogical, false, 3, overlap);
        // new G4PVPlacement(t4, lsolid_board, "phys_solid_board4", crystalSectorLogical, false, 4, overlap);
        acs->AddPlacedVolume(lsolid_board, t0);
        acs->AddPlacedVolume(lsolid_board, t1);
        acs->AddPlacedVolume(lsolid_board, t2);
        acs->AddPlacedVolume(lsolid_board, t3);
        acs->AddPlacedVolume(lsolid_board, t4);
      }
      G4Material* boxmaterial = Materials::get("G4_GLASS_PLATE");
      auto lvolumeb = [&](int part, double dx, double dy, double dz) {
        ostringstream ost(""); ost << "fwd_sboard_p" << part;
        G4VSolid* sv = new G4Box(ost.str().c_str(), dx / 2, dy / 2, dz / 2);
        ost.str(""); ost << "lboard_p" << part;
        return new G4LogicalVolume(sv, boxmaterial, ost.str().c_str(), 0, 0, 0);
      };

      const G4VisAttributes* asolid20 = att("plate");

      auto placeb = [&](G4LogicalVolume * lv, const G4Translate3D & move, int n) {
        lv->SetVisAttributes(asolid20);
        ostringstream ost(""); ost << "phys_" << lv->GetName();
        new G4PVPlacement(move, lv, ost.str().c_str(), lsolid_board, false, n, overlap);
      };

      double hboard = 2;
      G4LogicalVolume* lb1 = lvolumeb(1, 210, 110, hboard);
      placeb(lb1, G4Translate3D(0, 0, -hbv / 2 + hboard / 2), 0);

      double wcon = 20, hcon = 40, hc = hbv - hboard;
      G4VSolid* sv_connector_bundle = new G4Box("fwd_sv_connector_bundle", 4 * wcon / 2, hcon / 2, hc / 2);
      G4LogicalVolume* lv_connector_bundle = new G4LogicalVolume(sv_connector_bundle, Materials::get("G4_AIR"),
                                                                 "lv_connector_bundle", 0, 0, 0);
      lv_connector_bundle->SetVisAttributes(att("air"));
      new G4PVPlacement(G4Translate3D(-210 / 2 + 10 + wcon * 2, -110 / 2 + hcon / 2, -hbv / 2 + hboard + hc / 2), lv_connector_bundle,
                        "pv_connector_bundle", lsolid_board, false, 0, overlap);
      new G4PVPlacement(G4Translate3D(-210 / 2 + 10 + wcon * 2,    10 + hcon / 2, -hbv / 2 + hboard + hc / 2), lv_connector_bundle,
                        "pv_connector_bundle", lsolid_board, false, 0, overlap);
      new G4PVPlacement(G4Translate3D(10 + wcon * 2, -110 / 2 + hcon / 2, -hbv / 2 + hboard + hc / 2), lv_connector_bundle,
                        "pv_connector_bundle", lsolid_board, false, 0, overlap);
      new G4PVPlacement(G4Translate3D(10 + wcon * 2,    10 + hcon / 2, -hbv / 2 + hboard + hc / 2), lv_connector_bundle,
                        "pv_connector_bundle", lsolid_board, false, 0, overlap);

      G4VSolid* sv_crystal_connector = new G4Box("fwd_sv_crystal_connector", wcon / 2, hcon / 2, hc / 2);
      G4LogicalVolume* lv_crystal_connector = new G4LogicalVolume(sv_crystal_connector, Materials::get("G4_AIR"),
                                                                  "lv_crystal_connector", 0, 0, 0);
      lv_crystal_connector->SetVisAttributes(att("air"));

      new G4PVPlacement(G4Translate3D(-1.5 * 20, 0, 0), lv_crystal_connector, "pv_crystal_connector", lv_connector_bundle, false, 0,
                        overlap);
      new G4PVPlacement(G4Translate3D(-0.5 * 20, 0, 0), lv_crystal_connector, "pv_crystal_connector", lv_connector_bundle, false, 1,
                        overlap);
      new G4PVPlacement(G4Translate3D(0.5 * 20, 0, 0), lv_crystal_connector, "pv_crystal_connector", lv_connector_bundle, false, 2,
                        overlap);
      new G4PVPlacement(G4Translate3D(1.5 * 20, 0, 0), lv_crystal_connector, "pv_crystal_connector", lv_connector_bundle, false, 3,
                        overlap);

      G4VSolid* sv_crystal_connector_p1 = new G4Box("fwd_sv_crystal_connector_p1", 8 / 2, 30 / 2, 20 / 2);
      G4LogicalVolume* lv_crystal_connector_p1 = new G4LogicalVolume(sv_crystal_connector_p1, Materials::get("SUS304"),
          "lv_crystal_connector_p1", 0, 0, 0);
      lv_crystal_connector_p1->SetVisAttributes(att("connector"));

      new G4PVPlacement(G4Translate3D(-5, 0, -hc / 2 + 20. / 2), lv_crystal_connector_p1, "pv_crystal_connector_p1", lv_crystal_connector,
                        false, 0, overlap);

      G4VSolid* sv_capacitor = new G4Tubs("fwd_sv_capacitor", 0, 5, 5. / 2, 0, 2 * M_PI);
      G4LogicalVolume* lv_capacitor = new G4LogicalVolume(sv_capacitor, Materials::get("SUS304"), "lv_capacitor", 0, 0, 0);
      lv_capacitor->SetVisAttributes(att("capacitor"));

      new G4PVPlacement(G4Translate3D(5, -15, -hc / 2 + 5. / 2), lv_capacitor, "pv_capacitor", lv_crystal_connector, false, 0, overlap);
      new G4PVPlacement(G4Translate3D(5, -5, -hc / 2 + 5. / 2), lv_capacitor, "pv_capacitor", lv_crystal_connector, false, 1, overlap);
      new G4PVPlacement(G4Translate3D(5,  5, -hc / 2 + 5. / 2), lv_capacitor, "pv_capacitor", lv_crystal_connector, false, 2, overlap);
      new G4PVPlacement(G4Translate3D(5, 15, -hc / 2 + 5. / 2), lv_capacitor, "pv_capacitor", lv_crystal_connector, false, 3, overlap);

      G4VSolid* sv_board_connector_p1 = new G4Box("fwd_sv_board_connector_p1", 80. / 2, 8. / 2, 20. / 2);
      G4LogicalVolume* lv_board_connector_p1 = new G4LogicalVolume(sv_board_connector_p1, Materials::get("SUS304"),
          "lv_board_connector_p1", 0, 0, 0);
      lv_board_connector_p1->SetVisAttributes(att("connector"));

      new G4PVPlacement(G4Translate3D(-210 / 2 + 10 + 80 / 2, 0, -hbv / 2 + hboard + 20. / 2), lv_board_connector_p1,
                        "pv_board_connector_p1", lsolid_board, false, 0, overlap);
      new G4PVPlacement(G4Translate3D(210 / 2 - 10 - 80 / 2, 0, -hbv / 2 + hboard + 20. / 2), lv_board_connector_p1,
                        "pv_board_connector_p1", lsolid_board, false, 1, overlap);
    }

    G4Transform3D tr = G4RotateZ3D(M_PI / 16) * G4ReflectY3D();
    acs->MakeImprint(innervolumesector_logical,  tr, 0, overlap);
    tr = G4RotateZ3D(-M_PI / 16);
    acs->MakeImprint(innervolumesector_logical,  tr, 1, overlap);
  }// end of ECL-004K102.pdf page 15

  if (b_cover) {
    G4VSolid* solid8_p1 = new G4Tubs("fwd_solid8_p1", RI + tand(13.12) * (434 + 1) - 20 / cosd(13.12), 1415, 1. / 2, -M_PI / 16,
                                     M_PI / 8);
    G4VSolid* solid8_p2 = new G4Box("fwd_solid8_p2", 130. / 2, 270. / 2, 2. / 2);
    G4VSolid* solid8_p3 = new G4Tubs("fwd_solid8_p3", 0, 16, 2, 0, 2 * M_PI);
    G4VSolid* solid8_p4 = new G4Box("fwd_solid8_p4", 130. / 2, (75. - 2 * 16.) / 2, 2. / 2);
    double width_p5 = 180;
    G4VSolid* solid8_p5 = new G4Box("fwd_solid8_p5", width_p5 / 2, 2.5 + 75. / 2, 2. / 2);

    double xx0 = 1415 - 47.8715;
    G4VSolid* solid8 = new G4SubtractionSolid("fwd_solid8", solid8_p1, solid8_p2, G4TranslateX3D(xx0 - 130. / 2));
    double xx1 = xx0 + 1.7;
    solid8 = new G4SubtractionSolid("fwd_solid8", solid8, solid8_p3, G4Translate3D(xx1, 159.5, 0));
    solid8 = new G4SubtractionSolid("fwd_solid8", solid8, solid8_p3, G4Translate3D(xx1, 202.5, 0));
    solid8 = new G4SubtractionSolid("fwd_solid8", solid8, solid8_p4, G4Translate3D(xx1 - 16 + 130. / 2, (202.5 + 159.5) / 2, 0));
    solid8 = new G4SubtractionSolid("fwd_solid8", solid8, solid8_p5, G4Translate3D(xx1 + width_p5 / 2, (202.5 + 159.5) / 2, 0));
    solid8 = new G4SubtractionSolid("fwd_solid8", solid8, solid8_p5, G4Translate3D(xx1 - 130 + (1230.77 - 1230.88), -177.57,
                                    0)*G4RotateZ3D(-M_PI / 16)*G4Translate3D(width_p5 / 2, -75. / 2, 0));

    // for(int i=0;i<100000;i++){
    //  G4ThreeVector v = solid8->GetPointOnSurface();
    //  G4cout<<v.x()<<" "<<v.y()<<" "<<v.z()<<"\n";
    // }

    G4LogicalVolume* lsolid8 = new G4LogicalVolume(solid8, Materials::get("A5052"), "lsolid8", 0, 0, 0);
    lsolid8->SetVisAttributes(att("alum"));
    for (int i = 0; i < 8; i++) {
      G4Transform3D tc = G4Translate3D(0, 0, 1960 + 3 + 434 + 0.5) * G4RotateZ3D(M_PI / 8 + i * M_PI / 4);
      new G4PVPlacement(tc * G4RotateZ3D(M_PI / 16), lsolid8, suf("cover", 0 + 2 * i), top, false, 0 + 2 * i, overlap);
      G4ReflectionFactory::Instance()->Place(tc * G4RotateZ3D(-M_PI / 16)*G4ReflectY3D(), suf("cover", 0 + 2 * i), lsolid8, top, false,
                                             1 + 2 * i, overlap);
    }
  }
  // end of ECL-004K102.pdf page 11 - 12

}
