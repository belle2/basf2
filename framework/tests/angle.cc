#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Angle.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  double constexpr epsilon = 1e-7;

  TEST(Angle, BaseAngle)
  {
    class TestBaseAngle : public BaseAngle {
    public:
      TestBaseAngle(double angle, double error) : BaseAngle(angle, error) {};
    };

    const double angle = TMath::PiOver2();
    const double error = TMath::PiOver4();

    TestBaseAngle myBaseAngle(angle, error);
    EXPECT_DOUBLE_EQ(angle, myBaseAngle.getAngle());
    EXPECT_DOUBLE_EQ(error, myBaseAngle.getError());
    EXPECT_DOUBLE_EQ(angle * TMath::RadToDeg(), myBaseAngle.getAngleInDeg());
    EXPECT_DOUBLE_EQ(error * TMath::RadToDeg(), myBaseAngle.getErrorInDeg());
    EXPECT_DOUBLE_EQ(angle - error, myBaseAngle.getLowerIntervalBoundary());
    EXPECT_DOUBLE_EQ(angle + error, myBaseAngle.getUpperIntervalBoundary());
  }

  TEST(Angle, ThetaAngle)
  {
    const double angle = 0.4;
    const double error = 0.2;

    ThetaAngle myThetaAngle(angle, error);
    EXPECT_FALSE(myThetaAngle.contains(ThetaAngle(0., 0.)));
    EXPECT_FALSE(myThetaAngle.contains(ThetaAngle(1.6, 0.)));
    EXPECT_TRUE(myThetaAngle.contains(ThetaAngle(angle - error / 2, 0.)));
    EXPECT_TRUE(myThetaAngle.contains(ThetaAngle(angle + error / 2, 0.)));

    EXPECT_FLOAT_EQ(TMath::PiOver2(), ThetaAngle(TMath::Pi() + TMath::PiOver2(), 0.).getAngle());
  }

  TEST(Angle, PhiAngleIntervalNormalCase)
  {
    double angle = 0.4;
    double error = 0.2;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(0., 0.)));
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(TMath::TwoPi(), 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.)));

    EXPECT_FLOAT_EQ(TMath::Pi(), PhiAngle(TMath::TwoPi() + TMath::Pi(), 0.).getAngle());
  }

  TEST(Angle, PhiAngleIntervalBreaksZero)
  {
    const double angle = 0.1;
    const double error = 0.3;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 + epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 - epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() + epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() - epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.)));
  }

  TEST(Angle, PhiAngleIntervalBreaksTwoPi)
  {
    const double angle = TMath::TwoPi() - 0.1;
    const double error = 0.3;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 + epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 - epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() + epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() - epsilon, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.)));
  }

  TEST(Angle, PhiAngleNormalCase)
  {
    const double angle = TMath::Pi();
    const double error = 0.0;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(0 + epsilon, 0.2)));
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(0 - epsilon, 0.2)));
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() + epsilon, 0.2)));
    EXPECT_FALSE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() - epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.2)));
  }

  TEST(Angle, PhiAngleBreaksZero)
  {
    const double angle = 0.1;
    const double error = 0.0;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 + epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 - epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() + epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() - epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.2)));
  }

  TEST(Angle, PhiAngleBreaksTwoPi)
  {
    const double angle = TMath::TwoPi() - 0.1;
    const double error = 0.0;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 + epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(0 - epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() + epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(TMath::TwoPi() - epsilon, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle - error / 2, 0.2)));
    EXPECT_TRUE(myPhiAngle.contains(PhiAngle(angle + error / 2, 0.2)));
  }

  TEST(Angle, PhiAngleAndErrorBreakTwoPi)
  {
    const double angle = -0.0275 + TMath::TwoPi();
    const double error = 3 * 0.0104;
    PhiAngle myPhiAngle(angle, error);
    EXPECT_FALSE(myPhiAngle.containsIn(PhiAngle(2.6, 0), 1));
  }

}  // namespace
