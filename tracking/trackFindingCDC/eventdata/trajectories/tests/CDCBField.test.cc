/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
