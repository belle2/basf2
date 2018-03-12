#include <ecl/dataobjects/ECLShower.h>
#include <ecl/utility/utilityFunctions.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/ECLCluster.h>
#include "TRandom.h"
#include "TMath.h"
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  using namespace ECL;

  /** Test Constructors. */
  TEST(TestgetDetectorRegion, TestgetDetectorRegion)
  {
    ECLShower myECLShower;
    ECLCluster myECLCluster;
    for (int i = 0; i < 1e4; ++i) {
      const double theta = gRandom->Uniform(0, TMath::Pi());

      //Check shower
      myECLShower.setTheta(theta);
      EXPECT_EQ(myECLShower.getDetectorRegion(), static_cast<int>(getDetectorRegion(theta)));

      //Check ECLCluster
      myECLCluster.setTheta(theta);
      EXPECT_EQ(myECLCluster.getDetectorRegion(), static_cast<int>(getDetectorRegion(theta)));
    }

  } // Testcases for Something


}  // namespace
