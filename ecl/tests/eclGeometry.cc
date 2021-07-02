/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/geometry/BelleCrystal.h>
#include <ecl/geometry/BelleLathe.h>
#include <gtest/gtest.h>
#include "ecl/geometry/shapes.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

namespace {

  /** Declare BelleCrystal test */
  class BelleCrystalTest : public ::testing::Test {};

  /** Declare BelleLathe test */
  class BelleLatheTest : public ::testing::Test {
  protected:
    double absolute_tolerance = 0.00001;
  };

  TEST_F(BelleCrystalTest, BoundingBoxFourSides)
  {
    // Create box size 2 2 2
    G4ThreeVector vertices[] = {G4ThreeVector(-1, -1, -1), G4ThreeVector(1, -1, -1), G4ThreeVector(1, 1, -1), G4ThreeVector(-1, 1, -1),
                                G4ThreeVector(-1, -1, 1), G4ThreeVector(1, -1, 1), G4ThreeVector(1, 1, 1), G4ThreeVector(-1, 1, 1)
                               };
    BelleCrystal* crystal = new BelleCrystal("solid4", 4, vertices);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    crystal->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(-1, -1, -1); G4ThreeVector pMax_real(1, 1, 1);

    EXPECT_DOUBLE_EQ(pMin_real.x(), pMin.x());
    EXPECT_DOUBLE_EQ(pMin_real.y(), pMin.y());
    EXPECT_DOUBLE_EQ(pMin_real.z(), pMin.z());

    EXPECT_DOUBLE_EQ(pMax_real.x(), pMax.x());
    EXPECT_DOUBLE_EQ(pMax_real.y(), pMax.y());
    EXPECT_DOUBLE_EQ(pMax_real.z(), pMax.z());
  }

  TEST_F(BelleCrystalTest, BoundingBoxFiveSides)
  {
    // Create pentagon side
    double z = 1;
    double c1 = 0.25 * (sqrt(5) - 1), c2 = 0.25 * (sqrt(5) + 1);
    double s1 = 0.25 * sqrt(10 + 2 * sqrt(5)), s2 = 0.25 * sqrt(10 - 2 * sqrt(5));
    G4ThreeVector vertices[] = {G4ThreeVector(0, 1, -z), G4ThreeVector(-s1, c1, -z), G4ThreeVector(-s2, -c2, -z), G4ThreeVector(s2, -c2, -z), G4ThreeVector(s1, c1, -z),
                                G4ThreeVector(0, 1, z), G4ThreeVector(-s1, c1, z), G4ThreeVector(-s2, -c2, z), G4ThreeVector(s2, -c2, z), G4ThreeVector(s1, c1, z)
                               };
    BelleCrystal* crystal = new BelleCrystal("solid5", 5, vertices);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    crystal->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(-s1, -c2, -z); G4ThreeVector pMax_real(s1, 1, z);

    EXPECT_DOUBLE_EQ(pMin_real.x(), pMin.x());
    EXPECT_DOUBLE_EQ(pMin_real.y(), pMin.y());
    EXPECT_DOUBLE_EQ(pMin_real.z(), pMin.z());

    EXPECT_DOUBLE_EQ(pMax_real.x(), pMax.x());
    EXPECT_DOUBLE_EQ(pMax_real.y(), pMax.y());
    EXPECT_DOUBLE_EQ(pMax_real.z(), pMax.z());
  }

  TEST_F(BelleCrystalTest, PentagonSurface)
  {
    // Create pentagon side
    double z = 1;
    double c1 = 0.25 * (sqrt(5) - 1), c2 = 0.25 * (sqrt(5) + 1);
    double s1 = 0.25 * sqrt(10 + 2 * sqrt(5)), s2 = 0.25 * sqrt(10 - 2 * sqrt(5));
    G4ThreeVector p0m(0, 1, -z), p1m(-s1, c1, -z), p2m(-s2, -c2, -z), p3m(s2, -c2, -z), p4m(s1, c1, -z);
    G4ThreeVector p0p(0, 1, z), p1p(-s1, c1, z), p2p(-s2, -c2, z), p3p(s2, -c2, z), p4p(s1, c1, z);
    G4ThreeVector vertices[] = {p0m, p1m, p2m, p3m, p4m, p0p, p1p, p2p, p3p, p4p};
    BelleCrystal* c = new BelleCrystal("solid5", 5, vertices);

    for (int i = 0; i < 1000 * 100; i++) {
      G4ThreeVector p = c->GetPointOnSurface();
      EXPECT_EQ(c->Inside(p), kSurface);
    }
    delete c;
  }

  TEST_F(BelleCrystalTest, BarrelSurfaces)
  {
    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_barrel.dat");
    double wrapthickness = 0.17;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      std::string prefix("sv_"); prefix += "barrel"; prefix += "_wrap";
      G4Translate3D tw;
      G4VSolid* c = s->get_solid(prefix, wrapthickness, tw);
      for (int i = 0; i < 1000 * 100; i++) {
        G4ThreeVector p = c->GetPointOnSurface();
        EInside I = c->Inside(p);
        EXPECT_EQ(I, kSurface);
      }
      delete c;
    }
    for (auto it = cryst.begin(); it != cryst.end(); it++) delete *it;
  }

  TEST_F(BelleCrystalTest, ForwardSurfaces)
  {
    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_forward.dat");
    double wrapthickness = 0.17;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      std::string prefix("sv_"); prefix += "barrel"; prefix += "_wrap";
      G4Translate3D tw;
      G4VSolid* c = s->get_solid(prefix, wrapthickness, tw);
      for (int i = 0; i < 1000 * 100; i++) {
        G4ThreeVector p = c->GetPointOnSurface();
        EInside I = c->Inside(p);
        EXPECT_EQ(I, kSurface);
      }
      delete c;
    }
    for (auto it = cryst.begin(); it != cryst.end(); it++) delete *it;
  }

  TEST_F(BelleCrystalTest, BackwardSurfaces)
  {
    vector<shape_t*> cryst = load_shapes("/ecl/data/crystal_shape_backward.dat");
    double wrapthickness = 0.17;
    for (auto it = cryst.begin(); it != cryst.end(); it++) {
      shape_t* s = *it;
      std::string prefix("sv_"); prefix += "barrel"; prefix += "_wrap";
      G4Translate3D tw;
      G4VSolid* c = s->get_solid(prefix, wrapthickness, tw);
      for (int i = 0; i < 1000 * 100; i++) {
        G4ThreeVector p = c->GetPointOnSurface();
        EInside I = c->Inside(p);
        EXPECT_EQ(I, kSurface);
      }
      delete c;
    }
    for (auto it = cryst.begin(); it != cryst.end(); it++) delete *it;
  }

  // Bounding limits for half circle starting at pi/2
  TEST_F(BelleLatheTest, BoundingBoxSect0)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI / 2, M_PI, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(0, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for half circle starting at 3pi/2
  TEST_F(BelleLatheTest, BoundingBoxSect1)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", 3 * M_PI / 2, M_PI, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(0, -1, -1);
    G4ThreeVector pMax_real(1, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for quarter starting at pi
  TEST_F(BelleLatheTest, BoundingBoxSect2)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI, M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(0, 0, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for three quarter starting at 0
  TEST_F(BelleLatheTest, BoundingBoxSect3)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", 0, 3 * M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(1, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for quarter starting at pi/4 w/ hole
  TEST_F(BelleLatheTest, BoundingBoxSect4)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0.5}, {1, 0.5}, {1, 1}, { -1, 1}}; //z, r, rmin is 0.5
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI / 4, M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Manually calculated bounding box
    G4ThreeVector pMin_real(cos(3 * M_PI / 4), 0.5 * sin(M_PI / 4), -1);
    G4ThreeVector pMax_real(cos(M_PI / 4), 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }
}
