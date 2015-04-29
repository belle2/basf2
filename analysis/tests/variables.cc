#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <analysis/utility/PRestFrameLabTransform.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** test kinematic Variable. */
  TEST(KinematicVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    {
      Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      EXPECT_FLOAT_EQ(0.9, particleP(&p));
      EXPECT_FLOAT_EQ(1.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.68176979, particleP_CMS(&p));
      EXPECT_FLOAT_EQ(0.80920333, particleE_CMS(&p));
      EXPECT_FLOAT_EQ(0.058562335, particlePx_CMS(&p));
      EXPECT_FLOAT_EQ(-0.40000001, particlePy_CMS(&p));
      EXPECT_FLOAT_EQ(0.54898131, particlePz_CMS(&p));
      EXPECT_FLOAT_EQ(0.40426421, particlePt_CMS(&p));
      EXPECT_FLOAT_EQ(0.80522972, particleCosTheta_CMS(&p));
      EXPECT_FLOAT_EQ(-1.4254233, particlePhi_CMS(&p));

      PRestFrameLabTransform T;
      T.setRestFrame(p.get4Vector().BoostVector());
      EXPECT_ALL_NEAR(particleP_RF(&p), 0.0, 1e-9);
      EXPECT_FLOAT_EQ(0.4358899, particleE_RF(&p));
      EXPECT_ALL_NEAR(0.0, particlePx_RF(&p), 1e-9);
      EXPECT_ALL_NEAR(0.0, particlePy_RF(&p), 1e-9);
      EXPECT_ALL_NEAR(0.0, particlePz_RF(&p), 1e-9);
      EXPECT_ALL_NEAR(0.0, particlePt_RF(&p), 1e-9);

      T.resetRestFrame();
      EXPECT_FLOAT_EQ(0.9, particleP_RF(&p));
      EXPECT_FLOAT_EQ(1.0, particleE_RF(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx_RF(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy_RF(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz_RF(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt_RF(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta_RF(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi_RF(&p));

      T.setRestFrame(-p.get4Vector().BoostVector());
      Particle p2({ 0.0 , 0.0, 0.0, 0.4358899}, 11);
      EXPECT_FLOAT_EQ(0.9, particleP_RF(&p2));
      EXPECT_FLOAT_EQ(1.0, particleE_RF(&p2));
      EXPECT_FLOAT_EQ(0.1, particlePx_RF(&p2));
      EXPECT_FLOAT_EQ(-0.4, particlePy_RF(&p2));
      EXPECT_FLOAT_EQ(0.8, particlePz_RF(&p2));
      EXPECT_FLOAT_EQ(0.412310562, particlePt_RF(&p2));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta_RF(&p2));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi_RF(&p2));
      T.resetRestFrame();
    }

    {
      Particle p({ 0.0 , 0.0, 0.0, 0.0 }, 11);
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.0, particleP_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particleE_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt_CMS(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta_CMS(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi_CMS(&p));
    }

  }

}
