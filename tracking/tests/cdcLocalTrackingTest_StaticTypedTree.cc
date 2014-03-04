/*************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <cstdio>
#include <gtest/gtest.h>

#include <tracking/cdcLocalTracking/tempro/Named.h>
#include <tracking/cdcLocalTracking/tempro/BranchOf.h>
#include <tracking/cdcLocalTracking/tempro/StaticTypedTree.h>


using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTestWithoutGearBox, StaticTypedTree)
{

  typedef StaticTypedTree <
  BranchOf < int, NAMED("n") > ,
           BranchOf < int, NAMED("i") > ,
           BranchOf < float, NAMED("efficiency") >
           > TestTreeWrapper;

  ::testing::StaticAssertTypeEq < int, TestTreeWrapper::GetValueTypeAtTag < NAMED("i") > > ();

  TestTreeWrapper testTreeWrapper("test_tree", "A simple tree to test on");


  const char* testRootFileName = "test.root";
  TFile tFileToWriteTestTree(testRootFileName, "RECREATE");

  testTreeWrapper.create(tFileToWriteTestTree);

  //Make some dummy events
  int n = 10;
  float fullEfficiency = 1.0;

  for (int i = 0; i < n; ++i) {
    testTreeWrapper.setValue < NAMED("i") > (i);
    testTreeWrapper.setValue < NAMED("n") > (n);
    testTreeWrapper.setValue < NAMED("efficiency") > (fullEfficiency * i / n);
    testTreeWrapper.fill();
  }
  testTreeWrapper.save();

  EXPECT_EQ(10u, testTreeWrapper.size());

  tFileToWriteTestTree.Close();
  std::remove(testRootFileName);

}







