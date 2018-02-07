/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>
#include <framework/geometry/BFieldManager.h>
#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, eventdata_trajectories_CDCBField_getBFieldZSign)
{
  TVector3 bFieldAtOrigin = BFieldManager::getField(0, 0, 0) / Unit::T;
  Double_t bZAtOrigin = bFieldAtOrigin.Z();
  ESign bZSignAtOrigin = sign(bZAtOrigin);
  EXPECT_EQ(bZSignAtOrigin, CDCBFieldUtil::getBFieldZSign());
}
