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
}  // namespace
