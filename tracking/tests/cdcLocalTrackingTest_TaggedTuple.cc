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

#include <tracking/cdcLocalTracking/tempro/TaggedTuple.h>
#include <tracking/cdcLocalTracking/tempro/BranchOf.h>
#include <tracking/cdcLocalTracking/tempro/Named.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


// This file contains only compile time test.

TEST(CDCLocalTrackingTestWithoutGearBox, GetTag)
{
  typedef BranchOf<float> floatBranch;

  //Default tag is the type name itself
  ::testing::StaticAssertTypeEq< float, floatBranch::Tag>();
  ::testing::StaticAssertTypeEq< float, GetTag<floatBranch> >();


  typedef StaticString < 't', 'e', 's', 't' > StaticString_test;
  typedef BranchOf<float, StaticString_test> taggedFloatBranch;

  ::testing::StaticAssertTypeEq< StaticString_test, taggedFloatBranch::Tag>();
  ::testing::StaticAssertTypeEq< StaticString_test, GetTag<taggedFloatBranch> >();


  typedef BranchOf < float, NAMED("test") > namedFloatBranch;

  ::testing::StaticAssertTypeEq < NAMED("test"), namedFloatBranch::Tag > ();
  ::testing::StaticAssertTypeEq < NAMED("test"), GetTag<namedFloatBranch> > ();

  ::testing::StaticAssertTypeEq< StaticString_test, namedFloatBranch::Tag>();
  ::testing::StaticAssertTypeEq< StaticString_test, GetTag<namedFloatBranch> >();

}


TEST(CDCLocalTrackingTestWithoutGearBox, GetIndexInTuple)
{

  static_assert(0 == GetIndexInTuple<int, std::tuple<int> >::value, "");
  static_assert(0 == GetIndexInTuple<int, std::tuple<int, int> >::value, "");
  static_assert(1 == GetIndexInTuple<int, std::tuple<float, int> >::value, "");

}




TEST(CDCLocalTrackingTestWithoutGearBox, GetIndex)
{

  static_assert(0 == GetIndex<int, std::tuple<int> >::value, "");
  static_assert(0 == GetIndex<int, std::tuple<int, int> >::value, "");
  static_assert(1 == GetIndex<int, std::tuple<float, int> >::value, "");

}



TEST(CDCLocalTrackingTestWithoutGearBox, TaggedTuple)
{
  // Tests for the tagged tuple type
  // Uses BranchOf as an example of a tagged type

  typedef BranchOf<float> FloatBranch;
  typedef BranchOf < int, NAMED("test") > NamedIntBranch;

  typedef TaggedTuple<FloatBranch, NamedIntBranch> TestTaggedTuple;

  ::testing::StaticAssertTypeEq< FloatBranch, TestTaggedTuple::GetTypeAtTag<float> >();
  ::testing::StaticAssertTypeEq < NamedIntBranch, TestTaggedTuple::GetTypeAtTag < NAMED("test") > > ();

  typedef  TestTaggedTuple::GetTypeAtIndex<0> TypeAtIndex0;
  typedef  TestTaggedTuple::GetTypeAtIndex<1> TypeAtIndex1;

  ::testing::StaticAssertTypeEq< FloatBranch, TypeAtIndex0 >();
  ::testing::StaticAssertTypeEq< NamedIntBranch, TypeAtIndex1 >();

  TestTaggedTuple testTaggedTuple;

  FloatBranch& floatBranch = testTaggedTuple.get<float>();
  NamedIntBranch& namedIntBranch = testTaggedTuple.get < NAMED("test") > ();

  floatBranch.setValue(4.0);
  namedIntBranch.setValue(6);

  EXPECT_EQ(4.0,  floatBranch.getValue());
  EXPECT_EQ(6,  namedIntBranch.getValue());

}
