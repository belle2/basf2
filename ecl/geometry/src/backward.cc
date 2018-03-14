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
#include <G4Trd.hh>
#include <G4TwoVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4PVReplica.hh>
#include "G4UserLimits.hh"

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

void Belle2::ECL::GeoECLCreator::backward(G4LogicalVolume& _top)
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
  int overlap = m_overlap;

  int npoints = 1000 * 1000;
  if (b_inner_support_ring) {
    zr_t vc1[] = {{0., 452.3 + 3}, {0., 452.3}, {3., 474.9 - 20 / cosd(27.81)}, {434., 702.27 - 20 / cosd(27.81)}, {434., 702.27}, {3., 474.9}, {3., 452.3 + 3}};
    std::vector<zr_t> contour1(vc1, vc1 + sizeof(vc1) / sizeof(zr_t));
    G4VSolid* part1solid = new BelleLathe("part1solid", phi0, dphi, contour1);
    G4LogicalVolume* part1logical = new G4LogicalVolume(part1solid, Materials::get("SUS304"), "part1logical", 0, 0, 0);
    part1logical->SetVisAttributes(att("iron"));
    auto pv = new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part1logical, "part1physical", top, false, 0, 0);
    if (overlap) pv->CheckOverlaps(npoints);
  }

  if (b_support_wall) {
    // solveing equation to get L : 3+L*cosd(52.90)+1.6*cosd(52.90+90) = 435 - 202.67
    double L = (435 - 202.67 - 3 - 1.6 * cosd(52.90 + 90)) / cosd(52.90);
    zr_t vc23[] = {{0, 452.3 + 3}, {3, 452.3 + 3}, {3, 1190.2}, {3 + L * cosd(52.90), 1190.2 + L * sind(52.90)},
      {3 + L * cosd(52.90) + 1.6 * cosd(52.90 + 90), 1190.2 + L * sind(52.90) + 1.6 * sind(52.90 + 90)}, {3 + 1.6 * cosd(52.90 + 90), 1190.2 + 1.6 * sind(52.90 + 90)}, {0, 1190.2}
    };
    std::vector<zr_t> contour23(vc23, vc23 + sizeof(vc23) / sizeof(zr_t));
    G4VSolid* part23solid = new BelleLathe("part23solid", phi0, dphi, contour23);
    G4LogicalVolume* part23logical = new G4LogicalVolume(part23solid, Materials::get("A5052"), "part23logical", 0, 0, 0);
    part23logical->SetVisAttributes(att("alum"));
    auto pv = new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part23logical, "part23physical", top, false, 0, 0);
    if (overlap) pv->CheckOverlaps(npoints);
  }

  if (b_outer_support_ring) {
    zr_t vc4[] = {{434 - 214.8, 1496 - 20}, {434, 1496 - 20}, {434, 1496 - 5}, {434 + 5, 1496 - 5}, {434 + 5, 1496}, {434 - 199.66, 1496}};
    std::vector<zr_t> contour4(vc4, vc4 + sizeof(vc4) / sizeof(zr_t));
    G4VSolid* part4solid = new BelleLathe("part4solid", phi0, dphi, contour4);
    G4LogicalVolume* part4logical = new G4LogicalVolume(part4solid, Materials::get("SUS304"), "part4logical", 0, 0, 0);
    part4logical->SetVisAttributes(att("iron"));
    auto pv = new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), part4logical, "part4physical", top, false, 0, 0);
    if (overlap) pv->CheckOverlaps(npoints);
  }

  zr_t cont_array_in[] = {{3., 474.9}, {434., 702.27}, {434, 1496 - 20}, {434 - 214.8, 1496 - 20}, {3, 1190.2}};
  std::vector<zr_t> contour_in(cont_array_in, cont_array_in + sizeof(cont_array_in) / sizeof(zr_t));
  G4VSolid* innervolume_solid = new BelleLathe("innervolume_solid", 0, 2 * M_PI, contour_in);
  G4LogicalVolume* innervolume_logical = new G4LogicalVolume(innervolume_solid, Materials::get("G4_AIR"),
                                                             "innervolume_logical", 0, 0, 0);
  innervolume_logical->SetVisAttributes(att("air"));
  auto gpvbp = new G4PVPlacement(G4Translate3D(0, 0, -1020)*G4RotateY3D(M_PI), innervolume_logical, "ECLBackwardPhysical", top, false,
                                 0,
                                 0);
  if (overlap) gpvbp->CheckOverlaps(npoints);

  G4VSolid* innervolumesector_solid = new BelleLathe("innervolumesector_solid", -M_PI / 8, M_PI / 4, contour_in);
  G4LogicalVolume* innervolumesector_logical = new G4LogicalVolume(innervolumesector_solid, Materials::get("G4_AIR"),
      "innervolumesector_logical", 0, 0, 0);
  innervolumesector_logical->SetVisAttributes(att("air"));
  new G4PVReplica("ECLBackwardSectorPhysical", innervolumesector_logical, innervolume_logical, kPhi, 8, M_PI / 4, 0);

  if (b_ribs) {
    double H = 60, W = 20;
    double X0 = 702.27 + 0.001, X1 = 1496 - 20;
    G4TwoVector r0o(X1, 0), r1o(X1 * sqrt(1 - pow(W / X1, 2)), W);
    double beta = asin(W / X0);
    G4TwoVector r0i(X0 / cos(beta / 2), 0), r1i(X0 * cos(beta / 2) - tan(beta / 2) * (W - X0 * sin(beta / 2)), W);
    double dxymzp = (r0o - r0i).x(), dxypzp = (r1o - r1i).x();
    double theta = atan(tand(27.81) / 2);
    double dxymzm = dxymzp + tand(27.81) * H, dxypzm = dxypzp + tand(27.81) * H;

    G4TwoVector m0 = (r0i + r0o) * 0.5, m1 = (r1i + r1o) * 0.5, dm = m1 - m0;
    double alpha = atan(dm.x() / dm.y());

    G4VSolid* solid6_p1 = new G4Trap("solid6_p1", H / 2, theta, 0, W / 2, dxymzm / 2, dxypzm / 2, alpha, W / 2, dxymzp / 2, dxypzp / 2,
                                     alpha);
    G4LogicalVolume* lsolid6_p1 = new G4LogicalVolume(solid6_p1, Materials::get("SUS304"), "lsolid6", 0, 0, 0);
    G4VisAttributes* asolid6 = new G4VisAttributes(G4Colour(1., 0.3, 0.2));
    lsolid6_p1->SetVisAttributes(asolid6);
    G4Transform3D tsolid6_p1(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, W / 2, 434 - H / 2));
    auto pv61 = new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*tsolid6_p1, lsolid6_p1, "psolid6_p1", innervolumesector_logical, false, 0, 0);
    if (overlap) pv61->CheckOverlaps(npoints);
    auto pv62 = new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p1, lsolid6_p1, "psolid6_p2", innervolumesector_logical, false, 0, 0);
    if (overlap) pv62->CheckOverlaps(npoints);

    H = 40;
    dxymzm = dxymzp + tand(27.81) * H, dxypzm = dxypzp + tand(27.81) * H;
    G4VSolid* solid6_p2 = new G4Trap("solid6_p2", H / 2, theta, 0, W / 2, dxypzm / 2, dxymzm / 2, -alpha, W / 2, dxypzp / 2, dxymzp / 2,
                                     -alpha);
    G4LogicalVolume* lsolid6_p2 = new G4LogicalVolume(solid6_p2, Materials::get("SUS304"), "lsolid6", 0, 0, 0);
    lsolid6_p2->SetVisAttributes(asolid6);
    G4Transform3D tsolid6_p2(G4Translate3D(X0 * cos(beta / 2) + (dxymzp / 2 + dxypzp / 2) / 2 - tan(theta)*H / 2, -W / 2, 434 - H / 2));
    auto pv63 = new G4PVPlacement(G4RotateZ3D(0)*tsolid6_p2, lsolid6_p2, "psolid6_p3", innervolumesector_logical, false, 0, 0);
    if (overlap) pv63->CheckOverlaps(npoints);
    auto pv64 = new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid6_p2, lsolid6_p2, "psolid6_p4", innervolumesector_logical, false, 0, 0);
    if (overlap) pv64->CheckOverlaps(npoints);

    G4VSolid* solid7_p8 = new G4Box("solid7_p8", 171. / 2, (140. - 40) / 2 / 2, 40. / 2);
    G4LogicalVolume* lsolid7 = new G4LogicalVolume(solid7_p8, Materials::get("SUS304"), "lsolid7", 0, 0, 0);
    G4VisAttributes* asolid7 = new G4VisAttributes(G4Colour(1., 0.3, 0.2));
    lsolid7->SetVisAttributes(asolid7);
    double dx = sqrt(X1 * X1 - 70 * 70) - 171. / 2;
    G4Transform3D tsolid7_p1(G4Translate3D(dx, -20 - 25, 434 - 40. / 2));
    auto pv71 = new G4PVPlacement(tsolid7_p1, lsolid7, "psolid7_p1", innervolumesector_logical, false, 0, 0);
    if (overlap) pv71->CheckOverlaps(npoints);
    G4Transform3D tsolid7_p2(G4Translate3D(dx, 20 + 25, 434 - 40. / 2));
    auto pv72 = new G4PVPlacement(tsolid7_p2, lsolid7, "psolid7_p2", innervolumesector_logical, false, 0, 0);
    if (overlap) pv72->CheckOverlaps(npoints);

    double L = X1 - (X0 - tand(27.81) * 40) - 10;
    G4VSolid* solid13 = new G4Box("solid13", L / 2, 5. / 2, 18. / 2);
    G4LogicalVolume* lsolid13 = new G4LogicalVolume(solid13, Materials::get("SUS304"), "lsolid13", 0, 0, 0);
    G4VisAttributes* asolid13 = new G4VisAttributes(G4Colour(1., 0.5, 0.5));
    lsolid13->SetVisAttributes(asolid13);
    G4Transform3D tsolid13(G4TranslateZ3D(434 - 60 + 18. / 2)*G4TranslateY3D(-5. / 2 - 0.5 / 2)*G4TranslateX3D(X0 - tand(
                             27.81) * 40 + L / 2 + 5));
    auto pv131 = new G4PVPlacement(tsolid13, lsolid13, "psolid13_p1", innervolumesector_logical, false, 0, 0);
    if (overlap) pv131->CheckOverlaps(npoints);
    auto pv132 = new G4PVPlacement(G4RotateZ3D(M_PI / 8)*tsolid13, lsolid13, "psolid13_p2", innervolumesector_logical, false, 0, 0);
    if (overlap) pv132->CheckOverlaps(npoints);
  }


  double zsep = 135;
  if (b_septum_wall) {
    double d = 5;
    Point_t vin[] = {{434. - zsep, 702.27 - tand(27.81)* zsep}, {434. - 60, 702.27 - tand(27.81) * 60}, {434. - 60, 1496 - 20 - d}, {434. - zsep, 1496 - 20 - d}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector contour_swall[n * 2];
    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2);

    G4VSolid* septumwall_solid = new BelleCrystal("septumwall_solid", n, contour_swall);

    G4LogicalVolume* septumwall_logical = new G4LogicalVolume(septumwall_solid, Materials::get("A5052"),
                                                              "septumwall_logical", 0, 0, 0);
    septumwall_logical->SetVisAttributes(att("alum2"));
    auto pv = new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0), septumwall_logical,
                                "septumwall_physical", innervolumesector_logical, false, 0, 0);
    if (overlap) pv->CheckOverlaps(npoints);

    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall2_solid = new BelleCrystal("septumwall2_solid", n, contour_swall);

    G4LogicalVolume* septumwall2_logical = new G4LogicalVolume(septumwall2_solid, Materials::get("A5052"),
                                                               "septumwall2_logical", 0, 0, 0);
    septumwall2_logical->SetVisAttributes(att("alum2"));
    auto pv0 = new G4PVPlacement(G4RotateZ3D(-M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y,
                                 0.5 / 2 / 2),
                                 septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 0, 0);
    if (overlap) pv0->CheckOverlaps(npoints);
    auto pv1 = new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y,
                                 -0.5 / 2 / 2),
                                 septumwall2_logical, "septumwall2_physical", innervolumesector_logical, false, 1, 0);
    if (overlap) pv1->CheckOverlaps(npoints);
  }

  zr_t vcr[] = {{3., 474.9}, {434. - zsep, 702.27 - tand(27.81)* zsep}, {434 - zsep, 1496 - 20}, {434 - 214.8, 1496 - 20}, {3, 1190.2}};
  std::vector<zr_t> ccr(vcr, vcr + sizeof(vcr) / sizeof(zr_t));
  G4VSolid* crystalvolume_solid = new BelleLathe("crystalvolume_solid", 0, M_PI / 8, ccr);
  G4LogicalVolume* crystalvolume_logical = new G4LogicalVolume(crystalvolume_solid, Materials::get("G4_AIR"),
      "crystalvolume_logical", 0, 0, 0);
  crystalvolume_logical->SetVisAttributes(att("air"));
  auto gpv0 = new G4PVPlacement(G4RotateZ3D(-M_PI / 8), crystalvolume_logical, "ECLBackwardCrystalSectorPhysical_0",
                                innervolumesector_logical,
                                false, 0, 0);
  if (overlap) gpv0->CheckOverlaps(npoints);
  auto gpv1 = new G4PVPlacement(G4RotateZ3D(0), crystalvolume_logical, "ECLBackwardCrystalSectorPhysical_1",
                                innervolumesector_logical, false, 1,
                                0);
  if (overlap) gpv1->CheckOverlaps(npoints);

  if (b_septum_wall) {
    double d = 5, dr = 0.001;
    Point_t vin[] = {{3., 474.9}, {434. - zsep, 702.27 - tand(27.81)* zsep}, {434 - zsep, 1496 - 20 - d - dr}, {434 - 214.8 - d / tand(52.90), 1496 - 20 - d - dr}, {3, 1190.2 - dr}};
    const int n = sizeof(vin) / sizeof(Point_t);
    Point_t c = centerofgravity(vin, vin + n);
    G4ThreeVector contour_swall[n * 2];

    for (int i = 0; i < n; i++) contour_swall[i + 0] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, -0.5 / 2 / 2);
    for (int i = 0; i < n; i++) contour_swall[i + n] = G4ThreeVector(vin[i].x - c.x, vin[i].y - c.y, 0.5 / 2 / 2);

    G4VSolid* septumwall3_solid = new BelleCrystal("septumwall3_solid", n, contour_swall);

    G4LogicalVolume* septumwall3_logical = new G4LogicalVolume(septumwall3_solid, Materials::get("A5052"),
                                                               "septumwall3_logical", 0, 0, 0);
    septumwall3_logical->SetVisAttributes(att("alum2"));
    auto pv0 = new G4PVPlacement(G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y, 0.5 / 2 / 2),
                                 septumwall3_logical,
                                 "septumwall3_physical_0", crystalvolume_logical, false, 0, overlap);
    if (overlap) pv0->CheckOverlaps(npoints);
    auto pv1 = new G4PVPlacement(G4RotateZ3D(M_PI / 8)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2)*G4Translate3D(c.x, c.y,
                                 -0.5 / 2 / 2),
                                 septumwall3_logical, "septumwall3_physical_1", crystalvolume_logical, false, 1, overlap);
    if (overlap) pv1->CheckOverlaps(npoints);
  }

  //  vector<cplacement_t> bp = load_placements("/ecl/data/crystal_placement_backward.dat");
  vector<cplacement_t> bp = load_placements(m_sap, ECLParts::backward);
  if (b_crystals) {
    //    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_backward.dat");
    vector<shape_t*> cryst = load_shapes(m_sap, ECLParts::backward);
    vector<G4LogicalVolume*> wrapped_crystals;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      wrapped_crystals.push_back(wrapped_crystal(s, "backward", 0.20 - 0.02));
    }
    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      const cplacement_t& t = *it;
      auto s = find_if(cryst.begin(), cryst.end(), [&t](const shape_t* shape) {return shape->nshape == t.nshape;});
      if (s == cryst.end()) continue;

      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(t);
      int indx = it - bp.begin();
      auto pv = new G4PVPlacement(twc, wrapped_crystals[s - cryst.begin()], suf("ECLBackwardWrappedCrystal_Physical", indx),
                                  crystalvolume_logical,
                                  false, (1152 + 6624) / 16 + indx, 0);
      if (overlap)pv->CheckOverlaps(npoints);
    }
  }

  if (b_preamplifier) {
    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      G4Transform3D twc = G4Translate3D(0, 0, 3) * get_transform(*it);
      int indx = it - bp.begin();
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.20 + get_pa_box_height() / 2)*G4RotateZ3D(-M_PI / 2), get_preamp(),
                                  suf("phys_backward_preamplifier", indx), crystalvolume_logical, false, (1152 + 6624) / 16 + indx, 0);
      if (overlap)pv->CheckOverlaps(npoints);
    }
  }

  if (b_support_leg) {
    const G4VisAttributes* batt = att("iron");

    G4VSolid* s1 = new G4Box("leg_p1", 130. / 2, 185. / 2, (40. - 5) / 2);
    G4LogicalVolume* l1 = new G4LogicalVolume(s1, Materials::get("SUS304"), "l1", 0, 0, 0);
    G4Transform3D t1 = G4Translate3D(0, 185. / 2, (40. - 5) / 2);
    l1->SetVisAttributes(batt);

    Point_t v3[] = {{ -212. / 2, -135. / 2}, {212. / 2 - 30, -135. / 2}, {212. / 2, -135. / 2 + 30}, {212. / 2, 135. / 2} , { -212. / 2, 135. / 2}};
    const int n3 = sizeof(v3) / sizeof(Point_t);
    G4ThreeVector c3[n3 * 2];

    for (int i = 0; i < n3; i++) c3[i + 0] = G4ThreeVector(v3[i].x, v3[i].y, -60. / 2);
    for (int i = 0; i < n3; i++) c3[i + n3] = G4ThreeVector(v3[i].x, v3[i].y, 60. / 2);

    G4VSolid* s3 = new BelleCrystal("leg_p3", n3, c3);
    G4LogicalVolume* l3 = new G4LogicalVolume(s3, Materials::get("SUS304"), "l3", 0, 0, 0);
    G4Transform3D t3 = G4Translate3D(0, 135. / 2 + 35, 40. - 5. + 212. / 2) * G4RotateY3D(-M_PI / 2);
    l3->SetVisAttributes(batt);

    G4VSolid* s6 = new G4Box("leg_p6", 140. / 2, 189. / 2, 160. / 2);
    G4LogicalVolume* l6 = new G4LogicalVolume(s6, Materials::get("G4_AIR"), "l6", 0, 0, 0);
    G4Transform3D t6 = G4Translate3D(0, 170. + 189. / 2, 57. + 35. + 160. / 2);
    l6->SetVisAttributes(att("air"));

    G4VSolid* s6a = new G4Box("leg_p6a", 140. / 2, (189. - 45.) / 2, 160. / 2);
    G4LogicalVolume* l6a = new G4LogicalVolume(s6a, Materials::get("SUS304"), "l6a", 0, 0, 0);
    l6a->SetVisAttributes(batt);
    new G4PVPlacement(G4TranslateY3D(-45. / 2), l6a, "l6a_physical", l6, false, 0, overlap);

    G4VSolid* s6b = new G4Box("leg_p6b", 60. / 2, 45. / 2, 160. / 2);
    G4LogicalVolume* l6b = new G4LogicalVolume(s6b, Materials::get("SUS304"), "l6b", 0, 0, 0);
    l6b->SetVisAttributes(batt);
    double dy = 189. / 2 - 45 + 45. / 2;
    new G4PVPlacement(G4TranslateY3D(dy), l6b, "l6b_physical", l6, false, 0, overlap);

    G4VSolid* s6c = new G4Box("leg_p6c", 40. / 2, 45. / 2, 22.5 / 2);
    G4LogicalVolume* l6c = new G4LogicalVolume(s6c, Materials::get("SUS304"), "l6c", 0, 0, 0);
    l6c->SetVisAttributes(batt);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(30 + 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 1, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, 20 + 22.5 / 2), l6c, "l6c_physical", l6, false, 2, overlap);
    new G4PVPlacement(G4Translate3D(-30 - 20, dy, -20 - 22.5 / 2), l6c, "l6c_physical", l6, false, 3, overlap);

    G4AssemblyVolume* support_leg = new G4AssemblyVolume();

    support_leg->AddPlacedVolume(l1, t1);
    //    support_leg->AddPlacedVolume(l2,t2);
    support_leg->AddPlacedVolume(l3, t3);
    //    support_leg->AddPlacedVolume(l4,t4);
    //    support_leg->AddPlacedVolume(l5,t5);
    support_leg->AddPlacedVolume(l6, t6);

    G4VSolid* s_all = new G4Box("leg_all", 140. / 2, 359. / 2, (257. - 5.) / 2);
    G4LogicalVolume* l_all = new G4LogicalVolume(s_all, Materials::get("G4_AIR"), "l_all", 0, 0, 0);
    l_all->SetVisAttributes(att("air"));
    G4Transform3D tp = G4Translate3D(0, -359. / 2, -(257. - 5.) / 2);
    support_leg->MakeImprint(l_all, tp, 0, overlap);


    for (int i = 0; i < 8; i++)
      new G4PVPlacement(G4RotateX3D(M_PI)*G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1496 - 185 + 359. / 2,
                        1020 + 434 + 5 + (257. - 5.) / 2), l_all, "lall_physical", top, false, i, overlap);


    G4VSolid* s1a = new G4Box("leg_p1a", 130. / 2, 178. / 2, 5. / 2);
    G4LogicalVolume* l1a = new G4LogicalVolume(s1a, Materials::get("SUS304"), "l1a", 0, 0, 0);
    l1a->SetVisAttributes(batt);
    for (int i = 0; i < 8; i++)
      new G4PVPlacement(G4RotateX3D(M_PI)*G4RotateZ3D(-M_PI / 2 + M_PI / 8 + i * M_PI / 4)*G4Translate3D(0, 1496 - 185 + 178. / 2,
                        1020 + 434 + 5 - 5. / 2), l1a, "l1a_physical", top, false, i, overlap);

  }

}
