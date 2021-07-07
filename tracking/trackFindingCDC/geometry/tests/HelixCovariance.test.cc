/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_HelixCovariance_constructFromPerigeeAndSZCovariance)
{
  PerigeeCovariance perigeeCovariance = PerigeeCovariance::Zero();
  {
    using namespace NPerigeeParameterIndices;
    perigeeCovariance(c_Curv, c_Curv) = 1;
    perigeeCovariance(c_Curv, c_I) = 1;
    perigeeCovariance(c_I, c_Curv) = 1;
    perigeeCovariance(c_I, c_I) = 1;
  }

  SZCovariance szCovariance = SZCovariance::Zero();
  {
    using namespace NSZParameterIndices;
    szCovariance(c_TanL, c_TanL) = 1;
    szCovariance(c_Z0, c_Z0) = 1;
  }

  HelixCovariance helixCovariance = HelixUtil::stackBlocks(perigeeCovariance, szCovariance);
  {
    using namespace NHelixParameterIndices;
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
