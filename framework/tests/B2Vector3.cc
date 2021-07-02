/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/TestHelpers.h>
#include <framework/geometry/B2Vector3.h>
#include <TVector3.h>
#include <TError.h> // to shut up stupid messages

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;


namespace {
  struct PointRThetaPhi { double r; double theta; double phi; };
  struct PointXYZ { double x; double y; double z; };
  std::vector<PointRThetaPhi> test_vectors(const std::vector<double>& R = {1}, int nTheta = 3, int nPhi = 4, bool prune = true)
  {
    std::vector<PointRThetaPhi> result;
    auto get_theta = [nTheta](int i) -> double {
      if (nTheta < 2) return 0.;
      return M_PI / (nTheta - 1) * i;
    };
    auto get_phi = [nPhi](int i) -> double {
      if (nPhi < 2) return 0.;
      return 2 * M_PI / nPhi * i;
    };

    for (double r : R) {
      for (int i = 0; i < nTheta || (nTheta < 2 && i == 0); ++i) {
        double theta = get_theta(i);
        for (int j = 0; j < nPhi || (nPhi < 2 && j == 0); ++j) {
          double phi = get_phi(j);
          result.push_back(PointRThetaPhi{r, theta, phi});
          // if theta is 0 or 180 degree phi does not have any effect except for zero signs, so disable those for now
          if (prune && (theta == 0 || theta == M_PI)) break;
        }
        // if r is 0, theta and phi don't have any effect except for zero signs, so disable those for now
        if (prune && r == 0) break;
      }
    }
    return result;
  }

  TEST(B2Vector3, DoubleGetters)
  {
    gErrorIgnoreLevel = kError;
    TVector3 tvec;
    B2Vector3D bvec;
    for (auto& rtp : test_vectors({0, 1, 1e20}, 64, 64)) {
      tvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);
      bvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);
      EXPECT_EQ(bvec, tvec);

      B2Vector3D bvec2(tvec);
      EXPECT_EQ(bvec2, bvec);
      TVector3 tvec2 = bvec;
      EXPECT_EQ(tvec2, tvec);

      EXPECT_DOUBLE_EQ(bvec.CosTheta(), tvec.CosTheta()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Mag(), tvec.Mag()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Mag2(), tvec.Mag2()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Perp(), tvec.Perp()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Perp2(), tvec.Perp2()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Phi(), tvec.Phi()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Pt(), tvec.Pt()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Px(), tvec.Px()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Py(), tvec.Py()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Pz(), tvec.Pz()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Theta(), tvec.Theta()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.x(), tvec.x()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.y(), tvec.y()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.z(), tvec.z()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.X(), tvec.X()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Y(), tvec.Y()) << bvec.PrintString();
      EXPECT_DOUBLE_EQ(bvec.Z(), tvec.Z()) << bvec.PrintString();
      //Since TVector3::Mag uses sqrt(x**2 + y**2 + z**2) instead of std::hypot
      //there are some imprecisions in TVector3 so just compare this with float
      //precision
      EXPECT_FLOAT_EQ(bvec.Eta(), tvec.Eta()) << bvec.PrintString();
      EXPECT_FLOAT_EQ(bvec.PseudoRapidity(), tvec.PseudoRapidity()) << bvec.PrintString();
    }
  }

  TEST(B2Vector3, DoubleGettersVector)
  {
    gErrorIgnoreLevel = kError;
    TVector3 tvec;
    B2Vector3D bvec;
    for (auto& rtp : test_vectors({0, 1, 1e20}, 16, 16)) {
      tvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);
      bvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);

      TVector3 tvec2;
      B2Vector3D bvec2;
      for (auto& rtp2 : test_vectors({0, 1, 1e20}, 16, 16)) {
        tvec2.SetMagThetaPhi(rtp2.r, rtp2.theta, rtp2.phi);
        bvec2.SetMagThetaPhi(rtp2.r, rtp2.theta, rtp2.phi);
        EXPECT_DOUBLE_EQ(tvec.Angle(tvec2), bvec.Angle(bvec2)) << bvec.PrintString() << ", " << bvec2.PrintString();
        EXPECT_DOUBLE_EQ(tvec.DeltaPhi(tvec2), bvec.DeltaPhi(bvec2)) << bvec.PrintString() << ", " << bvec2.PrintString();
        EXPECT_DOUBLE_EQ(tvec.Dot(tvec2), bvec.Dot(bvec2)) << bvec.PrintString() << ", " << bvec2.PrintString();
        EXPECT_DOUBLE_EQ(tvec.Perp(tvec2), bvec.Perp(bvec2)) << bvec.PrintString() << ", " << bvec2.PrintString();
        EXPECT_DOUBLE_EQ(tvec.Perp2(tvec2), bvec.Perp2(bvec2)) << bvec.PrintString() << ", " << bvec2.PrintString();
        //ROOT uses sqrt(x**2+y**2+z**2) for the magnitude, we use
        //std::hypot(std::hypot(x,y), z). This leads to imprecissions in ROOT
        //so we have to adapt
        EXPECT_NEAR(tvec.DeltaR(tvec2), bvec.DeltaR(bvec2), 1e-14) << bvec.PrintString() << ", " << bvec2.PrintString();
        EXPECT_NEAR(tvec.DrEtaPhi(tvec2), bvec.DrEtaPhi(bvec2), 1e-14) << bvec.PrintString() << ", " << bvec2.PrintString();
      }
    }
  }

  TEST(B2Vector3, Rotation)
  {
    gErrorIgnoreLevel = kError;
    TVector3 tvec;
    B2Vector3D bvec;

    // rotate \vec{e_x} by 90deg around \vec{e_z} to become \vec{e_y}
    bvec.SetXYZ(1., 0., 0.);
    tvec.SetXYZ(1., 0., 0.);
    bvec.Rotate(M_PI_2, B2Vector3D(0., 0., 1.));
    tvec.Rotate(M_PI_2, TVector3(0., 0., 1.));
    EXPECT_NEAR(bvec.x(), 0., 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.y(), 1., 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Phi(), tvec.Phi(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Theta(), tvec.Theta(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.x(), tvec.x(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.y(), tvec.y(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.z(), tvec.z(), 1e-14) << bvec.PrintString();

    // rotate \vec{e_x} by 180deg around \vec{e_z} to become -\vec{e_x}
    bvec.SetXYZ(1., 0., 0.);
    tvec.SetXYZ(1., 0., 0.);
    bvec.Rotate(M_PI, B2Vector3D(0., 0., 1.));
    tvec.Rotate(M_PI, TVector3(0., 0., 1.));
    EXPECT_NEAR(bvec.x(), -1., 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.y(), 0., 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Phi(), tvec.Phi(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Theta(), tvec.Theta(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.x(), tvec.x(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.y(), tvec.y(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.z(), tvec.z(), 1e-14) << bvec.PrintString();

    // just test a "random" case
    bvec.SetXYZ(4., 3., 2.);
    tvec.SetXYZ(4., 3., 2.);
    bvec.Rotate(M_PI / 2.5, B2Vector3D(1., 2., 3.));
    tvec.Rotate(M_PI / 2.5, TVector3(1., 2., 3.));
    EXPECT_NEAR(bvec.Mag(), tvec.Mag(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Mag2(), tvec.Mag2(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Perp(), tvec.Perp(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Perp2(), tvec.Perp2(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Phi(), tvec.Phi(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Theta(), tvec.Theta(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.X(), tvec.X(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Y(), tvec.Y(), 1e-14) << bvec.PrintString();
    EXPECT_NEAR(bvec.Z(), tvec.Z(), 1e-14) << bvec.PrintString();

    // rotate the test vectors all around the same axis by different angles
    const B2Vector3D baxis(4., 3., -2.);
    const TVector3   taxis(4., 3., -2.);

    for (auto& rtp : test_vectors({0, 1, 1e20}, 64, 64)) {

      double epsilon = (rtp.r < 1e10 ? 1e-10 : rtp.r * 1e-10);

      // just test rotations around baxis / taxis by different angles in 0...2 M_PI
      for (int i = 0; i < 28; i++) {
        double angle = 2.*M_PI / 28 * i;
        tvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);
        bvec.SetMagThetaPhi(rtp.r, rtp.theta, rtp.phi);
        bvec.Rotate(angle, baxis);
        tvec.Rotate(angle, taxis);

        // Check for the single values to be +-PI and differing by 2 PI, which means both of them are
        // basically equal, at least in terms of trigonometrical functions
        if (fabs(fabs(bvec.Phi() - tvec.Phi()) - 2.*M_PI) < 1e-14) {
          bvec.SetPhi(-bvec.Phi());
        }

        // Phi, Theta and CosTheta are not affected by large values of r, thus keep 1e-12
        EXPECT_NEAR(bvec.CosTheta(), tvec.CosTheta(), 1e-12) << bvec.PrintString();
        EXPECT_NEAR(bvec.Phi(), tvec.Phi(), 1e-12) << bvec.PrintString();
        EXPECT_NEAR(bvec.Theta(), tvec.Theta(), 1e-12) << bvec.PrintString();
//         EXPECT_NEAR(bvec.Mag(), tvec.Mag(), (bvec.Mag() < 10 ? 1e-12 : bvec.Mag() * 1e-12)) << bvec.PrintString();
//         EXPECT_NEAR(bvec.Mag2(), tvec.Mag2(), (bvec.Mag2() < 10 ? 1e-12 : bvec.Mag2() * 1e-12)) << bvec.PrintString();
//         EXPECT_NEAR(bvec.Perp(), tvec.Perp(), (bvec.Perp() < 10 ? 1e-12 : bvec.Perp() * 1e-12)) << bvec.PrintString();
//         EXPECT_NEAR(bvec.Perp2(), tvec.Perp2(), (bvec.Perp2() < 10 ? 1e-12 : bvec.Perp2() * 1e-12)) << bvec.PrintString();
        EXPECT_NEAR(bvec.X(), tvec.X(), epsilon) << bvec.PrintString();
        EXPECT_NEAR(bvec.Y(), tvec.Y(), epsilon) << bvec.PrintString();
        EXPECT_NEAR(bvec.Z(), tvec.Z(), epsilon) << bvec.PrintString();
      }
    }
  }
}  // namespace
