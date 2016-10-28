/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, eventdata_trajectories_CDCBField_getBFieldZSign)
{
  BFieldMap& bFieldMap = BFieldMap::Instance();

  TVector3 origin(0.0, 0.0, 0.0);
  TVector3 bFieldAtOrigin = bFieldMap.getBField(origin);
  Double_t bZAtOrigin = bFieldAtOrigin.Z();
  ESign bZSignAtOrigin = sign(bZAtOrigin);
  EXPECT_EQ(bZSignAtOrigin, CDCBFieldUtil::getBFieldZSign());
}
