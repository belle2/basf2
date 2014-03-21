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
//#include "cdcLocalTrackingTest.h"

#include <tracking/cdcLocalTracking/tempro/StaticString.h>
#include <tracking/cdcLocalTracking/tempro/Named.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

TEST(CDCLocalTrackingTestWithoutGearBox, StaticString)
{

  EXPECT_EQ(1, StaticString<>::size); //Always count the terminating null character.
  EXPECT_EQ(1, EmptyStaticString::size);
  EXPECT_EQ('\0', EmptyStaticString::first);
  EXPECT_STREQ("", EmptyStaticString::chars);
  EXPECT_STREQ("", EmptyStaticString::c_str());
  ::testing::StaticAssertTypeEq< EmptyStaticString, StaticString<> >();


  typedef StaticString < 'a' > StaticString_a;
  typedef StaticStringTail<StaticString_a> StaticString_a_tail;

  EXPECT_EQ(2, StaticString_a::size);
  EXPECT_EQ('a', StaticString_a::first);
  EXPECT_STREQ("a", StaticString_a::chars);
  EXPECT_STREQ("a", StaticString_a::c_str());

  EXPECT_EQ(1, StaticString_a_tail::size);
  EXPECT_EQ('\0', StaticString_a_tail::first);
  EXPECT_STREQ("", StaticString_a_tail::chars);
  EXPECT_STREQ("", StaticString_a_tail::c_str());



  ::testing::StaticAssertTypeEq< EmptyStaticString, StaticString_a_tail>();


  typedef StaticString < 't', 'e', 's', 't' > StaticString_test;
  typedef StaticString < 'e', 's', 't' > StaticString_est;

  typedef StaticStringTail<StaticString_test> StaticString_test_tail;

  EXPECT_EQ(5, StaticString_test::size);
  EXPECT_EQ('t', StaticString_test::first);
  EXPECT_STREQ("test", StaticString_test::chars);
  EXPECT_STREQ("test", StaticString_test::c_str());
  ::testing::StaticAssertTypeEq<StaticString_est, StaticString_test_tail>();

}



TEST(CDCLocalTrackingTestWithoutGearBox, StaticString_StaticStrip)
{

  typedef StaticString < 't', 'e', 's', 't' > StaticString_test;

  typedef StaticStripFront < '\0', '\0', '\0', 't', 'e', 's', 't' > StaticString_test2;
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test2>();

  typedef StaticStripBack < 't', 'e', 's', 't', '\0', '\0', '\0' > StaticString_test3;
  ::testing::StaticAssertTypeEq< StaticString_test, StaticString_test3>();

}
