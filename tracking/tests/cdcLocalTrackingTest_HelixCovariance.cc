/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include "cdcLocalTrackingTest.h"

#include <tracking/cdcLocalTracking/geometry/HelixCovariance.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;



TEST_F(CDCLocalTrackingTest, HelixCovariance_perigeeCovariance)
{
  HelixCovariance helixCovariance;
  PerigeeCovariance perigeeCovariance = helixCovariance.perigeeCovariance();

  EXPECT_EQ(3, perigeeCovariance.matrix().GetNrows());
  EXPECT_EQ(3, perigeeCovariance.matrix().GetNcols());

}


