#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/FourHitFilters.h>
#include <tracking/vxdCaTracking/LittleHelper.h>
#include <tracking/vxdCaTracking/SharedFunctions.h>

// stl:
#include <vector>
#include <math.h>
#include <utility> // pair
#include <fstream>

// root:
#include <TVector3.h>
#include <gtest/gtest.h>
#include <TRandom.h>
#include <TMatrixD.h>

//boost:
// #include <boost/foreach.hpp>
#include <boost/bind/bind.hpp>
// #ifndef __CINT__
// #include <boost/chrono.hpp>
// #endif

//Eigen
#include <Eigen/Dense>

using namespace std;

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class FourHitFiltersTest : public ::testing::Test {
  protected:
  };

  /** tests filters using 4 hits. */
  TEST_F(FourHitFiltersTest, TestFourHits)
  {
    TVector3 hit2(-2, 0, 0.), hit3(0, 2, 0.), hit1(0, 0, 0.), hit4(2, 0, 0.);
    FourHitFilters aFilter = FourHitFilters(hit1, hit2, hit3, hit4);

    EXPECT_FLOAT_EQ(sqrt(2), aFilter.deltaDistCircleCenter());
    EXPECT_FLOAT_EQ(0.00263349, aFilter.deltapT());     //aFilter.calcPt(2)-aFilter.calcPt(sqrt(2))= 0.00263349

  }



}  // namespace
