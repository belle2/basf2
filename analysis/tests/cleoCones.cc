#include <analysis/ContinuumSuppression/CleoCones.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  class CleoConesTest : public ::testing::Test {
  protected:
  };

  TEST_F(CleoConesTest, CalculateCleoCones)
  {
    const bool use_all = true;
    const bool use_roe = true;
    const TVector3 thrustB(0., 0., 1.);
    std::vector<TVector3> momenta;
    momenta.reserve(9);
    // Values taken from a random generated event.
    momenta.push_back(TVector3(-0.16237690052171302, -0.18598630652190457, -0.18598630652190457));
    momenta.push_back(TVector3(0.07630033024231285, 0.26398409594930783, 0.26398409594930783));
    momenta.push_back(TVector3(0.15925254327966593, -0.060847436085921025, -0.060847436085921025));
    momenta.push_back(TVector3(0.1099055303280146, 0.46121889282910966, 0.46121889282910966));
    momenta.push_back(TVector3(-0.5690438860178658, -0.3668729368448115, -0.3668729368448115));
    momenta.push_back(TVector3(0.6213166747214871, -0.3567520028486626, -0.3567520028486626));
    momenta.push_back(TVector3(0.10081873338000946, 0.6814677255797857, 0.6814677255797857));
    momenta.push_back(TVector3(0.3679447092806477, -0.044973339550589825, -0.044973339550589825));
    momenta.push_back(TVector3(0.13150773389426154, 0.24437636979150335, 0.24437636979150335));
    momenta.push_back(TVector3(0.4012527094543953, 0.00046181291714367207, 0.00046181291714367207));
    momenta.push_back(TVector3(-0.5534924447079044, -0.6554721653498968, -0.6554721653498968));
    momenta.push_back(TVector3(0.02048997137566017, 0.09971662879528322, 0.09971662879528322));
    momenta.push_back(TVector3(-0.009505785468758163, 0.06972251203663654, 0.06972251203663654));
    momenta.push_back(TVector3(0.033475682325452234, -0.14769911713945041, -0.14769911713945041));
    momenta.push_back(TVector3(-0.11192008610720518, 0.16906106820714972, 0.16906106820714972));
    momenta.push_back(TVector3(0.38044550355419665, -0.054415858921282166, -0.054415858921282166));

    CleoCones myCleoCones(momenta, momenta, thrustB, use_all, use_roe);

    const auto& cleo_cone_with_all = myCleoCones.cleo_cone_with_all();
    const auto& cleo_cone_with_roe = myCleoCones.cleo_cone_with_roe();

    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[0]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[1]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[2]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[3]);
    EXPECT_FLOAT_EQ(2.8343837, cleo_cone_with_all[4]);
    EXPECT_FLOAT_EQ(1.6527448, cleo_cone_with_all[5]);
    EXPECT_FLOAT_EQ(1.5704279, cleo_cone_with_all[6]);
    EXPECT_FLOAT_EQ(0.18101434, cleo_cone_with_all[7]);
    EXPECT_FLOAT_EQ(1.1628052, cleo_cone_with_all[8]);

    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[0]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[1]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[2]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[3]);
    EXPECT_FLOAT_EQ(2.8343837, cleo_cone_with_roe[4]);
    EXPECT_FLOAT_EQ(1.6527448, cleo_cone_with_roe[5]);
    EXPECT_FLOAT_EQ(1.5704279, cleo_cone_with_roe[6]);
    EXPECT_FLOAT_EQ(0.18101434, cleo_cone_with_roe[7]);
    EXPECT_FLOAT_EQ(1.1628052, cleo_cone_with_roe[8]);



  }

}  // namespace
