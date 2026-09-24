/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <Math/Vector2D.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;


TEST(TrackFindingCDCTest, fitting_CDCObservations2D_centralize)
{
  CDCObservations2D observations;

  observations.fill(ROOT::Math::XYVector(0, 0));
  observations.fill(ROOT::Math::XYVector(1, 2));
  observations.fill(ROOT::Math::XYVector(2, 4));

  ROOT::Math::XYVector centralPoint = observations.centralize();

  EXPECT_EQ(ROOT::Math::XYVector(1, 2), centralPoint) <<
                                                      "Central point of observation (0,0) , (1,1) and (2,2) is not (1,1)" ;

  EXPECT_EQ(-1, observations.getX(0));
  EXPECT_EQ(-2, observations.getY(0));

  EXPECT_EQ(0, observations.getX(1));
  EXPECT_EQ(0, observations.getY(1));

  EXPECT_EQ(1, observations.getX(2));
  EXPECT_EQ(2, observations.getY(2));
}


