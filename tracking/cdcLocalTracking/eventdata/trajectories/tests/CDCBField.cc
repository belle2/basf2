/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCBField.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTest, eventdata_trajectories_CDCBField_getBFieldZSign)
{
  BFieldMap& bFieldMap = BFieldMap::Instance();

  TVector3 origin(0.0, 0.0, 0.0);
  TVector3 bFieldAtOrigin = bFieldMap.getBField(origin);
  Double_t bZAtOrigin = bFieldAtOrigin.Z();
  SignType bZSignAtOrigin = sign(bZAtOrigin);
  EXPECT_EQ(getBFieldZSign(), bZSignAtOrigin);

  // EXPECT_NE(0.0, bZAtOrigin);
  // EXPECT_EQ(getBFieldZ(), bZAtOrigin);

}
