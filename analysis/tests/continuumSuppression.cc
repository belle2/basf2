#include <analysis/ContinuumSuppression/Thrust.h>
//#include <analysis/ContinuumSuppression/FuncPtr.h>


#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  class ContinuumSuppressionTests : public ::testing::Test {
  protected:
  };

  TEST_F(ContinuumSuppressionTests, Thrust)
  {
    std::vector<TVector3> momenta;
    // random generated nubmers
    momenta.push_back(TVector3(0.5935352844151847, 0.28902324918117417, 0.9939000705771412));
    momenta.push_back(TVector3(0.7097025137911714, 0.5118418422879152, 0.44501044145648994));
    momenta.push_back(TVector3(0.6005771199332856, 0.12366608492454145, 0.7541373665256832));
    momenta.push_back(TVector3(0.8548902083897467, 0.6887268865943484, 0.34301136659215437));
    momenta.push_back(TVector3(0.26863521039211535, 0.011148638667487942, 0.96186334199901));

    const TVector3 thrustB = thrust(momenta.begin(), momenta.end(), SelfFunc(TVector3()));

    EXPECT_FLOAT_EQ(0.925838, thrustB.Mag());
    EXPECT_FLOAT_EQ(0.571661, thrustB.X());
    EXPECT_FLOAT_EQ(0.306741, thrustB.Y());
    EXPECT_FLOAT_EQ(0.660522, thrustB.Z());
  }

}  // namespace
