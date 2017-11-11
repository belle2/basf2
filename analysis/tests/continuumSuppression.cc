#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/CleoCones.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  class ContinuumSuppressionTests : public ::testing::Test {
  protected:
  };

  /** Test the calculation of a thrust axis */
  TEST_F(ContinuumSuppressionTests, Thrust)
  {
    std::vector<TVector3> momenta;
    // random generated nubmers
    momenta.push_back(TVector3(0.5935352844151847, 0.28902324918117417, 0.9939000705771412));
    momenta.push_back(TVector3(0.7097025137911714, 0.5118418422879152, 0.44501044145648994));
    momenta.push_back(TVector3(0.6005771199332856, 0.12366608492454145, 0.7541373665256832));
    momenta.push_back(TVector3(0.8548902083897467, 0.6887268865943484, 0.34301136659215437));
    momenta.push_back(TVector3(0.26863521039211535, 0.011148638667487942, 0.96186334199901));

    const TVector3 thrustB = Thrust::calculateThrust(momenta);

    EXPECT_FLOAT_EQ(0.925838, thrustB.Mag());
    EXPECT_FLOAT_EQ(0.571661, thrustB.X());
    EXPECT_FLOAT_EQ(0.306741, thrustB.Y());
    EXPECT_FLOAT_EQ(0.660522, thrustB.Z());
  }

  /** Test the calculation of the CleoClones variables */
  TEST_F(ContinuumSuppressionTests, CleoCones)
  {
    const bool use_all = true;
    const bool use_roe = true;
    std::vector<TVector3> momenta;
    std::vector<TVector3> sig_side_momenta;
    std::vector<TVector3> roe_side_momenta;

    // "Signal Side"
    sig_side_momenta.push_back(TVector3(0.5429965262452898, 0.37010582077332344, 0.0714978744529432));
    sig_side_momenta.push_back(TVector3(0.34160659934755344, 0.6444967896760643, 0.18455766323674105));
    sig_side_momenta.push_back(TVector3(0.9558442475237068, 0.3628892505037786, 0.545225050633818));
    sig_side_momenta.push_back(TVector3(0.8853521332124835, 0.340704481181513, 0.34728211023189237));
    sig_side_momenta.push_back(TVector3(0.3155615844988947, 0.8307541128801257, 0.45701302024212986));

    // "ROE Side"
    roe_side_momenta.push_back(TVector3(0.6100164897524695, 0.5077455724845565, 0.06639458334119974));
    roe_side_momenta.push_back(TVector3(0.5078972239903029, 0.9196504908351234, 0.3710366834603026));
    roe_side_momenta.push_back(TVector3(0.06252858849289977, 0.4680168989606487, 0.4056055050148607));
    roe_side_momenta.push_back(TVector3(0.61672460498333, 0.4472311336875816, 0.31288581834261064));
    roe_side_momenta.push_back(TVector3(0.18544654870476218, 0.0758107751704592, 0.31909701462121065));

    // "All momenta"
    momenta.push_back(TVector3(0.5429965262452898, 0.37010582077332344, 0.0714978744529432));
    momenta.push_back(TVector3(0.34160659934755344, 0.6444967896760643, 0.18455766323674105));
    momenta.push_back(TVector3(0.9558442475237068, 0.3628892505037786, 0.545225050633818));
    momenta.push_back(TVector3(0.8853521332124835, 0.340704481181513, 0.34728211023189237));
    momenta.push_back(TVector3(0.3155615844988947, 0.8307541128801257, 0.45701302024212986));
    momenta.push_back(TVector3(0.6100164897524695, 0.5077455724845565, 0.06639458334119974));
    momenta.push_back(TVector3(0.5078972239903029, 0.9196504908351234, 0.3710366834603026));
    momenta.push_back(TVector3(0.06252858849289977, 0.4680168989606487, 0.4056055050148607));
    momenta.push_back(TVector3(0.61672460498333, 0.4472311336875816, 0.31288581834261064));
    momenta.push_back(TVector3(0.18544654870476218, 0.0758107751704592, 0.31909701462121065));


    // Calculate thrust from "Signal Side"
    const TVector3 thrustB = Thrust::calculateThrust(sig_side_momenta);

    CleoCones myCleoCones(momenta, roe_side_momenta, thrustB, use_all, use_roe);

    const auto& cleo_cone_with_all = myCleoCones.cleo_cone_with_all();
    const auto& cleo_cone_with_roe = myCleoCones.cleo_cone_with_roe();

    EXPECT_FLOAT_EQ(0.823567, cleo_cone_with_all[0]);
    EXPECT_FLOAT_EQ(4.7405558, cleo_cone_with_all[1]);
    EXPECT_FLOAT_EQ(1.7517139, cleo_cone_with_all[2]);
    EXPECT_FLOAT_EQ(0.37677661, cleo_cone_with_all[3]);
    EXPECT_FLOAT_EQ(0.622467, cleo_cone_with_all[4]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[5]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[6]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[7]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[8]);

    EXPECT_FLOAT_EQ(0.823567, cleo_cone_with_roe[0]);
    EXPECT_FLOAT_EQ(1.9106253, cleo_cone_with_roe[1]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[2]);
    EXPECT_FLOAT_EQ(0.37677661, cleo_cone_with_roe[3]);
    EXPECT_FLOAT_EQ(0.622467, cleo_cone_with_roe[4]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[5]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[6]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[7]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[8]);
  }

  /** Test the calculation of the Fox-Wolfram moments */
  TEST_F(ContinuumSuppressionTests, FoxWolfram)
  {
    std::vector<TVector3> momenta;

    momenta.push_back(TVector3(0.5429965262452898, 0.37010582077332344, 0.0714978744529432));
    momenta.push_back(TVector3(0.34160659934755344, 0.6444967896760643, 0.18455766323674105));
    momenta.push_back(TVector3(0.9558442475237068, 0.3628892505037786, 0.545225050633818));
    momenta.push_back(TVector3(0.8853521332124835, 0.340704481181513, 0.34728211023189237));
    momenta.push_back(TVector3(0.3155615844988947, 0.8307541128801257, 0.45701302024212986));
    momenta.push_back(TVector3(0.6100164897524695, 0.5077455724845565, 0.06639458334119974));
    momenta.push_back(TVector3(0.5078972239903029, 0.9196504908351234, 0.3710366834603026));
    momenta.push_back(TVector3(0.06252858849289977, 0.4680168989606487, 0.4056055050148607));
    momenta.push_back(TVector3(0.61672460498333, 0.4472311336875816, 0.31288581834261064));
    momenta.push_back(TVector3(0.18544654870476218, 0.0758107751704592, 0.31909701462121065));

    FoxWolfram FW(momenta);
    EXPECT_FLOAT_EQ(0.63011014, FW.R(2));
  }

}  // namespace
