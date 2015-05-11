#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <analysis/utility/ReferenceFrame.h>

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

      {
        UseReferenceFrame<CMSFrame> dummy;
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.058562335, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54898131, particlePz(&p));
        EXPECT_FLOAT_EQ(0.40426421, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80522972, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4254233, particlePhi(&p));
      }

      {
        UseReferenceFrame<RestFrame> dummy(&p);
        EXPECT_ALL_NEAR(particleP(&p), 0.0, 1e-9);
        EXPECT_FLOAT_EQ(0.4358899, particleE(&p));
        EXPECT_ALL_NEAR(0.0, particlePx(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePy(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePz(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePt(&p), 1e-9);
      }

      {
        UseReferenceFrame<LabFrame> dummy;
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));
      }

      {
        Particle pinv({ -0.1 , 0.4, -0.8, 1.0 }, 11);
        UseReferenceFrame<RestFrame> dummy(&pinv);
        Particle p2({ 0.0 , 0.0, 0.0, 0.4358899}, 11);
        EXPECT_FLOAT_EQ(0.9, particleP(&p2));
        EXPECT_FLOAT_EQ(1.0, particleE(&p2));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p2));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p2));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p2));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p2));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p2));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p2));
      }
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

      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));
    }

  }

}
