#include <ecl/geometry/BelleCrystal.h>
#include <ecl/geometry/BelleLathe.h>
#include <gtest/gtest.h>

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

  /** Test the only method. */
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

    // Real vectors
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

    // Real vectors
    G4ThreeVector pMin_real(-s1, -c2, -z); G4ThreeVector pMax_real(s1, 1, z);

    EXPECT_DOUBLE_EQ(pMin_real.x(), pMin.x());
    EXPECT_DOUBLE_EQ(pMin_real.y(), pMin.y());
    EXPECT_DOUBLE_EQ(pMin_real.z(), pMin.z());

    EXPECT_DOUBLE_EQ(pMax_real.x(), pMax.x());
    EXPECT_DOUBLE_EQ(pMax_real.y(), pMax.y());
    EXPECT_DOUBLE_EQ(pMax_real.z(), pMax.z());
  }

  // Bounding limits for half circle starting pi/2
  TEST_F(BelleLatheTest, BoundingBoxSect0)
  {
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI / 2, M_PI, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Real vectors
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(0, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for half circle starting 3pi/2
  TEST_F(BelleLatheTest, BoundingBoxSect1)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", 3 * M_PI / 2, M_PI, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Real vectors
    G4ThreeVector pMin_real(0, -1, -1);
    G4ThreeVector pMax_real(1, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for quarter starting pi
  TEST_F(BelleLatheTest, BoundingBoxSect2)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI, M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Real vectors
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(0, 0, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for three quarter starting 0
  TEST_F(BelleLatheTest, BoundingBoxSect3)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0}, {1, 0}, {1, 1}, { -1, 1}}; //z, r
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", 0, 3 * M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Real vectors
    G4ThreeVector pMin_real(-1, -1, -1);
    G4ThreeVector pMax_real(1, 1, 1);

    EXPECT_NEAR(pMin_real.x(), pMin.x(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.y(), pMin.y(), absolute_tolerance);
    EXPECT_NEAR(pMin_real.z(), pMin.z(), absolute_tolerance);

    EXPECT_NEAR(pMax_real.x(), pMax.x(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.y(), pMax.y(), absolute_tolerance);
    EXPECT_NEAR(pMax_real.z(), pMax.z(), absolute_tolerance);
  }

  // Bouding limits for quarter starting pi/4 w/ hole
  TEST_F(BelleLatheTest, BoundingBoxSect4)
  {
    // Create the Belle Lathe
    zr_t bint[] = {{ -1, 0.5}, {1, 0.5}, {1, 1}, { -1, 1}}; //z, r, rmin is 0.5
    std::vector<zr_t> contourb(bint, bint + sizeof(bint) / sizeof(zr_t));
    BelleLathe* sect = new BelleLathe("sect", M_PI / 4, M_PI / 2, contourb);

    // Get bounding box
    G4ThreeVector pMin; G4ThreeVector pMax;
    sect->BoundingLimits(pMin, pMax);

    // Real vectors
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
