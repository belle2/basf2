/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/HelixCovariance.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, geometry_HelixCovariance_perigeeCovariance)
{
  HelixCovariance helixCovariance;
  PerigeeCovariance perigeeCovariance = helixCovariance.perigeeCovariance();

  EXPECT_EQ(3, perigeeCovariance.matrix().GetNrows());
  EXPECT_EQ(3, perigeeCovariance.matrix().GetNcols());

}



TEST(TrackFindingCDCTest, geometry_HelixCovariance_constructFromPerigeeAndSZCovariance)
{
  PerigeeCovariance perigeeCovariance;
  {
    using namespace NPerigeeParameter;
    perigeeCovariance(c_Curv, c_Curv) = 1;
    perigeeCovariance(c_Curv, c_I) = 1;
    perigeeCovariance(c_I, c_Curv) = 1;
    perigeeCovariance(c_I, c_I) = 1;
  }

  SZCovariance szCovariance;
  {
    using namespace NHelixParameter;
    szCovariance(c_TanL, c_TanL) = 1;
    szCovariance(c_Z0, c_Z0) = 1;
  }

  HelixCovariance helixCovariance(perigeeCovariance, szCovariance);
  {
    using namespace NHelixParameter;
    EXPECT_EQ(1, helixCovariance(c_Curv, c_Curv));
    EXPECT_EQ(1, helixCovariance(c_I, c_I));
    EXPECT_EQ(1, helixCovariance(c_Curv, c_I));
    EXPECT_EQ(1, helixCovariance(c_I, c_Curv));

    EXPECT_EQ(1, helixCovariance(c_TanL, c_TanL));
    EXPECT_EQ(1, helixCovariance(c_Z0, c_Z0));


    EXPECT_EQ(0, helixCovariance(c_Phi0, c_Phi0));
    EXPECT_EQ(0, helixCovariance(c_TanL, c_Curv));
  }
}
