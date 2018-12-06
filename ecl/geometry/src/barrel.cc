#include <ecl/geometry/GeoECLCreator.h>
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

#include <iostream>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#include "G4Vector3D.hh"
#include "G4Point3D.hh"

#include "ecl/geometry/BelleLathe.h"
#include "ecl/geometry/BelleCrystal.h"
#include "ecl/geometry/shapes.h"
#include <geometry/Materials.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::geometry;
using namespace ECL;

namespace {
  Point_t p1[]  = {{0.0, 0.0}, {263.4, 0.0}, {263.4, 197.2}, {248.6, 206.6}, {212.0, 152.6}, {145.2, 199.0}, {108.5, 146.6}, {46.4, 192.1}, {7.9, 140.5}, { -50.0, 185.2}, { -90.3, 134.2}, { -143.8, 178.3}, { -170.7, 145.0}};
  Point_t p2[]  = {{0.0, 0.0}, {384.8, 0.0}, {181.9, 172.5}, {136.8, 122.3}, {90.7, 163.9}, {46.5, 115.6}, {4.1, 156.3}, { -42.5, 109.9}, { -80.9, 149.5}, { -110.9, 119.8}};
  Point_t p3[]  = {{0.0, 0.0}, {343.2, 0.0}, {209.6, 144.2}, {158.0, 98.1}, {125.8, 135.0}, {74.3, 92.4}, {46.1, 128.4}, { -6.9, 87.3}, { -32.5, 121.3}, { -67.3, 95.8}};
  Point_t p4[]  = {{0.0, 0.0}, {311.5, 0.0}, {229.5, 116.7}, {170.3, 76.5}, {150.6, 107.6}, {92.8, 71.9}, {75.0, 101.2}, {16.1, 67.5}, {1.0, 95.2}, { -37.8, 75.6}};
  Point_t p5[]  = {{0.0, 0.0}, {289.2, 0.0}, {243.1, 92.2}, {177.4, 60.9}, {166.6, 85.0}, {103.3, 57.8}, {94.3, 79.8}, {30.0, 55.5}, {22.9, 75.3}, { -19.3, 61.8}};
  Point_t p6[]  = {{0.0, 0.0}, {276.1, 0.0}, {253.3, 73.3}, {185.2, 53.4}, {180.7, 69.1}, {110.5, 51.9}, {107.5, 65.3}, {38.7, 51.8}, {36.7, 62.8}, { -7.5, 56.8}};
  Point_t p7[]  = {{0.0, 0.0}, {253.1, 0.0}, {244.8, 62.7}, {171.0, 54.0}, {170.2, 60.3}, {100.8, 55.6}, {100.3, 59.6}, {38.2, 58.1}, {38.1, 59.5}, {0.0, 60.9}};
  Point_t p11[] = {{0.0, 0.0}, {314.1, 0.0}, {351.9, 75.6}, {312.4, 96.9}, {297.5, 69.4}, {238.2, 102.9}, {221.4, 73.2}, {163.1, 108.8}, {143.5, 77.4}, {87.1, 114.8}, {64.3, 81.7}, {9.3, 121.5}, {0.0, 108.4}};
  Point_t* pp[] = {p1, p2, p3, p4, p5, p6, p7, p11};
  int npp[] = {13, 10, 10, 10, 10, 10, 10, 13};

  G4VSolid* get_crystal_support(int n)
  {
    std::vector<G4TwoVector> p; p.reserve(npp[n]);
    for (int i = 0; i < npp[n]; i++)
      p.push_back(G4TwoVector(pp[n][i].x, pp[n][i].y));

    string name("barrel_crystal_support");
    name += to_string(n);

    G4TwoVector off(0, 0);
    return new G4ExtrudedSolid(name, p, 1.5, off, 1, off, 1);
  }
}

void Belle2::ECL::GeoECLCreator::barrel(G4LogicalVolume& _top)
{
  G4LogicalVolume* top = &_top;

  bool b_crystals = true;
  bool b_forward_support_legs = true;
  bool b_backward_support_legs = true;
  bool b_forward_support_ring = true;
  bool b_backward_support_ring = true;
  bool b_forward_support_wall = true;
  bool b_backward_support_wall = true;
  bool b_crystal_support = true;
  bool b_preamplifier = true;
  bool b_septum_walls = true;
  bool b_inner_support_wall = true;
  bool b_support_ribs = true;
  bool b_outer_plates = true;
  bool b_forward_part5 = true;
  bool b_crystal_holder = true;
  bool b_pipe = true;

  double phi0 = M_PI / 2 - M_PI / 72, dphi = 2 * M_PI; //2*2*M_PI/72;

  int nseg = 72;

  int overlap = m_overlap;

  G4LogicalVolume* sectorlogical; // tilted sector
  {
    double R = 1251.6;
    double zp = 4.9, zm = 2.5;
    //    zr_t bint[] = {{-1000.8-zm, 0}, {1982.6 + zp,0}, {2288.,198.2 - zp*tand(32.982)}, {2288.,404 + 12}, {-1219.,404 + 12}, {-1219., 404 -112.6-zm*tand(52.897)-3.2}};
    zr_t bint[] = {{ -1000.8 - zm, 0}, {1982.6 + zp, 0}, {2288., 198.2 - zp * tand(32.982)}, {2288., 1582 - R}, {2380, 1582 - R}, {2380, 404 + 12}, { -1330., 404 + 12}, { -1330., 1543 + 84 - R}, { -1219., 1543 + 84 - R}, { -1219., 404 - 112.6 - zm * tand(52.897) - 3.2}};
    for (unsigned int i = 0; i < sizeof(bint) / sizeof(zr_t); i++) bint[i].r += R;
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    G4VSolid* sect0 = new BelleLathe("sect0", phi0, 2 * M_PI, contourb);

    G4Vector3D n0(cos(phi0), sin(phi0), 0);
    // shift r0 by half of septum wall thickness so from one side of the sector the wall will be completely inside the sector volume
    G4Point3D r0 = R * n0 + 0.5 / 2 * (G4RotateZ3D(M_PI / 2) * n0), rt0 = r0 + G4RotateZ3D(M_PI / 144 - M_PI / 72) * (G4Vector3D(0, 417,
                   0)), r1 = G4RotateZ3D(2 * M_PI / 72) * r0, rt1 = G4RotateZ3D(2 * M_PI / 72) * rt0;
    G4Transform3D t0 = G4RotateZ3D(-M_PI / 144) * G4Translate3D(-r0);
    G4Point3D tr0 = t0 * r0, trt0 = t0 * rt0, tr1 = t0 * r1, trt1 = t0 * rt1;
    G4Vector3D tn1 = trt1 - tr1;
    double v0 = -(tr1.y() - tr0.y()) / tn1.y();
    double v1 = -(tr1.y() - trt0.y()) / tn1.y();

    G4Point3D u0 = tr1 + v0 * tn1, u1 = tr1 + v1 * tn1;
    G4VSolid* sect1 = new G4Trd("sect1", -u0.x() / 2, (trt0 - u1).x() / 2,  2000, 2000, trt0.y() / 2);
    G4IntersectionSolid* sector =  new G4IntersectionSolid("sector", sect0, sect1,
                                                           G4Translate3D(r0)*G4RotateZ3D(M_PI / 144)*G4Translate3D(u0.x() / 2, trt0.y() / 2, (2288 - 1219.) / 2)*G4RotateX3D(-M_PI / 2));
    sectorlogical = new G4LogicalVolume(sector, Materials::get("G4_AIR"), "ECLBarrelSectorLogical", 0, 0, 0);
    sectorlogical->SetVisAttributes(att("air"));
    for (int i = 0; i < nseg; i++) {
      double phi = i * M_PI / 36 - M_PI / 2;
      new G4PVPlacement(G4RotateZ3D(phi), sectorlogical, suf("ECLBarrelSectorPhysical", i), top, false, i, overlap);
    }
  }

  // phi septum wall
  if (b_septum_walls) {
    double zh = (0.5 - 0.001) / 2;
    G4ThreeVector psw[] = {G4ThreeVector(-1000.8, 0, -zh), G4ThreeVector(1982.6, 0, -zh), G4ThreeVector(2288, 198.2, -zh),
                           G4ThreeVector(2288, 404, -zh), G4ThreeVector(-1219., 404, -zh), G4ThreeVector(-1219., 404 - 112.6, -zh),
                           G4ThreeVector(-1000.8, 0, zh), G4ThreeVector(1982.6, 0, zh), G4ThreeVector(2288, 198.2, zh),
                           G4ThreeVector(2288, 404, zh), G4ThreeVector(-1219., 404, zh), G4ThreeVector(-1219., 404 - 112.6, zh)
                          };
    G4VSolid* pswsolid = new BelleCrystal("pswsolid", 6, psw);
    G4LogicalVolume* pswlogical = new G4LogicalVolume(pswsolid, Materials::get("A5052"), "pswlogical", 0, 0, 0);
    pswlogical->SetVisAttributes(att("alum"));
    new G4PVPlacement(G4RotateZ3D(2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6 + 0.01,
                      0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4RotateY3D(-M_PI / 2), pswlogical, "phiwallphysical", sectorlogical, false, 0, overlap);
  }

  if (b_forward_support_ring) {
    zr_t vc4[] = {{0, 1449.2}, {0, 1582}, {90, 1582}, {90, 1582 - 75.7}};
    std::vector<zr_t> contour4(vc4, vc4 + 4);
    G4VSolid* part4solid = new BelleLathe("part4solid", phi0, dphi, contour4);
    G4LogicalVolume* part4logical = new G4LogicalVolume(part4solid, Materials::get("SUS304"), "part4logical", 0, 0, 0);
    part4logical->SetVisAttributes(att("iron"));
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, 2290), part4logical, "part4physical", top, false, 0, overlap);
  }

  if (b_backward_support_ring) {
    zr_t vc6[] = {{0, 1543}, { -105, 1543}, { -105, 1543 + 84}, {0, 1543 + 84}};
    std::vector<zr_t> contour6(vc6, vc6 + 4);
    G4VSolid* part6solid = new BelleLathe("part6solid", phi0, dphi, contour6);
    G4LogicalVolume* part6logical = new G4LogicalVolume(part6solid, Materials::get("SUS304"), "part6logical", 0, 0, 0);
    part6logical->SetVisAttributes(att("iron"));
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, -1225), part6logical, "part6physical", top, false, 0, overlap);
  }

  if (b_forward_support_wall) {
    zr_t vc2[] = {{0, 1180.1}, {0, 1249.5}, {54.3, 1249.5}, {445.8, 1503.6}, {445.8 + 4 * sind(32.982), 1503.6 - 4 * cosd(32.982)}, {54.3 + 4 * sind(32.982 / 2), 1249.5 - 4}, {4, 1249.5 - 4}, {4, 1180.1}};
    std::vector<zr_t> contour2(vc2, vc2 + 8);
    G4VSolid* part2solid = new BelleLathe("part2solid", phi0, dphi, contour2);
    G4LogicalVolume* part2logical = new G4LogicalVolume(part2solid, Materials::get("A5083"), "part2logical", 0, 0, 0);
    part2logical->SetVisAttributes(att("alum"));
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, 1930), part2logical, "part2physical", top, false, 0, overlap);
  }

  if (b_backward_support_wall) {
    zr_t vc3[] = {{0, 1180.1}, { -4, 1180.1}, { -4, 1249.5 - 4}, { -61.9 - 4 * sind(52.897 / 2), 1249.5 - 4}, { -285, 1539.1 - 4 * sind(52.897 / 2)}, { -285, 1618.8}, { -285 + 4, 1618.8}, { -285 + 4, 1539.1}, { -61.9, 1249.5}, {0, 1249.5}};
    std::vector<zr_t> contour3(vc3, vc3 + 10);
    G4VSolid* part3solid = new BelleLathe("part3solid", phi0, dphi, contour3);
    G4LogicalVolume* part3logical = new G4LogicalVolume(part3solid, Materials::get("A5083"), "part3logical", 0, 0, 0);
    part3logical->SetVisAttributes(att("alum"));
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, -940), part3logical, "part3physical", top, false, 0, overlap);
  }

  if (b_inner_support_wall) {
    G4VSolid* part1solid = new G4Tubs("part1solid", 1250.1, 1250.1 + 1.5, (988.5 + 1972.5) / 2, phi0, dphi);
    G4LogicalVolume* part1logical = new G4LogicalVolume(part1solid, Materials::get("A5083"), "part1logical", 0, 0, 0);
    part1logical->SetVisAttributes(att("alum2"));
    new G4PVPlacement(NULL, G4ThreeVector(0, 0, (-988.5 + 1972.5) / 2), part1logical, "part1physical", top, false, 0, overlap);
  }

  if (b_support_ribs || b_outer_plates || b_forward_part5) {
    //    double zh = (1219.+2288.)/2, dz = (-1219.+2288.)/2;
    double zh = (1330. + 2380.) / 2, dz = (-1330. + 2380.) / 2;
    if (b_support_ribs) {
      G4ThreeVector vc9[] = {G4ThreeVector(0, 0, -zh), G4ThreeVector(25, 0, -zh), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), -zh), G4ThreeVector(0, 24.5, -zh),
                             G4ThreeVector(0, 0, zh), G4ThreeVector(25, 0, zh), G4ThreeVector(25, 24.5 - 25.0 * tand(2.5), zh), G4ThreeVector(0, 24.5, zh)
                            };
      G4VSolid* part9solid = new BelleCrystal("part9solid", 4, vc9);
      G4LogicalVolume* part9logical = new G4LogicalVolume(part9solid, Materials::get("SUS304"), "part9logical", 0, 0, 0);
      part9logical->SetVisAttributes(att("silv"));
      G4Transform3D tpart9a = G4RotateZ3D(2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                              0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4Translate3D(0.5 / 2, 404 - 25 - 3.7 + 0.2, dz);
      G4Transform3D tpart9b = G4RotateZ3D(-2.5 / 180 * M_PI) * G4Translate3D(0, 1251.6,
                              0) * G4RotateZ3D(1.25 / 180 * M_PI) * G4Translate3D(-0.5 / 2, 404 - 25 - 3.7 + 0.5, dz) * G4RotateY3D(M_PI);
      new G4PVPlacement(tpart9a, part9logical, "part9aphysical", sectorlogical, false, 0, overlap);
      new G4PVPlacement(tpart9b, part9logical, "part9bphysical", sectorlogical, false, 0, overlap);
    }

    if (b_outer_plates) {
      G4VSolid* part10solid = new G4Box("part10solid", 143. / 2, 8. / 2, zh);
      G4LogicalVolume* part10logical = new G4LogicalVolume(part10solid, Materials::get("SUS304"), "part10logical", 0, 0, 0);
      part10logical->SetVisAttributes(att("iron2"));
      G4Transform3D tpart10 = G4Translate3D(0, 1251.6, 0) * G4Translate3D(-404 * tand(1.25), 404 - 1.1,
                              dz) * G4RotateZ3D(0.3 / 180 * M_PI);
      new G4PVPlacement(tpart10, part10logical, "part10physical", sectorlogical, false, 0, overlap);
    }

    // G4VSolid *part5solid = new G4Box("part5solid", 60./2, 45./2, 90./2);
    // G4LogicalVolume* part5logical = new G4LogicalVolume(part5solid, Materials::get("SUS304"), "part5logical", 0, 0, 0);
    // part5logical->SetVisAttributes(att("iron2"));
    // new G4PVPlacement(G4RotateZ3D(2.796/180*M_PI)*G4Translate3D(0,1582+45./2,2380.-90./2), part5logical, "part5physical", sectorlogical, false, 1, overlap);


    if (b_forward_part5) { // slice the element to fit into the sector
      double R = 1251.6, phi1 = phi0 + 1.25 * M_PI / 180, phi2 = (2.796 + 90 - 5) / 180 * M_PI;
      G4Vector3D n0(cos(phi0), sin(phi0), 0), n1(cos(phi1), sin(phi1), 0), n2(cos(phi2), sin(phi2), 0);
      // shift r0 by half of septum wall thickness so from one side of the sector the wall will be completely inside the sector volume
      G4Point3D r0 = R * n0 + 0.5 / 2 * (G4RotateZ3D(M_PI / 2) * n0);

      //      n2*(r0 + t*n1 - n2*1582) = 0;
      double t0 = -n2 * (r0 - n2 * 1582) / (n2 * n1);
      double t1 = -n2 * (r0 - n2 * (1582 + 45)) / (n2 * n1);

      G4Point3D x0 = r0 + t0 * n1;
      G4Point3D x1 = (t1 - t0) * n1;

      Point_t c5a[] = {{0, 0}, {30 * cosd(5 - 2.796), -30 * sind(5 - 2.796)}, {30 * cosd(5 - 2.796) + 45 * cos(phi2), -30 * sind(5 - 2.796) + 45 * sin(phi2)}, {x1.x(), x1.y()}};
      G4ThreeVector vc5a[8];
      for (int i = 0; i < 4; i++) vc5a[i] = G4ThreeVector(c5a[i].x, c5a[i].y, -90. / 2);
      for (int i = 0; i < 4; i++) vc5a[i + 4] = G4ThreeVector(c5a[i].x, c5a[i].y, 90. / 2);
      G4VSolid* part5asolid = new BelleCrystal("part5asolid", 4, vc5a);
      G4LogicalVolume* part5alogical = new G4LogicalVolume(part5asolid, Materials::get("SUS304"), "part5alogical", 0, 0, 0);
      part5alogical->SetVisAttributes(att("iron2"));
      new G4PVPlacement(G4RotateZ3D(2 * M_PI / 72)*G4Translate3D(x0.x(), x0.y(), 2380. - 90. / 2), part5alogical, "part5aphysical",
                        sectorlogical, false, 0, overlap);

      Point_t c5b[] = {{ -30 * cosd(5 - 2.796), 30 * sind(5 - 2.796)}, {0, 0}, {x1.x(), x1.y()}, { -30 * cosd(5 - 2.796) + 44.8 * cos(phi2), 30 * sind(5 - 2.796) + 44.8 * sin(phi2)}};
      G4ThreeVector vc5b[8];
      for (int i = 0; i < 4; i++) vc5b[i] = G4ThreeVector(c5b[i].x, c5b[i].y, -90. / 2);
      for (int i = 0; i < 4; i++) vc5b[i + 4] = G4ThreeVector(c5b[i].x, c5b[i].y, 90. / 2);
      G4VSolid* part5bsolid = new BelleCrystal("part5bsolid", 4, vc5b);
      G4LogicalVolume* part5blogical = new G4LogicalVolume(part5bsolid, Materials::get("SUS304"), "part5blogical", 0, 0, 0);
      part5blogical->SetVisAttributes(att("iron2"));
      new G4PVPlacement(G4Translate3D(x0.x(), x0.y(), 2380. - 90. / 2), part5blogical, "part5bphysical", sectorlogical, false, 0,
                        overlap);
    }

  }

  if (b_forward_support_legs) {
    G4ThreeVector vc11a[] = {G4ThreeVector(0, 0, -63. / 2), G4ThreeVector(35, 24.5, -63. / 2), G4ThreeVector(35, 158 - 35, -63. / 2), G4ThreeVector(0, 158 - 35, -63. / 2),
                             G4ThreeVector(0, 0, 63. / 2), G4ThreeVector(35, 24.5, 63. / 2), G4ThreeVector(35, 158 - 35, 63. / 2), G4ThreeVector(0, 158 - 35, 63. / 2)
                            };
    G4VSolid* part11asolid = new BelleCrystal("part11asolid", 4, vc11a);
    G4VSolid* part11bsolid = new G4Box("part11bsolid", 63. / 2, 35. / 2, 100. / 2);
    G4LogicalVolume* part11alogical = new G4LogicalVolume(part11asolid, Materials::get("SUS304"), "part11alogical", 0, 0, 0);
    part11alogical->SetVisAttributes(att("iron2"));
    G4LogicalVolume* part11blogical = new G4LogicalVolume(part11bsolid, Materials::get("SUS304"), "part11blogical", 0, 0, 0);
    part11blogical->SetVisAttributes(att("iron"));

    double Ro = 158.4;
    zr_t vsf[] = {{0, 0}, {35, 24.5}, {35, 158 - 35}, {100, 158 - 35}, {100, Ro}, {0, Ro}};
    for (zr_t* it = vsf; it != vsf + 6; it++) {it->r += 1668 - 158;}
    std::vector<zr_t> csf(vsf, vsf + 6);
    G4VSolid* sf = new BelleLathe("sf", 0, 2 * M_PI, csf);
    G4LogicalVolume* sfl = new G4LogicalVolume(sf, Materials::get("G4_AIR"), "supportfwd", 0, 0, 0);
    sfl->SetVisAttributes(att("air"));
    new G4PVPlacement(G4Translate3D(0, 0, 2380), sfl, "supportfwdphysical", top, false, 0, overlap);

    G4VSolid* sfs = new BelleLathe("sfs", -M_PI / 72, 2 * M_PI / 72, csf);
    G4LogicalVolume* sfsl = new G4LogicalVolume(sfs, Materials::get("G4_AIR"), "supportfwdsector", 0, 0, 0);
    sfsl->SetVisAttributes(att("air"));

    new G4PVPlacement(G4Translate3D(1668 - 158, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4RotateY3D(-M_PI / 2), part11alogical, "part11aphysical",
                      sfsl, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(1668 - 158, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (158. - 35.) + 35. / 2, 100. / 2),
                      part11blogical, "part11bphysical", sfsl, false, 0, overlap);
    new G4PVReplica("supportfwdrep", sfsl, sfl, kPhi, nseg, 2 * M_PI / 72, (2.796 - 2.5)*M_PI / 180);
  }

  if (b_backward_support_legs) {
    G4VSolid* part12asolid = new G4Box("part12asolid", 90. / 2, (118. - 35.) / 2, 35. / 2);
    G4VSolid* part12bsolid = new G4Box("part12bsolid", 90. / 2, 35. / 2, 120. / 2);
    G4LogicalVolume* part12alogical = new G4LogicalVolume(part12asolid, Materials::get("SUS304"), "part12alogical", 0, 0, 0);
    part12alogical->SetVisAttributes(att("iron2"));
    G4LogicalVolume* part12blogical = new G4LogicalVolume(part12bsolid, Materials::get("SUS304"), "part12blogical", 0, 0, 0);
    part12blogical->SetVisAttributes(att("iron"));

    double Rb = 118.7;
    zr_t vsb[] = {{0, 0}, {0, Rb}, { -120, Rb}, { -120, 118 - 35}, { -35, 118 - 35}, { -35, 0}};
    for (zr_t* it = vsb; it != vsb + 6; it++) {it->r += 1668 - 118;}
    std::vector<zr_t> csb(vsb, vsb + 6);
    G4VSolid* sb = new BelleLathe("sb", 0, 2 * M_PI, csb);
    G4LogicalVolume* sbl = new G4LogicalVolume(sb, Materials::get("G4_AIR"), "supportbkw", 0, 0, 0);
    sbl->SetVisAttributes(att("air"));
    new G4PVPlacement(G4Translate3D(0, 0, -1330), sbl, "supportbkwphysical", top, false, 0, overlap);

    G4VSolid* sbs = new BelleLathe("sbs", -M_PI / 72, 2 * M_PI / 72, csb);
    G4LogicalVolume* sbsl = new G4LogicalVolume(sbs, Materials::get("G4_AIR"), "supportbkwsector", 0, 0, 0);
    sbsl->SetVisAttributes(att("air"));

    new G4PVPlacement(G4Translate3D(1668 - 118, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (118. - 35.) / 2, -35. / 2),
                      part12alogical, "part12aphysical", sbsl, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(1668 - 118, 0, 0)*G4RotateZ3D(-M_PI / 2)*G4Translate3D(0, (118. - 35.) + 35. / 2, -120. / 2),
                      part12blogical, "part12bphysical", sbsl, false, 0, overlap);

    new G4PVReplica("supportbkwrep", sbsl, sbl, kPhi, nseg, 2 * M_PI / 72, (2.796 - 2.5)*M_PI / 180);
  }

  if (b_septum_walls) {
    struct zwall_t { double zlow, zup;};
    zwall_t zs[] = {{ -682.1, -883.9}, { -445.9, -571.7}, { -220.8, -274.2}, {0, 0}, {220.8, 274.2}, {445.9, 571.7}, {682.1, 883.9}, {936.6, 1220.5}, {1217.1, 1591.2}, {1532., 2007.4}};

    const double dz = 0.5 / 2; // half of theta fin thickness

    G4Vector3D n0(sind(90 + 2.5 + 1.25), -cosd(90 + 2.5 + 1.25), 0); // normal vector of the left sector wall pointing inside the sector
    G4Point3D r0 = G4Point3D(-1251.6 * sind(2.5), 0, 0) + dz * n0; // point in the left sector wall shifted by phi fin
    G4Vector3D n1(-sind(90 - 2.5 + 1.25), cosd(90 - 2.5 + 1.25),
                  0); // normal vector of the right sector wall pointing inside the sector
    G4Point3D r1 = G4Point3D(1251.6 * sind(2.5), 0, 0) + dz * n1; // point in the right sector wall shifted by phi fin

    G4Vector3D n2(0, 1, 0); G4Point3D r2(0, 0, 0), r3(0, 310, 0);

    for (unsigned int i = 0; i < sizeof(zs) / sizeof(zwall_t); i++) {
      double th = atan2(404, -(zs[i].zlow - zs[i].zup));

      G4Vector3D nf(0, -cos(th), sin(th));
      // take into account finite thickness of the theta fin
      G4Point3D rf(0, 0, copysign(dz / sin(th), cos(th)));

      // calculate three plane intersection point
      auto inter = [](const G4Vector3D & l_n0, const G4Point3D & l_r0,
                      const G4Vector3D & l_n1, const G4Point3D & l_r1,
      const G4Vector3D & l_n2, const G4Point3D & l_r2) -> G4Point3D {
        CLHEP::HepMatrix A(3, 3);
        CLHEP::HepVector B(3);
        A[0][0] = l_n0.x(), A[0][1] = l_n0.y(), A[0][2] = l_n0.z();
        A[1][0] = l_n1.x(), A[1][1] = l_n1.y(), A[1][2] = l_n1.z();
        A[2][0] = l_n2.x(), A[2][1] = l_n2.y(), A[2][2] = l_n2.z();

        B[0] = l_r0 * l_n0;
        B[1] = l_r1 * l_n1;
        B[2] = l_r2 * l_n2;

        CLHEP::HepVector r = A.inverse() * B;
        G4Point3D res(r[0], r[1], r[2]);
        return  res;
      };

      G4Transform3D tfin(G4RotateX3D(-th + M_PI / 2)), tfini(G4RotateX3D(th - M_PI / 2));
      G4ThreeVector t0 = tfini * inter(n0, r0, n2, r2, nf, rf);
      G4ThreeVector t1 = tfini * inter(n1, r1, n2, r2, nf, rf);
      G4ThreeVector t2 = tfini * inter(n0, r0, n2, r3, nf, rf);
      G4ThreeVector t3 = tfini * inter(n1, r1, n2, r3, nf, rf);
      //    G4cout<<t0<<" "<<t1<<" "<<t2<<" "<<t3<<" "<<G4RotateX3D(th-M_PI/2)*G4Point3D(0,404,zs[i].zup-zs[i].zlow)<<G4endl;

      G4ThreeVector thfin[8];
      thfin[0] = G4ThreeVector(t0.x(), t0.y(), -dz);
      thfin[1] = G4ThreeVector(t1.x(), t1.y(), -dz);
      thfin[2] = G4ThreeVector(t3.x(), t3.y(), -dz);
      thfin[3] = G4ThreeVector(t2.x(), t2.y(), -dz);
      thfin[4] = G4ThreeVector(t0.x(), t0.y(), +dz);
      thfin[5] = G4ThreeVector(t1.x(), t1.y(), +dz);
      thfin[6] = G4ThreeVector(t3.x(), t3.y(), +dz);
      thfin[7] = G4ThreeVector(t2.x(), t2.y(), +dz);

      G4VSolid* thfinsolid = new BelleCrystal("thfinsolid", 4, thfin);
      G4LogicalVolume* thfinlogical = new G4LogicalVolume(thfinsolid, Materials::get("A5052"), "thfinlogical", 0, 0, 0);
      thfinlogical->SetVisAttributes(att("iron2"));
      new G4PVPlacement(G4Translate3D(0, 1251.6, zs[i].zlow)*tfin, thfinlogical, "thfinphysical", sectorlogical, false, 0, overlap);
    }
  }

  if (b_pipe) {
    double zh = (1219. + 2288.) / 2, dz = (-1219. + 2288.) / 2;
    G4VSolid* tubec = new G4Tubs("tubec", 0, 12. / 2, zh, 0, 2 * M_PI);
    G4VSolid* tubew = new G4Tubs("tubew", 0, 10. / 2, zh, 0, 2 * M_PI);
    G4LogicalVolume* tubeclogical = new G4LogicalVolume(tubec, Materials::get("C1220"), "tubec", 0, 0, 0);
    G4LogicalVolume* tubewlogical = new G4LogicalVolume(tubew, Materials::get("G4_WATER"), "tubew", 0, 0, 0);
    new G4PVPlacement(G4Translate3D(0, 0, 0), tubewlogical, "tubewphysical", tubeclogical, false, 1, overlap);
    tubeclogical->SetVisAttributes(att("iron2"));
    new G4PVPlacement(G4Translate3D(25, 1640, dz), tubeclogical, "tubecphysical", sectorlogical, false, 0, overlap);
    new G4PVPlacement(G4Translate3D(-35, 1640, dz), tubeclogical, "tubecphysical", sectorlogical, false, 1, overlap);
  }

  //  vector<cplacement_t> bp = load_placements("/ecl/data/crystal_placement_barrel.dat");
  vector<cplacement_t> bp = load_placements(m_sap, ECLParts::barrel);

  if (b_preamplifier) {
    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      G4Transform3D twc = get_transform(*it);
      int indx = it - bp.begin();
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.250 + get_pa_box_height() / 2), get_preamp(),
                                  suf("phys_barrel_preamplifier", indx),
                                  sectorlogical, false, 72 + 9 * (indx / 2) + (indx % 2), 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_crystal_holder) {
    G4VSolid* holder0 = new G4Box("holder0", 19. / 2, 38. / 2, 6. / 2);
    G4VSolid* holder1 = new G4Box("holder1", 7. / 2, 26. / 2, 7. / 2);
    G4VSolid* holder = new G4SubtractionSolid("holder", holder0, holder1);
    G4LogicalVolume* holderlogical = new G4LogicalVolume(holder, Materials::get("A5052"), "holderlogical", 0, 0, 0);
    holderlogical->SetVisAttributes(att("holder"));
    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      G4Transform3D twc = get_transform(*it);
      int indx = it - bp.begin();
      auto pv = new G4PVPlacement(twc * G4TranslateZ3D(300 / 2 + 0.250 + get_pa_box_height() + 38. / 2)*G4RotateZ3D(M_PI / 2)*G4RotateX3D(
                                    M_PI / 2), holderlogical, "holderphysical", sectorlogical, false, indx, 0);
      if (overlap)pv->CheckOverlaps(1000);
    }
  }

  if (b_crystals) {
    //    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_barrel.dat");
    vector<shape_t*> cryst = load_shapes(m_sap, ECLParts::barrel);
    vector<G4LogicalVolume*> wrapped_crystals;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      wrapped_crystals.push_back(wrapped_crystal(s, "barrel", 0.17 - 0.0325));
    }

    for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
      const cplacement_t& t = *it;
      auto s = find_if(cryst.begin(), cryst.end(), [&t](const shape_t* shape) {return shape->nshape == t.nshape;});
      if (s == cryst.end()) continue;

      G4Transform3D twc = get_transform(t);
      int indx = it - bp.begin();
      new G4PVPlacement(twc, wrapped_crystals[s - cryst.begin()], suf("ECLBarrelWrappedCrystal_Physical", indx), sectorlogical, false,
                        72 + 9 * (indx / 2) + (indx % 2), overlap);
    }
  }

  if (b_crystal_support) {
    G4LogicalVolume* tl[8];
    for (int i = 0; i < 8; i++) {
      G4VSolid* t = get_crystal_support(i);
      tl[i] = new G4LogicalVolume(t, Materials::get("SUS304"), "crystal_support_logical", 0, 0, 0);
      tl[i]->SetVisAttributes(att("iron2"));
    }

    double offset[] = { -945, -607, -305, -45, 45, 305, 607, 909, 1240, 1628, 2040};
    int ns[] = {8, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1};
    double offsetb[] = {263.4 - 210 - 30, 27.4, 10.6, 16.1, 26.2, 21.7, 35, 22 + 240};
    bool flip[] = {1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
    for (int i = 0; i < 11; i++) {
      G4Transform3D tflip = flip[i] ? G4RotateY3D(-M_PI / 2) : G4RotateY3D(M_PI / 2);
      G4Transform3D tr0(G4RotateZ3D(2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6,
                        0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4Translate3D(25 + 3. / 2 + 0.5 / 2, 404 - 25 - 3.7 + 24.5 - 25.0 * tand(2.5) - 0.5,
                            offset[i])*tflip * G4Translate3D(-offsetb[ns[i] - 1], 0, 0)*G4RotateX3D(M_PI));
      new G4PVPlacement(tr0, tl[ns[i] - 1], "crystal_support_physical", sectorlogical, false, 0, overlap);
      G4Transform3D tr1(G4RotateZ3D(-2.5 / 180 * M_PI)*G4Translate3D(0, 1251.6,
                        0)*G4RotateZ3D(1.25 / 180 * M_PI)*G4Translate3D(-25 - 3. / 2 - 0.5 / 2, 404 - 25 - 3.7 + 24.5 - 25.0 * tand(2.5) - 0.5,
                            offset[i])*tflip * G4Translate3D(-offsetb[ns[i] - 1], 0, 0)*G4RotateX3D(M_PI));
      new G4PVPlacement(tr1, tl[ns[i] - 1], "crystal_support_physical", sectorlogical, false, 1, overlap);
    }
  }

}
